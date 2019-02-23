#include <stdio.h>
#include <string.h>
#include "asm/lex.h"
#include "test.h"
#include "ut.h"

static int check_next_token(lex_t *l, token_type_t type)
{
    token_t *t = lex_next_token(l);
    ASSERT(t != NULL);
    ASSERT(type == t->type);
    token_destroy(t);
    return 0;
}

static int check_next_token_str(lex_t *l, token_type_t type, const char *s)
{
    token_t *t = lex_next_token(l);
    ASSERT(type == t->type);
    ASSERT(strcmp(s, t->s) == 0);
    token_destroy(t);
    return 0;
}

static int basic(void)
{
    lex_t lex;
    char src[] = "AND R0, R1, R2, LSL #0";
    FILE *stream = fmemopen(src, sizeof(src) - 1, "r");
    ASSERT(stream != NULL);
    ASSERT(lex_init(&lex, stream) == 0);
    ASSERT(check_next_token(&lex, TOKEN_KW_AND) == 0);
    ASSERT(check_next_token(&lex, TOKEN_KW_R0) == 0);
    ASSERT(check_next_token(&lex, ',') == 0);
    ASSERT(check_next_token(&lex, TOKEN_KW_R1) == 0);
    ASSERT(check_next_token(&lex, ',') == 0);
    ASSERT(check_next_token(&lex, TOKEN_KW_R2) == 0);
    ASSERT(check_next_token(&lex, ',') == 0);
    ASSERT(check_next_token(&lex, TOKEN_KW_LSL) == 0);
    ASSERT(check_next_token(&lex, '#') == 0);
    ASSERT(check_next_token_str(&lex, TOKEN_CONSTANT, "0") == 0);
    fclose(stream);
    return 0;
}

void lex_test(void)
{
    ut_run(basic);
}
