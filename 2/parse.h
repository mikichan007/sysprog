#ifndef PARSE
#define PARSE

#include <stdbool.h>

enum operator {PIPE, CMDAND, CMDOR, BGND};

struct cmd {
    const char *name;
    const char **argv;
    int argc;
};


struct conveyor {
    struct cmd **commands;
    const enum operator **op;
    int cn;
};

struct parse_state {
    struct conveyor* conveyor;
    bool error;
    bool shielding;
    bool single_quote;
    bool double_quote;
    bool cmdand;
    bool cmdor;
    bool pipe;
    int cn;
};

void parse_cmd_v2(struct parse_state* state, char *command);

void delete_cmd(struct cmd* cmd);

#endif