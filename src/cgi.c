#define _POSIX_C_SOURCE 200809L
#include "../sws.h"

extern char **environ;

/* Send all bytes, including when write() is partial. */
static int send_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;

    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

/* CGI-specific HTTP error response. */
static void cgi_error(int fd, int status, const char *message) {
    char response[512];
    int n = snprintf(response, sizeof(response),
        "HTTP/1.0 %d %s\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%d %s\n",
        status, message, status, message);

    if (n > 0 && (size_t)n < sizeof(response))
        send_all(fd, response, (size_t)n);
}

/* Recognize a request for a CGI resource. */
int is_cgi_request(const char *uri) {
    return uri != NULL && strncmp(uri, "/cgi-bin/", 9) == 0;
}

/* Execute CGI script using pipe(), fork(), and execve(). */
void execute_cgi(int client_fd, const char *uri,
                 const char *query_string) {
    char script_path[1024];
    int pipe_fd[2];
    pid_t pid;
    struct stat st;
    char buffer[BUFFER_SIZE];
    ssize_t n;
    int length;

    if (!config.cgi_dir || !is_cgi_request(uri)) {
        cgi_error(client_fd, 404, "Not Found");
        return;
    }

    /* Only allow simple script names inside the CGI directory. */
    const char *name = uri + 9;
    if (*name == '\0' || strchr(name, '/') ||
        strcmp(name, ".") == 0 || strstr(name, "..")) {
        cgi_error(client_fd, 403, "Forbidden");
        return;
    }

    length = snprintf(script_path, sizeof(script_path),
                      "%s/%s", config.cgi_dir, name);

    if (length < 0 || (size_t)length >= sizeof(script_path)) {
        cgi_error(client_fd, 400, "Bad Request");
        return;
    }

    if (stat(script_path, &st) < 0) {
        cgi_error(client_fd, errno == EACCES ? 403 : 404,
                  errno == EACCES ? "Forbidden" : "Not Found");
        return;
    }

    if (!S_ISREG(st.st_mode) || access(script_path, X_OK) != 0) {
        cgi_error(client_fd, 403, "Forbidden");
        return;
    }

    if (pipe(pipe_fd) < 0) {
        cgi_error(client_fd, 500, "Internal Server Error");
        return;
    }

    pid = fork();

    if (pid < 0) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        cgi_error(client_fd, 500, "Internal Server Error");
        return;
    }

    if (pid == 0) {
        char *args[] = {script_path, NULL};

        /* Restore signal handling for the CGI process. */
        signal(SIGCHLD, SIG_DFL);

        close(pipe_fd[0]);

        if (dup2(pipe_fd[1], STDOUT_FILENO) < 0)
            _exit(127);

        close(pipe_fd[1]);

        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("REQUEST_METHOD", "GET", 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.0", 1);
        setenv("QUERY_STRING",
               query_string ? query_string : "", 1);
        setenv("SCRIPT_NAME", uri, 1);

        char port[16];
        snprintf(port, sizeof(port), "%d", config.port);
        setenv("SERVER_PORT", port, 1);

        execve(script_path, args, environ);
        _exit(127);
    }

    /* Parent process reads the CGI output. */
    close(pipe_fd[1]);

    const char *header = "HTTP/1.0 200 OK\r\n";
    if (send_all(client_fd, header, strlen(header)) == 0) {
        while ((n = read(pipe_fd[0], buffer,
                         sizeof(buffer))) > 0) {
            if (send_all(client_fd, buffer, (size_t)n) < 0)
                break;
        }
    }

    close(pipe_fd[0]);

    while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {
    }
}
