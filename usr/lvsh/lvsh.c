/**
 * @file lvsh.c
 * @brief A simple shell for myOS. Name after my girlfriend.
 */

#include <stdio.h>
#include <unistd.h>
#include <common/types.h>
#include <common/debug.h>

#define LINE_LEN 128

// get line from stdin
static int lvsh_getline(char* line) {
    char c = 0;
    int id = 0;
    do {
        if (-1 == read(STDIN_FILENO, &c, 1)) {
            return -1;
        }
        if (c == '\b' && id > 0) {
            id--;
        } else {
            line[id++] = c;
        }
    } while (c != '\n');
    line[id] = 0;
    return id;
}

void lvsh(void) {
    char line[LINE_LEN];
    while (1) {
        printf("$> ");
        int ok = lvsh_getline(line);
        printf("Your input: [%d][%s]\n", ok, line);
    }
}

