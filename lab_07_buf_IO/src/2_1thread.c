#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

int flag1 = 1, flag2 = 1;

void* thread1_func(void* arg) {
    char c;
    int fd = *(int*)arg;
    while (flag1 == 1 && flag2 == 1) {
        flag1 = read(fd, &c, 1);
        if (flag1 == 1) {
            write(1, &c, 1);
        }
    }
    close(fd);
    return NULL;
}

int main() {
    char c;
    pthread_t t1, t2;
    int fd1 = open("alphabet.txt", O_RDONLY);
    int fd2 = open("alphabet.txt", O_RDONLY);
    pthread_create(&t1, NULL, thread1_func, &fd1);
    while (flag1 == 1 && flag2 == 1) {
        flag2 = read(fd2, &c, 1);
        if (flag2 == 1) {
            write(1, &c, 1);
        }
    }
    close(fd2);
    pthread_join(t1, NULL);
    c = '\n';
    write(1, &c, 1);
    return 0;
}
