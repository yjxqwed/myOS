/**
 * @file lvsh.c
 * @brief A simple shell for myOS. Name after my girlfriend.
 */

#include <stdio.h>
#include <unistd.h>
#include <common/types.h>
#include <common/debug.h>
#include <lib/string.h>
#include <stdlib.h>

#define LINE_LEN 128
#define NR_ARGS 16

// get line from stdin
static int lvsh_getline(char* line) {
    int nrd = read(STDIN_FILENO, line, LINE_LEN);

    if (nrd > 0) {
        nrd--;
        line[nrd] = '\0';
    }

    return nrd;
}

int parse_cmd(char *line, char *argv[]) {
    int i = 0;
    int argc = 0;
    while (line[i] != '\0' && argc < NR_ARGS) {
        while (line[i] == ' ') {
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        argc++;
        int j = i;
        while (line[j] != ' ' && line[j] != '\0') {
            j++;
        }
        // [i, j) is a cmd;
        if (j > i) {
            argv[argc - 1] = (char *)malloc(j - i + 1);
            memset(argv[argc - 1], 0, j - i + 1);
            memcpy(line + i, argv[argc - 1], j - i);
        }
        i = j;
    }
    return argc;
}

void lvsh(void) {
    char line[LINE_LEN] = {'\0'};
    char *argv[NR_ARGS + 1] = {NULL};
    int argc = 0;
    while (1) {
        printf("$> ");
        int ok = lvsh_getline(line);
        if (ok <= 0) {
            continue;
        }
        argc = parse_cmd(line, argv);
        if (argc <= 0) {
            continue;
        }
        // for (int i = 0; i < argc; i++) {
        //     printf("%d: {%s}\n", i, argv[i]);
        // }
        if (strcmp(argv[0], "ls") == 0) {
            stat_t *buffer = malloc(4096 * sizeof(stat_t));
            int nfiles = list_files(buffer);
            printf("simplefs_list_files: %d\n", nfiles);
            for (int i = 0; i < nfiles; i++) {
                stat_t *sb = buffer + i;
                printf("%d, %d, %d, %s\n", sb->file_id, sb->size, sb->blocks, sb->filename);
            }
            free(buffer);
        } else if (strcmp(argv[0], "pid") == 0) {
            pid_t pid = getpid();
            printf("pid: %d\n", pid);
        } else if (strcmp(argv[0], "ppid") == 0) {
            pid_t ppid = getppid();
            printf("ppid: %d\n", ppid);
        } else if (strcmp(argv[0], "clear") == 0) {
            clear();
        } else if (strcmp(argv[0], "ps") == 0) {
            task_info_t *tis = malloc(10 * sizeof(task_info_t));
            int nps = ps(tis, 10);
            printf("PID, PPID, TIME, TTY, STATUS, NAME\n");
            for (int i = 0; i < nps; i++) {
                printf(
                    "%d, %d, %d, %d, %d, %s\n",
                    tis[i].task_id, tis[i].parent_id,
                    tis[i].elapsed_ticks, tis[i].tty_no,
                    tis[i].status, tis[i].task_name
                );
            }
            printf("%d tasks printed.\n", nps);
            free(tis);
        } else if (strcmp(argv[0], "mm") == 0) {
            mm_info_t mm_info;
            mm(&mm_info);
            printf("MEM: total=%d, used=%d, free=%d\n", mm_info.total_mem_installed, mm_info.used_mem, mm_info.free_mem);
        } else if (strcmp(argv[0], "sleep") == 0) {
            sleep(2000);
        } else if (strcmp(argv[0], "exit") == 0) {
            break;
        } else {
            if (stat(argv[0], NULL) != 0) {
                printf("lvsh: %s: No such file\n", argv[0]);
            } else {
                // printf("Your input: [%d][%s]\n", ok, argv[0]);
                pid_t cpid = create_process(argv[0], argc, argv);
                if (cpid < 0) {
                    printf("lvsh: %s: Cannot execute\n", argv[0]);
                } else {
                    int cexit_stat = 0;
                    wait(&cexit_stat);
                    printf("lvsh: cpid = %d, ces = %d\n", cpid, cexit_stat);
                }
            }
        }
        for (int i = 0; i < NR_ARGS; i++) {
            free(argv[i]);
            argv[i] = NULL;
        }
    }
}

int main(int argc, char *argv[]) {
    lvsh();
    return 0;
}

