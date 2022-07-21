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

// get line from stdin
static int lvsh_getline(char* line) {
    int nrd = read(STDIN_FILENO, line, LINE_LEN);

    if (nrd > 0) {
        if (line[nrd - 1] == '\n') {
            nrd--;
            line[nrd] = '\0';
        }
    }

    return nrd;
}

void lvsh(void) {
    char line[LINE_LEN];
    while (1) {
        printf("$> ");
        int ok = lvsh_getline(line);
        if (ok <= 0) {
            continue;
        }
        if (strcmp(line, "ls") == 0) {
            stat_t *buffer = malloc(4096 * sizeof(stat_t));
            int nfiles = list_files(buffer);
            printf("simplefs_list_files: %d\n", nfiles);
            for (int i = 0; i < nfiles; i++) {
                stat_t *sb = buffer + i;
                printf("%d, %d, %d, %s\n", sb->file_id, sb->size, sb->blocks, sb->filename);
            }
            free(buffer);
        } else if (strcmp(line, "pid") == 0) {
            pid_t pid = getpid();
            printf("pid: %d\n", pid);
        } else if (strcmp(line, "ppid") == 0) {
            pid_t ppid = getppid();
            printf("ppid: %d\n", ppid);
        } else if (strcmp(line, "clear") == 0) {
            clear();
        } else {
            printf("Your input: [%d][%s]\n", ok, line);
            pid_t cpid = create_process(line, NULL);
            printf("cpid = %d\n", cpid);
        }
    }
}

int main() {
    lvsh();
    return 0;
}

