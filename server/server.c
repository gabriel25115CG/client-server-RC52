#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <stdarg.h>

#include "linked_list.h"
#include "access_code.h"
#include "history.h"

#define MSG_LEN 1024
#define MAX_CLIENTS 32
#define HISTORY_FILE "access_history.log"

/* ANSI colors */
#define C_RESET "\x1b[0m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_RED "\x1b[31m"
#define C_CYAN "\x1b[36m"

/* helper send formatted */
int sendf(int sock, const char *fmt, ...) {
    char buf[MSG_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return write(sock, buf, strlen(buf));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int server_fd;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];
    client_node_t *clients = NULL;
    int nfds = 1;

    /* Init access code */
    char default_code[CODE_LEN + 1];
    generate_default_code(default_code);
    set_code(default_code, 0);
    printf(C_CYAN "[server] Initial code: %s\n" C_RESET, get_current_code());

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); return 1;
    }

    listen(server_fd, 10);
    printf(C_CYAN "[server] Listening on port %d\n" C_RESET, port);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    char last_code[CODE_LEN + 1];
    strncpy(last_code, get_current_code(), CODE_LEN + 1);

    while (1) {
        int ret = poll(fds, nfds, 1000);
        if (ret < 0) { perror("poll"); break; }

        /* expiration */
        check_and_rotate_code_if_expired();
        if (strcmp(last_code, get_current_code()) != 0) {
            client_node_t *owner = find_owner(clients);
            if (owner)
                sendf(owner->socket_fd, "NEW_CODE:%s\n", get_current_code());
            printf(C_YELLOW "[server] Code rotated: %s\n" C_RESET, get_current_code());
            strncpy(last_code, get_current_code(), CODE_LEN + 1);
        }

        /* new connection */
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in caddr;
            socklen_t len = sizeof(caddr);
            int csock = accept(server_fd, (struct sockaddr*)&caddr, &len);
            if (csock >= 0 && nfds < MAX_CLIENTS) {
                clients = add_client(clients, csock, caddr);
                fds[nfds].fd = csock;
                fds[nfds].events = POLLIN;
                nfds++;
                printf(C_GREEN "[server] New connection fd=%d\n" C_RESET, csock);
            }
        }

        /* clients */
        for (int i = 1; i < nfds; i++) {
            if (!(fds[i].revents & POLLIN)) continue;

            char buf[MSG_LEN];
            int n = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
            if (n <= 0) {
                printf(C_RED "[server] Client fd=%d disconnected\n" C_RESET, fds[i].fd);
                clients = remove_client(clients, fds[i].fd);
                close(fds[i].fd);
                fds[i] = fds[nfds - 1];
                nfds--;
                i--;
                continue;
            }

            buf[n] = '\0';
            char *nl = strchr(buf, '\n');
            if (nl) *nl = '\0';

            client_node_t *c = find_client_by_fd(clients, fds[i].fd);
            if (!c) continue;

            /* quit */
            if (strcmp(buf, "/quit") == 0) {
                clients = remove_client(clients, c->socket_fd);
                close(c->socket_fd);
                fds[i] = fds[nfds - 1];
                nfds--;
                i--;
                continue;
            }

            /* identification */
            if (c->role == ROLE_UNKNOWN) {
                if (strncmp(buf, "OWNER:", 6) == 0) {
                    strncpy(c->pseudo, buf + 6, sizeof(c->pseudo) - 1);
                    c->role = ROLE_OWNER;
                    sendf(c->socket_fd, "DEFAULT_CODE:%s\n", get_current_code());
                    printf(C_CYAN "[server] Owner identified: %s\n" C_RESET, c->pseudo);
                } else if (strcmp(buf, "TENANT") == 0) {
                    c->role = ROLE_TENANT;
                    strcpy(c->pseudo, "0");
                    sendf(c->socket_fd, "SENDCODE\n");
                    printf(C_CYAN "[server] Tenant connected fd=%d\n" C_RESET, c->socket_fd);
                } else {
                    sendf(c->socket_fd, "ERROR:IDENTIFY\n");
                }
                continue;
            }

            /* owner */
            if (c->role == ROLE_OWNER) {
                if (strncmp(buf, "SETCODE:", 8) == 0) {
                    char code[CODE_LEN + 1];
                    long dur = 0;
                    if (sscanf(buf + 8, "%6[0-9]:%ld", code, &dur) >= 1) {
                        set_code(code, dur);
                        sendf(c->socket_fd, "OK:CODE_SET:%s\n", get_current_code());
                        strncpy(last_code, get_current_code(), CODE_LEN + 1);
                        printf(C_GREEN "[server] Owner set code: %s\n" C_RESET, code);
                    } else sendf(c->socket_fd, "ERROR:FORMAT\n");
                } else if (strcmp(buf, "GETCODE") == 0) {
                    sendf(c->socket_fd, "CURRENT_CODE:%s\n", get_current_code());
                } else if (strncmp(buf, "MSG:", 4) == 0) {
                    printf(C_CYAN "[owner] %s\n" C_RESET, buf + 4);
                } else {
                    sendf(c->socket_fd, "ERROR:UNKNOWN_COMMAND\n");
                }
                continue;
            }

            /* tenant */
            if (c->role == ROLE_TENANT) {
                if (strncmp(buf, "CODE:", 5) == 0) {
                    char code[CODE_LEN + 1];
                    sscanf(buf + 5, "%6[0-9]", code);
                    int attempts = atoi(c->pseudo) + 1;

                    if (validate_code(code)) {
                        sendf(c->socket_fd, "OK:ACCESS_GRANTED\n");
                        strcpy(c->pseudo, "0");
                        printf(C_GREEN "[tenant fd=%d] Access granted\n" C_RESET, c->socket_fd);
                    } else if (attempts >= 3) {
                        char newc[CODE_LEN + 1];
                        generate_default_code(newc);
                        set_code(newc, 0);
                        sendf(c->socket_fd, "ALARM:ACCESS_BLOCKED\n");
                        client_node_t *owner = find_owner(clients);
                        if (owner)
                            sendf(owner->socket_fd, "NEW_CODE:%s\n", get_current_code());
                        strcpy(c->pseudo, "0");
                        strncpy(last_code, get_current_code(), CODE_LEN + 1);
                        printf(C_RED "[server] Alarm triggered, code rotated: %s\n" C_RESET, get_current_code());
                    } else {
                        char tmp[16];
                        snprintf(tmp, sizeof(tmp), "%d", attempts);
                        strcpy(c->pseudo, tmp);
                        sendf(c->socket_fd, "ERROR:BAD_CODE_ATTEMPT:%d\n", attempts);
                        printf(C_YELLOW "[tenant fd=%d] Bad attempt %d\n" C_RESET, c->socket_fd, attempts);
                    }
                } else if (strncmp(buf, "MSG:", 4) == 0) {
                    printf(C_CYAN "[tenant fd=%d] %s\n" C_RESET, c->socket_fd, buf + 4);
                } else {
                    sendf(c->socket_fd, "ERROR:EXPECTED_CODE\n");
                }
            }
        }
    }

    free_clients(clients);
    close(server_fd);
    return 0;
}
