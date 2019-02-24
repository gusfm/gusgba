#include "parser.h"
#include "arm7_enc.h"

typedef enum {
    REG_R0,
    REG_R1,
    REG_R2,
    REG_R3,
    REG_R4,
    REG_R5,
    REG_R6,
    REG_R7,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
    REG_INVALID
} reg_t;

typedef enum {
    SHIFT_TYPE_LSL,
    SHIFT_TYPE_LSR,
    SHIFT_TYPE_ASR,
    SHIFT_TYPE_ROR,
    SHIFT_TYPE_INVALID
} shift_type_t;

typedef enum {
    COND_EQ,
    COND_NE,
    COND_CS,
    COND_CC,
    COND_MI,
    COND_PL,
    COND_VS,
    COND_VC,
    COND_HI,
    COND_LS,
    COND_GE,
    COND_LT,
    COND_GT,
    COND_LE,
    COND_AL,
    COND_INVALID,
} cond_t;

int parser_init(parser_t *p, FILE *input, FILE *output)
{
    lex_init(&p->l, input);
    p->out = output;
    return 0;
}

static token_t *parser_next_token(parser_t *p)
{
    return lex_next_token(&p->l);
}

static token_type_t parser_next_token_type(parser_t *p)
{
    token_type_t type;
    token_t *tok = parser_next_token(p);
    if (tok) {
        type = tok->type;
    } else {
        type = TOKEN_END;
    }
    token_destroy(tok);
    return type;
}

static reg_t parse_reg(token_type_t tok_type)
{
    switch (tok_type) {
        case TOKEN_KW_R0:
            return REG_R0;
        case TOKEN_KW_R1:
            return REG_R1;
        case TOKEN_KW_R2:
            return REG_R2;
        case TOKEN_KW_R3:
            return REG_R3;
        case TOKEN_KW_R4:
            return REG_R4;
        case TOKEN_KW_R5:
            return REG_R5;
        case TOKEN_KW_R6:
            return REG_R6;
        case TOKEN_KW_R7:
            return REG_R7;
        case TOKEN_KW_R8:
            return REG_R8;
        case TOKEN_KW_R9:
            return REG_R9;
        case TOKEN_KW_R10:
            return REG_R10;
        case TOKEN_KW_R11:
            return REG_R11;
        case TOKEN_KW_R12:
            return REG_R12;
        case TOKEN_KW_R13:
            return REG_R13;
        case TOKEN_KW_R14:
            return REG_R14;
        case TOKEN_KW_R15:
            return REG_R15;
        default:
            return REG_INVALID;
    }
}

static shift_type_t parse_shift(token_type_t tok_type)
{
    switch (tok_type) {
        case TOKEN_KW_ASL:
        case TOKEN_KW_LSL:
            return SHIFT_TYPE_LSL;
        case TOKEN_KW_LSR:
            return SHIFT_TYPE_LSR;
        case TOKEN_KW_ASR:
            return SHIFT_TYPE_ASR;
        case TOKEN_KW_ROR:
            return SHIFT_TYPE_ROR;
        default:
            return SHIFT_TYPE_INVALID;
    }
}

static int parse_comma(token_type_t tok_type)
{
    return (char)tok_type == ',';
}

static int out_u32(FILE *f, uint32_t op)
{
    size_t ret = fwrite(&op, 4, 1, f);
    if (ret != 1) {
        fprintf(stderr, "failed to write opcode 0x%x\n", op);
    }
    return PARSER_OK;
}

static int parse_cmd_and(parser_t *p)
{
    reg_t rd, rn, rm;
    shift_type_t shift_type;

    rd = parse_reg(parser_next_token_type(p));
    if (rd == REG_INVALID) {
        return PARSER_ERR_SYNTAX;
    }
    if (!parse_comma(parser_next_token_type(p))) {
        return PARSER_ERR_SYNTAX;
    }

    rn = parse_reg(parser_next_token_type(p));
    if (rn == REG_INVALID) {
        return PARSER_ERR_SYNTAX;
    }
    if (!parse_comma(parser_next_token_type(p))) {
        return PARSER_ERR_SYNTAX;
    }

    rm = parse_reg(parser_next_token_type(p));
    if (rm == REG_INVALID) {
        return PARSER_ERR_SYNTAX;
    }
    if (!parse_comma(parser_next_token_type(p))) {
        return PARSER_ERR_SYNTAX;
    }

    shift_type = parse_shift(parser_next_token_type(p));
    if (shift_type == SHIFT_TYPE_INVALID) {
        return PARSER_ERR_SYNTAX;
    }
    if (parser_next_token_type(p) != '#') {
        return PARSER_ERR_SYNTAX;
    }
    if (parser_next_token_type(p) != TOKEN_CONSTANT) {
        return PARSER_ERR_SYNTAX;
    }
    if (parser_next_token_type(p) != TOKEN_END) {
        return PARSER_ERR_SYNTAX;
    }
    return out_u32(p->out,
                   arm7_enc_and_imm(COND_AL, rd, rn, rm, shift_type, 0));
}

static int parse_cmd(parser_t *p, token_t *tok)
{
    token_type_t tok_type = tok->type;
    token_destroy(tok);
    switch (tok_type) {
        case TOKEN_KW_AND:
            return parse_cmd_and(p);
        default:
            return PARSER_ERR_CMD;
    }
}

int parser_exec(parser_t *p)
{
    token_t *tok;
    while ((tok = parser_next_token(p)) != NULL) {
        int ret;
        if ((ret = parse_cmd(p, tok)) != 0) {
            return ret;
        }
        lex_checkpoint(&p->l);
    }
    return PARSER_OK;
}

static const char *parser_strerror(int error)
{
    switch (error) {
        case PARSER_OK:
            return "";
        case PARSER_ERR_CMD:
            return "invalid command";
        case PARSER_ERR_SYNTAX:
            return "invalid syntax";
        default:
            return "unknown error";
    }
}

void parser_print_error(parser_t *p, int error)
{
    if (error != PARSER_OK) {
        lex_error(&p->l, parser_strerror(error));
    }
}
