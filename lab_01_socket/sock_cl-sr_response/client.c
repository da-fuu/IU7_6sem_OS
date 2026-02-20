#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    char buf[64];
    struct sockaddr server_addr, client_addr;
    socklen_t server_addr_len;
    int bytes_read;
    int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }
    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, "myserver.sock");
    client_addr.sa_family = AF_UNIX;
    sprintf(client_addr.sa_data, "%d.sock", getpid());
    if (bind(sock_fd, &client_addr, sizeof(client_addr)) == -1) {
        perror("bind");
        exit(1);
    }
    sprintf(buf, "%d", getpid());
    printf("send: %s\n", buf);
    if (sendto(sock_fd, buf, strlen(buf), 0, &server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        exit(1);
    }
    bytes_read = recvfrom(sock_fd, buf, sizeof(buf), 0, &server_addr, &server_addr_len);
    if (bytes_read == -1) {
        perror("recvfrom");
        exit(1);
    }
    buf[bytes_read] = '\0';
    printf("recieve: %s\n", buf);
    sleep(3);
    close(sock_fd);
    unlink(client_addr.sa_data);
    exit(0);
}
