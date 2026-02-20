#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define SOCK_NAME "myserver.sock"
int sock_fd;

void sigint_handler(int signum) {
    close(sock_fd);
    unlink(SOCK_NAME);
    exit(0);
}

int main() {
    struct sockaddr server_addr, client_addr;
    socklen_t client_addr_len;
    char buf[64];
    int bytes_read;
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }
    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, "myserver.sock");
    if (bind(sock_fd, &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }
    while(1) {
        bytes_read = recvfrom(sock_fd, buf, sizeof(buf), 0, &client_addr, &client_addr_len);
        if (bytes_read == -1) {
            perror("recvfrom");
            exit(1);
        }
        else {
            buf[bytes_read] = '\0';
            printf("recieve: %s\n", buf);
        }
    }
    exit(0);
}
