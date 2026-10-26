#ifndef SWS_H
#define SWS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 8192

struct server_config {
    int port;
    int debug_mode;
    char *cgi_dir;
    char *log_file;
    char *doc_root;
};

extern struct server_config config;

/* Functions Prototypes */
void daemonize(void);
int init_server_socket(int port);
void handle_client(int client_fd);
void parse_http_request(int client_fd);
int is_cgi_request(const char *uri);
void execute_cgi(int client_fd, const char *uri, const char *query_string);
void write_log(const char *ip, const char *request, int status, size_t bytes);

#endif
