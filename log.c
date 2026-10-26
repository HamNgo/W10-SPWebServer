#include "sws.h"

void write_log(const char *ip, const char *request, int status, size_t bytes) {
    if (!config.log_file) return;
    FILE *f = fopen(config.log_file, "a");
    if (!f) return;

    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(f, "%s - [%s] \"%s\" %d %zu\n", ip, time_str, request, status, bytes);
    fclose(f);
}

