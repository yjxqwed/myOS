#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("nothing: pid=%d, ppid=%d\n", pid, ppid);
    while (1) {}
    return 0;
}
