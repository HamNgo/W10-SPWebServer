#include "sws.h"

void send_error(int client_fd, int status, const char *msg) {
    char resp[BUFFER_SIZE];
    snprintf(resp, sizeof(resp),
             "HTTP/1.0 %d %s\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<html><body><h1>%d %s</h1></body></html>\n",
             status, msg, status, msg);
    write(client_fd, resp, strlen(resp));
}

void parse_http_request(int client_fd) {
    char buf[BUFFER_SIZE];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
    if (n <= 0) return;
    buf[n] = '\0';

    char method[16], uri[256], version[16];
    if (sscanf(buf, "%15s %255s %15s", method, uri, version) < 3) {
        send_error(client_fd, 400, "Bad Request");
        return;
    }

    if (strcmp(method, "GET") != 0) {
        send_error(client_fd, 501, "Not Implemented");
        return;
    }

    // Chống đường dẫn nguy hiểm (Directory Traversal)
    if (strstr(uri, "..")) {
        send_error(client_fd, 403, "Forbidden");
        return;
    }

    // Kiểm tra kịch bản CGI
    char *query_string = strchr(uri, '?');
    if (query_string) {
        *query_string = '\0';
        query_string++;
    }

    if (config.cgi_dir && is_cgi_request(uri)) {
        execute_cgi(client_fd, uri, query_string);
        return;
    }

    // Phục vụ file tĩnh
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", config.doc_root, strcmp(uri, "/") == 0 ? "/index.html" : uri);

    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0) {
        if (errno == EACCES) send_error(client_fd, 403, "Forbidden");
        else send_error(client_fd, 404, "Not Found");
        return;
    }

    struct stat st;
    fstat(file_fd, &st);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.0 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n\r\n", (long)st.st_size);
    write(client_fd, header, strlen(header));

    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buf, sizeof(buf))) > 0) {
        write(client_fd, buf, bytes_read);
    }
    close(file_fd);

    if (config.log_file) {
        write_log("127.0.0.1", uri, 200, st.st_size);
    }
}

