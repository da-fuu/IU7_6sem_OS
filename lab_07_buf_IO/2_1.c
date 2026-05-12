#include <fcntl.h>
#include <unistd.h>

int main() {
    char c;
    int flag1 = 1, flag2 = 1;
    int fd1 = open("alphabet.txt", O_RDONLY);
    int fd2 = open("alphabet.txt", O_RDONLY);
    while (flag1 == 1 && flag2 == 1) {
        flag1 = read(fd1, &c, 1);
        if (flag1 == 1) {
            write(1, &c, 1);
            flag2 = read(fd2, &c, 1);
            if (flag2 == 1)
                write(1, &c, 1);
        }
    }
    close(fd1);
    close(fd2);
    c = '\n';
    write(1, &c, 1);
    return 0;
}
