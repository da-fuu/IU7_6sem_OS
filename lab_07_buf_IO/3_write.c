#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

void print_stat(char *msg) {
    struct stat statbuf;
    stat("q.txt", &statbuf);
    fprintf(stdout, "%s: inode = %ld, size = %ld bytes\n", msg, statbuf.st_ino, statbuf.st_size);
}

int main() {
    int fd1 = open("q.txt", O_RDWR);
    print_stat("open fd1");
    int fd2 = open("q.txt", O_RDWR);
    print_stat("open fd2");
    for (char c = 'a'; c <= 'z'; c++) {
        if (c % 2)
            write(fd1, &c, 1);
        else
            write(fd2, &c, 1);
        print_stat("write");
    }
    close(fd1);
    print_stat("close fd1");
    close(fd2);
    print_stat("close fd2");
    return 0;
}
