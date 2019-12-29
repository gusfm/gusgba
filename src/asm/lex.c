#include "lex.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "token.h"

#define NUM_KEYWORDS (sizeof(keywords) / sizeof(keywords[0]))

/* clang-format off */

static struct keyword {
    const char *str;
    token_type_t value;
} keywords[] = {
    { "adc", TOKEN_KW_ADC },
    { "add", TOKEN_KW_ADD },
    { "and", TOKEN_KW_AND },
    { "ands", TOKEN_KW_ANDS },
    { "asl", TOKEN_KW_ASL },
    { "asr", TOKEN_KW_ASR },
    { "bic", TOKEN_KW_BIC },
    { "cmn", TOKEN_KW_CMN },
    { "cmp", TOKEN_KW_CMP },
    { "eor", TOKEN_KW_EOR },
    { "fp", TOKEN_KW_FP },
    { "ip", TOKEN_KW_IP },
    { "lr", TOKEN_KW_LR },
    { "lsl", TOKEN_KW_LSL },
    { "lsr", TOKEN_KW_LSR },
    { "mov", TOKEN_KW_MOV },
    { "mvn", TOKEN_KW_MVN },
    { "orr", TOKEN_KW_ORR },
    { "pc", TOKEN_KW_PC },
    { "r0", TOKEN_KW_R0 },
    { "r1", TOKEN_KW_R1 },
    { "r10", TOKEN_KW_R10 },
    { "r11", TOKEN_KW_R11 },
    { "r12", TOKEN_KW_R12 },
    { "r13", TOKEN_KW_R13 },
    { "r14", TOKEN_KW_R14 },
    { "r15", TOKEN_KW_R15 },
    { "r2", TOKEN_KW_R2 },
    { "r3", TOKEN_KW_R3 },
    { "r4", TOKEN_KW_R4 },
    { "r5", TOKEN_KW_R5 },
    { "r6", TOKEN_KW_R6 },
    { "r7", TOKEN_KW_R7 },
    { "r8", TOKEN_KW_R8 },
    { "r9", TOKEN_KW_R9 },
    { "ror", TOKEN_KW_ROR },
    { "rrx", TOKEN_KW_RRX },
    { "rsb", TOKEN_KW_RSB },
    { "rsc", TOKEN_KW_RSC },
    { "sbc", TOKEN_KW_SBC },
    { "sl", TOKEN_KW_SL },
    { "sp", TOKEN_KW_SP },
    { "sub", TOKEN_KW_SUB },
    { "teq", TOKEN_KW_TEQ },
    { "tst", TOKEN_KW_TST },
};

/* clang-format on */

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
        if (isxdigit(c) || c == 'x') {
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

void lex_checkpoint(lex_t *l)
{
    l->checkpoint = ftell(l->input);
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
