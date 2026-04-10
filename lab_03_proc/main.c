#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>


void read_pagemap(FILE *output, const char *path, const char *desc) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(output, "Error opening %s\n", path);
        return;
    }

    fprintf(output, "\n[%s]\n", desc);

    uint64_t buffer[512];  // 512 * 8 байт = 4096 байт
    ssize_t bytes_read;

    int i = 0;

    while (i++ < 5 && ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)) {
        for (size_t i = 0; i < bytes_read / sizeof(uint64_t); i++) {
            fprintf(output, "%016lx\n", buffer[i]);
        }
    }

    if (bytes_read == -1) {
        fprintf(output, "Error reading %s\n", path);
    }

    close(fd);
}


void read_file(FILE *output, const char *path, const char *desc)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        fprintf(output, "Error opening %s\n", path);
        return;
    }

    fprintf(output, "\n[%s]\n", desc);

    char buffer[4096];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            if (buffer[i] == '\0')
            {
                fputc('\n', output);
            }
            else
            {
                fputc(buffer[i], output);
            }
        }
    }

    fclose(file);
}

void read_maps(FILE *output, const char *path, const char *desc)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        fprintf(output, "Error opening %s\n", path);
        return;
    }
    fprintf(output, "\n[%s]\n", desc);
    char buffer[4096];

    unsigned long st, end;

    unsigned long long first_start = 0;
    unsigned long long last_end = 0;
    int is_first = 1;

    long sum_pages = 0;
    unsigned long long sum = 0;
    long page_sz = getpagesize();

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (sscanf(buffer, "%lx-%lx", &st, &end) == 2)
        {
            if (is_first)
            {
                first_start = st;
                is_first = 0;
            }
            last_end = end;

            unsigned long size = end - st;
            fprintf(output, "%5ld  ", size / page_sz);
            fprintf(output, "%s", buffer);
            sum += size;
            sum_pages += size / page_sz;
        }
    }

    unsigned long long continuous_span = 0;
    double terabytes = 0.0;

    if (!is_first)
    {
        continuous_span = last_end - first_start;
        terabytes = (double)continuous_span / (1024ULL * 1024 * 1024 * 1024);
    }

    // Вывод в файл
    fprintf(output, "Total pages: %ld\nTotal vsize: %llu bytes\n", sum_pages, sum);
    fprintf(output, "address space: %.2f TB\n", terabytes);

    // Вывод в консоль
    printf("Total pages: %ld\nTotal vsize: %llu bytes\n", sum_pages, sum);
    printf("address space: %.2f TB\n", terabytes);

    fclose(file);
}

void read_symlink(FILE *output, const char *path, const char *desc)
{
    char buffer[PATH_MAX];
    ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        fprintf(output, "\n[%s]: %s\n", desc, buffer);
    }
    else
    {
        fprintf(output, "Error reading %s\n", path);
    }
}

void list_dir(FILE *output, const char *path, const char *desc)
{
    fprintf(output, "\n[%s]:\n", desc);
    DIR *dir = opendir(path);
    if (!dir)
    {
        fprintf(output, "Error opening %s\n", path);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        fprintf(output, "%s\n", entry->d_name);
    }
    closedir(dir);
}

