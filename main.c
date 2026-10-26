#include "sws.h"

struct server_config config;

void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    if (chdir("/") < 0) exit(EXIT_FAILURE);

    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null != -1) {
        dup2(dev_null, STDIN_FILENO);
        dup2(dev_null, STDOUT_FILENO);
        dup2(dev_null, STDERR_FILENO);
        close(dev_null);
    }
}

int main(int argc, char **argv) {
    int opt;
    config.port = DEFAULT_PORT;
    config.debug_mode = 0;
    config.cgi_dir = NULL;
    config.log_file = NULL;

    while ((opt = getopt(argc, argv, "c:dl:p:h")) != -1) {
        switch (opt) {
            case 'c': config.cgi_dir = optarg; break;
            case 'd': config.debug_mode = 1; break;
            case 'l': config.log_file = optarg; break;
            case 'p': config.port = atoi(optarg); break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-c dir] [-d] [-l file] [-p port] dir\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: Missing document root directory.\n");
        exit(EXIT_FAILURE);
    }
    config.doc_root = argv[optind];

    if (!config.debug_mode) {
        daemonize();
    }

    int server_fd = init_server_socket(config.port);
    if (config.debug_mode) {
        printf("[Scenario 1 - Khoi] sws server listening on port %d...\n", config.port);
    }

    while (1) {
        struct sockaddr_in6 client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle_client(client_fd);
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        close(client_fd);
    }
    return 0;
}
