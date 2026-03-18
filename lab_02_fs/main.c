#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

typedef int Myfunc(const char *, const struct stat *, int);
Myfunc myfunc;
int myftw(char *, Myfunc *);
int dopath(Myfunc *);
long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

#define FTW_F 1   /* файл, не являющийся каталогом */
#define FTW_D 2   /* каталог */
#define FTW_DNR 3 /* каталог, который недоступен для чтения */
#define FTW_NS 4  /* файл, информацию о котором невозможно получить с помощью stat */
char *fullpath; /* полный путь к каждому из файлов */
size_t pathlen;

#ifdef PATH_MAX
long pathmax = PATH_MAX;
#else
long pathmax = 0;
#endif
long posix_version = 0;
long xsi_version = 0;
#define PATH_MAX_GUESS 1024

char *path_alloc(size_t *sizep) {
    char *ptr;
    size_t size;
    if (posix_version == 0)
        posix_version = sysconf(_SC_VERSION);
    if (xsi_version == 0)
        xsi_version = sysconf(_SC_XOPEN_VERSION);
    if (pathmax == 0) { /* первый вызов функции */
        errno = 0;
        if ((pathmax = pathconf("/", _PC_PATH_MAX)) == -1) {
            if (errno == 0)
                pathmax = PATH_MAX_GUESS; /* если константа не определена */
            else {
                perror("pathconf");
                exit(1);
            }
        } else {
            pathmax++; /* добавить 1, так как путь относительно корня */
        }
    }
    if ((posix_version < 200112L) && (xsi_version < 4))
        size = pathmax + 1;
    else
        size = pathmax;
    if ((ptr = malloc(size)) == NULL) {
        perror("malloc");
        exit(1);
    }
    if (sizep != NULL)
        *sizep = size;
    return(ptr);
}

int main(int argc, char *argv[]) {
    int ret;
    if (argc != 2) {
        printf("Использование: ftw <начальный_каталог>");
        exit(1);
    }
    ret = myftw(argv[1], myfunc); /* выполняет всю работу */
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1;/* во избежание деления на 0 вывести 0 для всех счетчиков */
    printf("обычные файлы =                          %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
    printf("каталоги =                               %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
    printf("специальные файлы блочных устройств =    %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
    printf("специальные файлы символьных устройств = %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
    printf("FIFO =                                   %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
    printf("символические ссылки =                   %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
    printf("сокеты =                                 %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
    exit(ret);
}

int myftw(char *pathname, Myfunc *func) {
    fullpath = path_alloc(&pathlen); /* выделить память для PATH_MAX+1 байт */
    if (pathlen <= strlen(pathname)) {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL) {
            perror("realloc");
            exit(1);
        }
    }
    strcpy(fullpath, pathname);
    return(dopath(func));
}

int dopath(Myfunc* func) {
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;
    bool fl = 1;

    if (lstat(fullpath, &statbuf) == -1)
        return(func(fullpath, &statbuf, FTW_NS));
    if (S_ISDIR(statbuf.st_mode) == 0) /* не каталог */
        return(func(fullpath, &statbuf, FTW_F));
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);
    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) { /* увеличить размер буфера */
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL) {
            perror("realloc");
            exit(1);
        }
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;
    if ((dp = opendir(fullpath)) == NULL) /* каталог недоступен */
        return(func(fullpath, &statbuf, FTW_DNR));
    while ((dirp = readdir(dp)) != NULL && fl) {
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
            strcpy(&fullpath[n], dirp->d_name); /* добавить имя после слеша */
            if ((ret = dopath(func)) != 0)
                fl = 0; /* выход по ошибке */
        }
    }
    fullpath[n-1] = 0; /* стереть часть строки от слеша и до конца */
    if (closedir(dp) == -1)
        printf("невозможно закрыть каталог %s", fullpath);
    return(ret);
}

int myfunc(const char *pathname, const struct stat *statptr, int type) {
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:   nreg++;     break;
        case S_IFBLK:   nblk++;     break;
        case S_IFCHR:   nchr++;     break;
        case S_IFIFO:   nfifo++;    break;
        case S_IFLNK:   nslink++;   break;
        case S_IFSOCK:  nsock++;    break;
        case S_IFDIR:   printf("признак S_IFDIR для %s", pathname);
        }
        break;
    case FTW_D:
        ndir++;
        break;
    case FTW_DNR:
        printf("закрыт доступ к каталогу %s", pathname);
        break;
    case FTW_NS:
        printf("ошибка вызова функции stat для %s", pathname);
        break;
    default:
        printf("неизвестный тип %d для файла %s", type, pathname);
    }
    return(0);
}