void read_stat(FILE *output, const char *pid)
{
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);

    FILE *file = fopen(path, "r");
    if (!file)
    {
        fprintf(output, "Error opening %s\n", path);
        return;
    }

    long pid_val, ppid, pgrp, session, tty_nr, tpgid, flags;
    char comm[256], state;
    unsigned long minflt, cminflt, majflt, cmajflt, utime, stime, cutime, cstime;
    long priority, nice, num_threads, itrealvalue, starttime, vsize, rss, rsslim;
    unsigned long startcode, endcode, startstack, kstkesp, kstkeip, signal;
    unsigned long blocked, sigignore, sigcatch, wchan, nswap, cnswap;
    int exit_signal, processor, rt_priority, policy;
    unsigned long delayacct_blkio_ticks, guest_time, cguest_time;
    unsigned long start_data, end_data, start_brk, arg_start, arg_end, env_start, env_end, exit_code;

    fscanf(file, "%ld %255s %c %ld %ld %ld %ld %ld %ld %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %lu %lu %lu %lu %lu %lu %lu %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %d",
           &pid_val, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags,
           &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime, &cutime, &cstime,
           &priority, &nice, &num_threads, &itrealvalue, &starttime, &vsize, &rss, &rsslim,
           &startcode, &endcode, &startstack, &kstkesp, &kstkeip, &signal, &blocked, &sigignore,
           &sigcatch, &wchan, &nswap, &cnswap, &exit_signal, &processor, &rt_priority,
           &policy, &delayacct_blkio_ticks, &guest_time, &cguest_time, &start_data, &end_data,
           &start_brk, &arg_start, &arg_end, &env_start, &env_end, &exit_code);

    fclose(file);

    fprintf(output, "\n[Process Stat]\n");
    fprintf(output, "1. PID: %ld\n", pid_val);
    fprintf(output, "2. Command: %s\n", comm);
    fprintf(output, "3. State: %c\n", state);
    fprintf(output, "4. PPID: %ld\n", ppid);
    fprintf(output, "5. PGRP: %ld\n", pgrp);
    fprintf(output, "6. Session: %ld\n", session);
    fprintf(output, "7. TTY_NR: %ld\n", tty_nr);
    fprintf(output, "8. Tpgid: %ld\n", tpgid);
    fprintf(output, "9. Flags: %ld\n", flags);
    fprintf(output, "10. Minflt: %lu\n", minflt);
    fprintf(output, "11. Cminflt: %lu\n", cminflt);
    fprintf(output, "12. Majflt: %lu\n", majflt);
    fprintf(output, "13. Cmajflt: %lu\n", cmajflt);
    fprintf(output, "14. Utime: %lu\n", utime);
    fprintf(output, "15. Stime: %lu\n", stime);
    fprintf(output, "16. Cutime: %lu\n", cutime);
    fprintf(output, "17. Cstime: %lu\n", cstime);
    fprintf(output, "18. Priority: %ld\n", priority);
    fprintf(output, "19. Nice: %ld\n", nice);
    fprintf(output, "20. Num Threads: %ld\n", num_threads);
    fprintf(output, "21. Starttime: %ld\n", starttime);
    fprintf(output, "22. Vsize: %ld\n", vsize);
    fprintf(output, "23. Rss: %ld\n", rss);
    fprintf(output, "24. Rsslim: %ld\n", rsslim);
    fprintf(output, "25. Startcode: %lu (%lx)\n", startcode, startcode);
    fprintf(output, "26. Endcode: %lu (%lx)\n", endcode, endcode);
    fprintf(output, "27. Startstack: %lu (%lx)\n", startstack, startstack);
    fprintf(output, "28. Kstkesp: %lu\n", kstkesp);
    fprintf(output, "29. Kstkeip: %lu\n", kstkeip);
    fprintf(output, "30. Signal: %lu\n", signal);
    fprintf(output, "31. Blocked: %lu\n", blocked);
    fprintf(output, "32. Sigignore: %lu\n", sigignore);
    fprintf(output, "33. Sigcatch: %lu\n", sigcatch);
    fprintf(output, "34. Wchan: %lu\n", wchan);
    fprintf(output, "35. Nswap: %lu\n", nswap);
    fprintf(output, "36. Cnswap: %lu\n", cnswap);
    fprintf(output, "37. Exit_signal: %d\n", exit_signal);
    fprintf(output, "38. Processor: %d\n", processor);
    fprintf(output, "39. Rt_priority: %d\n", rt_priority);
    fprintf(output, "40. Policy: %d\n", policy);
    fprintf(output, "41. Delayacct_blkio_ticks: %lu\n", delayacct_blkio_ticks);
    fprintf(output, "42. Guest_time: %lu\n", guest_time);
    fprintf(output, "43. Cguest_time: %lu\n", cguest_time);
    fprintf(output, "44. Start_data: %lu (%lx)\n", start_data, start_data);
    fprintf(output, "45. End_data: %lu (%lx)\n", end_data, end_data);
    fprintf(output, "46. Start_brk: %lu (%lx)\n", start_brk, start_brk);
    fprintf(output, "47. Arg_start: %lu (%lx)\n", arg_start, arg_start);
    fprintf(output, "48. Arg_end: %lu (%lx)\n", arg_end, arg_end);
    fprintf(output, "49. Env_start: %lu (%lx)\n", env_start, env_start);
    fprintf(output, "50. Env_end: %lu (%lx)\n", env_end, env_end);
    fprintf(output, "51. Exit_code: %d\n", exit_code);
}

