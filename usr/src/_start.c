#include <unistd.h>

extern int main();

void _start() {
    _exit(main());
}