#include <unistd.h>
#include <myos.h>
#include <mm/vmm.h>

extern int main(int argc, char *argv[]);

void _start() {
    cmd_args_t *cmd_args = (cmd_args_t *)USER_ARGS;
    _exit(main(cmd_args->argc, cmd_args->argv));
}