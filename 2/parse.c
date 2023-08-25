#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parse.h"

// TODO: remove
#include <stdio.h>


const char* PIPE = "|";
const char* AND = "&&";
const char* OR = "||";

const char* DELIMITERS = " \n\t";


struct token {
    char* str;
    int len;
};

struct token* new_token() {
    struct token* token = malloc(sizeof(struct token *));
    token->str = NULL;
    token->len = 0;
    return token;
}

struct cmd* new_cmd() {
    return malloc(sizeof(struct cmd));
}

void add_symbol(struct token* token, char symbol) {
    token->str = realloc(token->str, ++token->len * sizeof(char));
    token->str[token->len - 1] = symbol;
}

void add_token(struct cmd* cmd, struct token* token) {
    if (cmd->name == NULL) {
        // TODO: endsymbol?
        cmd->name = token->str;
        return;
    }

    // cmd->argv
}

void add_cmd(struct conveyor* conveyor, struct cmd* cmd, char* op) {
    conveyor->commands = realloc(conveyor->commands, ++conveyor->cn * sizeof(struct cmd**));
    conveyor->commands[conveyor->cn - 1] = cmd;
    if (conveyor->cn > 1) { 
        conveyor->op[conveyor->cn - 2] = op;
    }
}

struct conveyor* new_conveyor() {
    struct conveyor* conveyor = malloc(sizeof(struct conveyor));
    conveyor->commands = malloc(sizeof(struct cmd *));
    conveyor->op = malloc(sizeof(enum operator *));
    conveyor->cn = 0;
    return conveyor;
}

struct parse_state *new_parse_state() {
    struct parse_state* parse_state = malloc(sizeof(struct parse_state));
    parse_state->conveyor = new_conveyor();
    parse_state->error = false;
    parse_state->shielding = false;
    parse_state->single_quote = false;
    parse_state->double_quote = false;
    parse_state->cmdand = false;
    parse_state->cmdor = false;
    parse_state->pipe = false;
}

void parse_cmd_v2(struct parse_state* state, char *command) {    
    const char **argv = malloc(sizeof(char *));

    struct token* token = new_token();
    struct conveyor* conveyor = new_conveyor();
    struct cmd* cmd = new_cmd();

    for (int i = 0; i < strlen(command); i++) {
        switch (command[i])
        {
        case '\'':
            if (state->double_quote || state->shielding) add_symbol(token, '\'');
            else if (state->single_quote) state->single_quote = false;
            else state->single_quote = true;
            break;
        
        case '"':
            if (state->single_quote || state->shielding) add_symbol(token, '"');
            else if (state->double_quote) state->double_quote = false;
            else state->double_quote = true;
            break;

        case '\\':
            if (state->single_quote || state->shielding) add_symbol(token, '\\');
            else state->shielding = true;
            break;

        case '|':
            add_token(cmd, token);
            if (i < strlen(command) - 1 && command[i + 1] == '|') {
                i++;
                state->cmdor = true;
                // TODO: is it OK?
                add_cmd(conveyor, cmd, "|");
            } else {
                state->conveyor = true;
                add_cmd(conveyor, cmd, "||");
            }
            cmd = new_cmd();
            // TODO: add token to command and add command to parse_state, create new command
            break;

        case '&':
            if (i < strlen(command) - 1 && command[i + 1] == '&') {
                i++;
                state->cmdand = true;
            }
            // TODO: add token to command and add command to parse_state, create new command
            break;

        case ' ':
            // TODO: add token to command
            break;

        default:
            break;
        }
    }
}


void delete_cmd(struct cmd* cmd) {
    // TODO: why?
    // free(cmd->name);
    // for (int i = 0; i < cmd->argc; i++) free((char *) cmd->argv[i]);
    free(cmd->argv);
    free(cmd);
}
