#include <fcntl.h>
#include <stdio.h>

int main() {
    FILE *fs1, *fs2;
    char buff1[20], buff2[20], c;
    int flag1 = 1, flag2 = 1;
    int fd = open("alphabet.txt", O_RDONLY);
    fs1 = fdopen(fd, "r");
    setvbuf(fs1, buff1, _IOFBF, 20);
    fs2 = fdopen(fd, "r");
    setvbuf(fs2, buff2, _IOFBF, 20);
    while (flag1 == 1 || flag2 == 1) {
        flag1 = fscanf(fs1, "%c", &c);
        if (flag1 == 1)
            fprintf(stdout, "%c", c);
        flag2 = fscanf(fs2, "%c", &c);
        if (flag2 == 1)
            fprintf(stdout, "%c", c);
    }
    fprintf(stdout, "\n");
    return 0;
}
