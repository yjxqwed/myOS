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
    char c = 0;
    int id = 0;
    while (1) {
        if (-1 == read(STDIN_FILENO, &c, 1)) {
            return -1;
        }
        if (c == '\b' && id > 0) {
            id--;
        } else if (c == '\n') {
            line[id] = '\0';
            break;
        } else {
            line[id++] = c;
        }
    }
    return id;
}

void lvsh(void) {
    char line[LINE_LEN];
    while (1) {
        printf("$> ");
        int ok = lvsh_getline(line);
        if (strcmp(line, "ls") == 0) {
            stat_t *buffer = line;
            int nfiles = list_files(buffer);
            printf("simplefs_list_files: %d\n", nfiles);
            for (int i = 0; i < nfiles; i++) {
                stat_t *sb = buffer + i;
                printf("%d, %d, %d, %s\n", sb->file_id, sb->size, sb->blocks, sb->filename);
            }
        } else {
            printf("Your input: [%d][%s]\n", ok, line);
        }
    }
}

