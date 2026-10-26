#ifndef HTTP_H
#define HTTP_H

#define HTTP_METHOD_SIZE 16
#define HTTP_URI_SIZE 1024
#define HTTP_VERSION_SIZE 16

struct http_request {
    char method[HTTP_METHOD_SIZE];
    char uri[HTTP_URI_SIZE];
    char version[HTTP_VERSION_SIZE];
};

/*
 * Parse an HTTP request line.
 *
 * Returns:
 *   0  on success
 *  -1  if the request line is malformed or unsupported
 */
int http_parse_request_line(const char *line,
                            struct http_request *request);

#endif
