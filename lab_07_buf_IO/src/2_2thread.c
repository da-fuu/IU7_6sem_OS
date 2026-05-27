#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

int flag1 = 1, flag2 = 1;

void* thread1_func(void* arg) {
    char c;
    int fd = open("alphabet.txt", O_RDONLY);
    while (flag1 == 1 && flag2 == 1) {
        flag1 = read(fd, &c, 1);
        if (flag1 == 1) {
            write(1, &c, 1);
        }
    }
    close(fd);
    pthread_exit(NULL);
}

void* thread2_func(void* arg) {
    char c;
    int fd = open("alphabet.txt", O_RDONLY);
    while (flag1 == 1 && flag2 == 1) {
        flag2 = read(fd, &c, 1);
        if (flag2 == 1) {
            write(1, &c, 1);
        }
    }
    close(fd);
    pthread_exit(NULL);
}

int main() {
    pthread_t t1, t2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t1, &attr, thread1_func, NULL);
    pthread_create(&t2, &attr, thread2_func, NULL);
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
}
