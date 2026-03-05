#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>

#define SERV_PORT 9878
#define BUF_LEN 50
#define SB 0
#define SE 1
#define SF 2
#define P -1
#define V  1

struct sembuf start_produce[2] = { {SE, P, 0}, {SB, P, 0} };
struct sembuf stop_produce[2] =  { {SB, V, 0}, {SF, V, 0} };
struct sembuf start_consume[2] = { {SF, P, 0}, {SB, P, 0} };
struct sembuf stop_consume[2] =  { {SB, V, 0}, {SE, V, 0} };
char letter = 'a';
char buffer[BUF_LEN];
char *prod_ptr;
char *cons_ptr;
int listenfd;
int semfd;

void producer(int connfd) {
    char buf[1];
    if (semop(semfd, start_produce, 2) == -1) {
        perror("semop");
        exit(1);
    }
    *prod_ptr = letter;
    printf("Produce: %c ", letter);
    if (letter == 'z')
        letter = 'a';
    else
        letter++;
    if (prod_ptr == buffer + BUF_LEN - 1)
        prod_ptr = buffer;
    else
        prod_ptr++;
    if (semop(semfd, stop_produce, 2) == -1) {
        perror("semop");
        exit(1);
    }
    buf[0] = 'o';
    printf("send: %c\n", buf[0]);
    if (send(connfd, &buf, sizeof(buf), 0) == -1) {
        perror("send");
        exit(1);
    }
}

void consumer(int connfd) {
    char buf[1];
    if (semop(semfd, start_consume, 2) == -1) {
        perror("semop");
        exit(1);
    }
    buf[0] = *cons_ptr;
    if (cons_ptr == buffer + BUF_LEN - 1)
        cons_ptr = buffer;
    else
        cons_ptr++;
    if (semop(semfd, stop_consume, 2) == -1) {
        perror("semop");
        exit(1);
    }
    printf("Consume and send: %c\n", buf[0]);
    if (send(connfd, &buf, sizeof(buf), 0) == -1) {
        perror("send");
        exit(1);
    }
}

void* process_client(void *args) {
    int connfd = *(int *)&args;
    char buf[10];

    printf("Connected\n");
    if (recv(connfd, buf, sizeof(buf), 0) == -1) {
        perror("recv");
        exit(1);
    }
    if (buf[0] == 'p')
        producer(connfd);
    else if (buf[0] == 'c')
        consumer(connfd);
    else
        printf("Unknown request: %c\n", buf[0]);
    close(connfd);
    return NULL;
}

void sigint_handler(int signum) {
    close(listenfd);
    if (semctl(semfd, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(1);
    }
    exit(0);
}

int main(void) {
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    key_t semkey;
    int connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr = { 0 };
    pthread_t th;

    if ((semkey = ftok("./key.txt", 1)) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((semfd = semget(semkey, 3, IPC_CREAT | perms)) == -1) {
        perror("semget");
        exit(1);
    }
    if (semctl(semfd, SB, SETVAL, 1) == -1) {
        perror("semctl");
        exit(1);
    }
    if (semctl(semfd, SE, SETVAL, BUF_LEN) == -1) {
        perror("semctl");
        exit(1);
    }
    if (semctl(semfd, SF, SETVAL, 0) == -1) {
        perror("semctl");
        exit(1);
    }
    prod_ptr = buffer;
    cons_ptr = buffer;
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(1);
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(listenfd, 1024) == -1) {
        perror("listen");
        exit(1);
    }
    while (1) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd == -1) {
            perror("accept");
            exit(1);
        }
        if (pthread_create(&th, NULL, process_client, (void*)connfd) == -1) {
            perror("pthread_create");
            exit(1);
        }
        if (pthread_detach(th) == -1) {
            perror("pthread_detach");
            exit(1);
        }
    }
    exit(0);
}
