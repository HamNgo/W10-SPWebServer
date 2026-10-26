#include "sws.h"

static void reap_child(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int init_server_socket(int port) {
    int fd;
    struct sockaddr_in6 server_addr;
    int opt = 1, v6only = 0;

    struct sigaction sa;
    sa.sa_handler = reap_child;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, 10) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    return fd;
}

void handle_client(int client_fd) {
    parse_http_request(client_fd);
}