typedef struct {
    uint64_t pfn : 55;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;


int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
    size_t nread;
    ssize_t ret;
    uint64_t data;

    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(pagemap_fd, ((uint8_t*)&data) + nread, sizeof(data) - nread,
                (vaddr / sysconf(_SC_PAGE_SIZE)) * sizeof(data) + nread);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->pfn = data & (((uint64_t)1 << 55) - 1);
    entry->soft_dirty = (data >> 55) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;
    return 0;
}

int fprintf_pagemap_info(const int pid, FILE *out) {
    char buffer[BUFSIZ];
    long int buf[1000];
    char maps_file[BUFSIZ];
    char pagemap_file[BUFSIZ];
    int maps_fd;
    int offset = 0;
    int pagemap_fd;

    snprintf(maps_file, sizeof(maps_file), "/proc/%ju/maps", (uintmax_t)pid);
    snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
    maps_fd = open(maps_file, O_RDONLY);
    if (maps_fd < 0) {
        perror("open maps");
        return EXIT_FAILURE;
    }
    pagemap_fd = open(pagemap_file, O_RDONLY);
    if (pagemap_fd < 0) {
        perror("open pagemap");
        return EXIT_FAILURE;
    }
    fprintf(out, "\nPAGEMAP:\n");
    int r = 0;
    fprintf(out, "addr\t\t\tpfn\t\t\tsoft-dirty\tfile/shared swapped present library\n");
    for (;;) {
        ssize_t length = read(maps_fd, buffer + offset, sizeof buffer - offset);
        if (length <= 0) break;
        length += offset;
        for (size_t i = offset; i < (size_t)length; i++) {
            uintptr_t low = 0, high = 0;
            if (buffer[i] == '\n' && i) {
                const char *lib_name;
                size_t y;
                /* Parse a line from maps. Each line contains a range that contains many pages. */
                {
                    size_t x = i - 1;
                    while (x && buffer[x] != '\n') x--;
                    if (buffer[x] == '\n') x++;
                    while (buffer[x] != '-' && x < sizeof buffer) {
                        char c = buffer[x++];
                        low *= 16;
                        if (c >= '0' && c <= '9') {
                            low += c - '0';
                        } else if (c >= 'a' && c <= 'f') {
                            low += c - 'a' + 10;
                        } else {
                            break;
                        }
                    }
                    while (buffer[x] != '-' && x < sizeof buffer) x++;
                    if (buffer[x] == '-') x++;
                    while (buffer[x] != ' ' && x < sizeof buffer) {
                        char c = buffer[x++];
                        high *= 16;
                        if (c >= '0' && c <= '9') {
                            high += c - '0';
                        } else if (c >= 'a' && c <= 'f') {
                            high += c - 'a' + 10;
                        } else {
                            break;
                        }
                    }
                    lib_name = 0;
                    for (int field = 0; field < 4; field++) {
                        x++;
                        while(buffer[x] != ' ' && x < sizeof buffer) x++;
                    }
                    while (buffer[x] == ' ' && x < sizeof buffer) x++;
                    y = x;
                    while (buffer[y] != '\n' && y < sizeof buffer) y++;
                    buffer[y] = 0;
                    lib_name = buffer + x;
                }
                /* Print Amount of Pages */
                buf[r++] =  (uintmax_t)((high - low) / sysconf(_SC_PAGE_SIZE));
                fprintf(out, "Pages: %ju\n", (uintmax_t)((high - low) / sysconf(_SC_PAGE_SIZE)));

                /* Get info about all pages in this page range with pagemap. */
                {
                    PagemapEntry entry;
                    for (uintptr_t addr = low; addr < high; addr += sysconf(_SC_PAGE_SIZE)) {
                        if (!pagemap_get_entry(&entry, pagemap_fd, addr)) {
                            fprintf(out, "%jx\t%6jx\t\t\t%u\t\t\t%u\t\t%u\t\t%u\t%s\n",
                                (uintmax_t)addr,
                                (uintmax_t)entry.pfn,
                                entry.soft_dirty,
                                entry.file_page,
                                entry.swapped,
                                entry.present,
                                lib_name
                            );
                        }
                    }
                }
                buffer[y] = '\n';
            }
        }
    }
    close(maps_fd);
    close(pagemap_fd);

    return 0;
}

