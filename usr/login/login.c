#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    pid_t ppid = getppid();
    if (ppid != 0) {
        printf("login: Don't run this binary!\n");
        return -1;
    }
    char line[128];
    while (1) {
        printf("login: ");
        read(STDIN_FILENO, line, 128);
        create_process("lvsh", 0, NULL);
        int cexit_stat = 0;
        wait(&cexit_stat);
    }
    return 0;
}

