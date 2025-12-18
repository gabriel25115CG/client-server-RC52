#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#define MSG_LEN 1024
#define C_CYAN "\x1b[36m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_RED "\x1b[31m"
#define C_RESET "\x1b[0m"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <server_port> <pseudo>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    struct pollfd fds[2];
    char buf[MSG_LEN];

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }
    printf(C_GREEN "[owner] Connected\n" C_RESET);

    snprintf(buf, sizeof(buf), "OWNER:%s\n", argv[3]);
    write(sock, buf, strlen(buf));

    printf(C_YELLOW "[owner] Commands:\n" C_RESET);
    printf("  SETCODE:<6digits>:<duration>\n");
    printf("  GETCODE\n");
    printf("  message texte libre\n");
    printf("  /quit\n");

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sock;
    fds[1].events = POLLIN;

    while (poll(fds, 2, -1) > 0) {

        if (fds[1].revents & POLLIN) {
            int n = read(sock, buf, sizeof(buf) - 1);
            if (n <= 0) break;
            buf[n] = '\0';
            printf(C_CYAN "[server] %s" C_RESET, buf);
        }

        if (fds[0].revents & POLLIN) {
            if (!fgets(buf, sizeof(buf), stdin)) break;

            if (strncmp(buf, "/quit", 5) == 0) {
                write(sock, "/quit\n", 6);
                break;
            }

            buf[strcspn(buf, "\n")] = '\0';
            if (strncmp(buf, "SETCODE:", 8) == 0 ||
                strcmp(buf, "GETCODE") == 0) {
                char out[MSG_LEN];
                snprintf(out, sizeof(out), "%s\n", buf);
                write(sock, out, strlen(out));
            } else {
                char out[MSG_LEN];
                snprintf(out, sizeof(out), "MSG:%s\n", buf);
                write(sock, out, strlen(out));
            }
        }
    }

    close(sock);
    printf(C_RED "[owner] Disconnected\n" C_RESET);
    return 0;
}
