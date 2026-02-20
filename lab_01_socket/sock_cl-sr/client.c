#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    char buf[64];
    struct sockaddr server_addr;
    int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }
    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, "myserver.sock");
    sprintf(buf, "%d", getpid());
    printf("send: %s\n", buf);
    if (sendto(sock_fd, buf, strlen(buf), 0, &server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        exit(1);
    }
    close(sock_fd);
    exit(0);
}
