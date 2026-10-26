#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "http.h"

int
http_parse_request_line(const char *line, struct http_request *request)
{
    char method[HTTP_METHOD_SIZE];
    char uri[HTTP_URI_SIZE];
    char version[HTTP_VERSION_SIZE];
    char extra;
    int i;

    if (line == NULL || request == NULL) {
        return -1;
    }

    /*
     * A request line must contain:
     * METHOD SP URI SP HTTP-VERSION
     *
     * The extra %c detects unexpected additional fields.
     */
    if (sscanf(line, "%15s %1023s %15s %c",
               method, uri, version, &extra) != 3) {
        return -1;
    }

    /*
     * HTTP methods are case-sensitive tokens.
     * Check that the method contains valid token characters.
     */
    for (i = 0; method[i] != '\0'; i++) {
        unsigned char c = (unsigned char)method[i];

        if (!isalnum(c) &&
            c != '!' && c != '#' && c != '$' &&
            c != '%' && c != '&' && c != '\'' &&
            c != '*' && c != '+' && c != '-' &&
            c != '.' && c != '^' && c != '_' &&
            c != '`' && c != '|' && c != '~') {
            return -1;
        }
    }

    /*
     * This assignment requires relative URIs.
     * Reject absolute URIs and other URI forms here.
     */
    if (uri[0] != '/' || uri[1] == '\0') {
        if (strcmp(uri, "/") != 0) {
            return -1;
        }
    }

    /*
     * Accept HTTP/1.0 and HTTP/1.1 requests.
     */
    if (strcmp(version, "HTTP/1.0") != 0 &&
        strcmp(version, "HTTP/1.1") != 0) {
        return -1;
    }

    /*
     * Copy the validated fields into the output structure.
     */
    strcpy(request->method, method);
    strcpy(request->uri, uri);
    strcpy(request->version, version);

    return 0;
}
