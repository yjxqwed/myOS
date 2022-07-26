#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("USAGE: cat file1 file2 ...\n");
    } else {
        stat_t s;
        for (int i = 1; i < argc; i++) {
            char *file = argv[i];
            if (stat(file, &s) != 0) {
                printf("cat: %s: No such file\n", file);
            } else {
                void *buffer = malloc(s.size);
                int fd = open(file, O_RDONLY);
                read(fd, buffer, s.size);
                write(STDOUT_FILENO, buffer, s.size);
                free(buffer);
            }
        }
    }
    return 0;
}
