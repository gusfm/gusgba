#ifndef PARSER_H
#define PARSER_H

#include "lex.h"

enum {
    PARSER_OK,
    PARSER_ERR_CMD,
    PARSER_ERR_SYNTAX,
};

typedef struct {
    lex_t l;
    FILE *out;
} parser_t;

int parser_init(parser_t *p, FILE *input, FILE *output);
int parser_exec(parser_t *p);
void parser_print_error(parser_t *p, int error);

#endif /* PARSER_H */
