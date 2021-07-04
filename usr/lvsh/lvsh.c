/**
 * @file lvsh.c
 * @brief A simple shell for myOS. Name after my girlfriend.
 */

#include <stdio.h>

void lvsh() {
    // char cmd[512];
    while (1) {
        printf("$> ");
        char c = 0;
        // int idx = 0;
        while (c != '\n') {
            c = getchar();
            // if (idx >= 511) {
            //     continue;
            // }
            // putchar(c);
            // if (c == '\n')
            // cmd[idx++] = c;
        }
        // cmd[idx] = '\0';
    }
}