void print_mem(int pid, const char *output_file) {
    char mem_path[256], maps_path[256];
    snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    int mem_fd = open(mem_path, O_RDONLY);
    FILE *maps = fopen(maps_path, "r");
    FILE *out = fopen(output_file, "w");

    if (mem_fd < 0 || !maps || !out) {
        perror("Ошибка при открытии файлов");
        if (mem_fd >= 0) close(mem_fd);
        if (maps) fclose(maps);
        if (out) fclose(out);
        return;
    }

    char line[256];
    uint64_t start, end;
    char perm[5];
    int i = 0;
    while (fgets(line, sizeof(line), maps)) {
        i++;
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perm) != 3 || !(strstr(line, "[stack]") || i == 2 || i == 3))
            continue;

        if (perm[0] != 'r')
            continue;
        lseek(mem_fd, start, SEEK_SET);
        uint64_t size = end - start;
        unsigned char buffer[BUFSIZ];

        while (size > 0) {
            ssize_t bytes_read = read(mem_fd, buffer, (size > BUFSIZ) ? BUFSIZ : size);
            if (bytes_read <= 0)
                break;

            for (ssize_t i = 0; i < bytes_read; i++) {
                fprintf(out, "%.2x ", buffer[i]);
                if ((i + 1) % 16 == 0) fprintf(out, "\n");
            }

            size -= bytes_read;
        }
        fprintf(out, "\n");
    }

    close(mem_fd);
    fclose(maps);
    fclose(out);
}


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <pid> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *pid = argv[1];
    if (mkdir(argv[2], 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkdir");
            exit(1);
        }
    }
    if (chdir(argv[2]) == -1)
    {
        perror("chdir");
        exit(1);
    }
    FILE *output = fopen("out.txt", "w");
    if (!output)
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    char path[PATH_MAX];

    // Check if process exists
    snprintf(path, sizeof(path), "/proc/%s", pid);
    if (access(path, F_OK) != 0)
    {
        fprintf(stderr, "Process %s not found\n", pid);
        fclose(output);
        exit(EXIT_FAILURE);
    }

    // Read environment variables
    snprintf(path, sizeof(path), "/proc/%s/environ", pid);
    read_file(output, path, "Process Environment Variables");
    // Read stat file
    // fprintf(output, "\n[Process Stat]\n");
    read_stat(output, pid);

    // Read command line arguments
    snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);
    read_file(output, path, "Command Line Arguments");

    // Read file descriptors
    snprintf(path, sizeof(path), "/proc/%s/fd", pid);
    list_dir(output, path, "File Descriptors");

    // Read current working directory
    snprintf(path, sizeof(path), "/proc/%s/cwd", pid);
    read_symlink(output, path, "Current Working Directory");

    // Read executable path
    snprintf(path, sizeof(path), "/proc/%s/exe", pid);
    read_symlink(output, path, "Executable Path");

    // Read root directory
    snprintf(path, sizeof(path), "/proc/%s/root", pid);
    read_symlink(output, path, "Root Directory");

    // Read I/O statistics
    snprintf(path, sizeof(path), "/proc/%s/io", pid);
    read_file(output, path, "I/O Statistics");
    // Read process name
    snprintf(path, sizeof(path), "/proc/%s/comm", pid);
    read_file(output, path, "Process Name");

    // Read task subdirectory
    snprintf(path, sizeof(path), "/proc/%s/task", pid);
    list_dir(output, path, "Task Subdirectory");

    // Read memory map
    FILE *f0 = fopen( "maps.txt", "w");
    snprintf(path, sizeof(path), "/proc/%s/maps", pid);
    read_maps(f0, path, "Memory Map");
    fclose(f0);

    // Read page map
    FILE *f1 = fopen( "pagemap.txt", "w");
    fprintf_pagemap_info(atoi(pid), f1);
    fclose(f1);

    fclose(output);
    return 0;
}
