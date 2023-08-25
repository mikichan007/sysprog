#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "console.h"


const int MAX_COMMAND_LENGTH = 1024;


// Source: https://stackoverflow.com/questions/314401/how-to-read-a-line-from-the-console-in-c
char *readline() {
    char* line = malloc(100);
    char* linep = line;
    int lenmax = 100, len = lenmax, c;

    if (line == NULL) return NULL;

    for (;;) {
        c = fgetc(stdin);
        if (c == EOF) break;

        if (--len == 0) {
            len = lenmax;
            char* linen = realloc(linep, lenmax *= 2);

            if (linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if ((*line++ = c) == '\n') break;
    }

    *(line - 1) = '\0';
    return linep;
}


int execute_command(struct cmd* cmd) {
    if (fork() == 0) {        
        int ret_val = execvp(cmd->name, (char *const *) cmd->argv);
        if (ret_val == -1) {
            printf("Note: process did not terminate correctly\n");
        }
        return 1;
    }
    wait(NULL);
    return 0;
}


void console_main_loop() {
    char* line;

    for (;;) {
        printf("$> ");
        line = readline();

        if (line == NULL) {
            printf("An error occurred while reading the command\n");
            continue;
        }

        struct cmd* cmd = parse_cmd(line);

        if (strcmp(cmd->name, "exit") == 0) break;

        if (execute_command(cmd)) return;

        delete_cmd(cmd);
        free(line);
    }

    printf("Exiting...\n");
}
