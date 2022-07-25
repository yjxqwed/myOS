#include <stdio.h>
#include <unistd.h>

#include <common/debug.h>

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("nothing: pid=%d, ppid=%d\n", pid, ppid);
    printf("nothing: argc=%d\n", argc);
    printf("nothing: argv=0x%X\n", argv);
    MAGICBP;
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: {%s}\n", i, argv[i]);
    }
    return 0;
}
