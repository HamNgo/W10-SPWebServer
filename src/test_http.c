#include <stdio.h>
#include "http.h"

int
main(void)
{
    struct http_request request;

    const char *line = "GET /index.html HTTP/1.0";

    if (http_parse_request_line(line, &request) == 0) {
        printf("Method: %s\n", request.method);
        printf("URI: %s\n", request.uri);
        printf("Version: %s\n", request.version);
    } else {
        printf("Invalid HTTP request line\n");
    }

    return 0;
}
