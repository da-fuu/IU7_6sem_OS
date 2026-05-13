#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

void print_stat(char *msg) {
    struct stat statbuf;
    stat("q.txt", &statbuf);
    fprintf(stdout, "%s: inode = %ld, size = %ld bytes\n", msg, statbuf->st_ino, statbuf->st_size);
}

int main() {
    FILE *fs1 = fopen("q.txt", "w");
    print_stat("fopen fs1");
    FILE *fs2 = fopen("q.txt", "w");
    print_stat("fopen fs2");
    for (char c = 'a'; c <= 'z'; c++) {
        if (c % 2)
            fprintf(fs1, "%c", c);
        else
            fprintf(fs2, "%c", c);
        print_stat("fprintf");
    }
    fclose(fs1);
    print_stat("fclose fs1");
    fclose(fs2);
    print_stat("fclose fs2");
    return 0;
}
