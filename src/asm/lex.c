#include "lex.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "token.h"

static struct keyword {
    const char *str;
    token_type_t value;
} keywords[] = {
    { "ADC", TOKEN_KW_ADC },
    { "ADD", TOKEN_KW_ADD },
    { "AND", TOKEN_KW_AND },
    { "ASL", TOKEN_KW_ASL },
    { "ASR", TOKEN_KW_ASR },
    { "BIC", TOKEN_KW_BIC },
    { "CMN", TOKEN_KW_CMN },
    { "CMP", TOKEN_KW_CMP },
    { "EOR", TOKEN_KW_EOR },
    { "LSL", TOKEN_KW_LSL },
    { "LSR", TOKEN_KW_LSR },
    { "MOV", TOKEN_KW_MOV },
    { "MVN", TOKEN_KW_MVN },
    { "ORR", TOKEN_KW_ORR },
    { "R0", TOKEN_KW_R0 },
    { "R1", TOKEN_KW_R1 },
    { "R10", TOKEN_KW_R10 },
    { "R11", TOKEN_KW_R11 },
    { "R12", TOKEN_KW_R12 },
    { "R13", TOKEN_KW_R13 },
    { "R14", TOKEN_KW_R14 },
    { "R15", TOKEN_KW_R15 },
    { "R2", TOKEN_KW_R2 },
    { "R3", TOKEN_KW_R3 },
    { "R4", TOKEN_KW_R4 },
    { "R5", TOKEN_KW_R5 },
    { "R6", TOKEN_KW_R6 },
    { "R7", TOKEN_KW_R7 },
    { "R8", TOKEN_KW_R8 },
    { "R9", TOKEN_KW_R9 },
    { "ROR", TOKEN_KW_ROR },
    { "RSB", TOKEN_KW_RSB },
    { "RSC", TOKEN_KW_RSC },
    { "SBC", TOKEN_KW_SBC },
    { "SUB", TOKEN_KW_SUB },
    { "TEQ", TOKEN_KW_TEQ },
    { "TST", TOKEN_KW_TST },
};

#define NUM_KEYWORDS (sizeof(keywords) / sizeof(keywords[0]))

static int lex_readc(lex_t *l)
{
    int c = fgetc(l->input);
    if (c == '\n') {
        l->line++;
        l->col = 1;
    } else {
        l->col++;
    }
    return c;
}

static int lex_ungetc(lex_t *l, int c)
{
    l->col--;
    return ungetc(c, l->input);
}

static void lex_skip_space(lex_t *l)
{
    int c;
    while ((c = lex_readc(l)) != EOF) {
        if (isblank(c)) {
            continue;
        }
        lex_ungetc(l, c);
        return;
    }
}

static int keyword_compare(const void *k1, const void *k2)
{
    struct keyword *kw1 = (struct keyword *)k1;
    struct keyword *kw2 = (struct keyword *)k2;
    return strcmp(kw1->str, kw2->str);
}

static token_type_t get_token_type(char *s)
{
    struct keyword kw, *res;
    kw.str = s;
    res = bsearch(&kw, keywords, NUM_KEYWORDS, sizeof(kw), keyword_compare);
    if (res) {
        return res->value;
    }
    return TOKEN_IDENT;
}

static token_t *read_keyword(lex_t *l, int c)
{
    str_t *str = str_create();
    str_append(str, (char)c);
    for (;;) {
        token_type_t tok_type;
        char *tok_str;
        c = lex_readc(l);
        if (isalnum(c)) {
            str_append(str, (char)c);
            continue;
        }
        lex_ungetc(l, c);
        str_append(str, '\0');
        tok_str = str_destroy(str);
        tok_type = get_token_type(tok_str);
        free(tok_str);
        if (tok_type == TOKEN_IDENT) {
            return token_create(TOKEN_KW_INVALID);
        }
        return token_create(tok_type);
    }
}

static token_t *read_number(lex_t *l, int c)
{
    str_t *str = str_create();
    str_append(str, (char)c);
    for (;;) {
        c = lex_readc(l);
        if (isdigit(c)) {
            str_append(str, (char)c);
            continue;
        }
        lex_ungetc(l, c);
        str_append(str, '\0');
        return token_create_string(TOKEN_CONSTANT, str_destroy(str));
    }
}

token_t *lex_next_token(lex_t *l)
{
    int c;
    lex_skip_space(l);
    l->last_tok_line = l->line;
    l->last_tok_col = l->col;
    c = lex_readc(l);
    switch (c) {
        case ',':
        case '#':
            return token_create(c);
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return read_number(l, c);
        case '\0':
        case '\n':
        case '\r':
            l->checkpoint = ftell(l->input);
            return token_create(TOKEN_END);
        case EOF:
            return NULL;
        default:
            return read_keyword(l, c);
    }
}

int lex_init(lex_t *l, FILE *input)
{
    l->input = input;
    l->line = 1;
    l->col = 1;
    l->checkpoint = 0;
    return 0;
}

void lex_error(lex_t *l, const char *msg)
{
    char str[100];
    fseek(l->input, l->checkpoint, SEEK_SET);
    if (fgets(str, sizeof(str), l->input) == NULL) {
        fprintf(stderr, "%s:%d: error: fgets\n", __func__, __LINE__);
        return;
    }
    fprintf(stderr, "%d:%d: error: %s\n%s\n", l->last_tok_line, l->last_tok_col,
            msg, str);
    for (int i = 1; i < l->last_tok_col; ++i) {
        fputc(' ', stderr);
    }
    fprintf(stderr, "^\n");
}
