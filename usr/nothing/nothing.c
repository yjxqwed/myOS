#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <common/debug.h>

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("nothing: pid=%d, ppid=%d\n", pid, ppid);
    printf("nothing: argc=%d\n", argc);
    printf("nothing: argv=0x%X\n", argv);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: {%s}\n", i, argv[i]);
    }
    if (argc >= 2 && strcmp(argv[1], "pagefault") == 0) {
        *(int *)NULL = 12;
    }
    return argc;
}
