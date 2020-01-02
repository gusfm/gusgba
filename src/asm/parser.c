#include "parser.h"

#include <stdlib.h>
#include "arm_enc.h"

#define CHK(f)               \
    do {                     \
        int rv = f;          \
        if (rv != PARSER_OK) \
            return rv;       \
    } while (0)

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

static int parse_number(parser_t *p, long int *number)
{
    int ret = PARSER_ERR_SYNTAX;
    token_t *tok = parser_next_token(p);
    if (tok->type == TOKEN_CONSTANT) {
        int base = tok->s[1] == 'x' ? 16 : 10;
        *number = strtol(tok->s, NULL, base);
        ret = PARSER_OK;
    }
    token_destroy(tok);
    return ret;
}

static int parse_reg(token_type_t tok_type, reg_t *reg)
{
    switch (tok_type) {
        case TOKEN_KW_R0:
            *reg = REG_R0;
            return PARSER_OK;
        case TOKEN_KW_R1:
            *reg = REG_R1;
            return PARSER_OK;
        case TOKEN_KW_R2:
            *reg = REG_R2;
            return PARSER_OK;
        case TOKEN_KW_R3:
            *reg = REG_R3;
            return PARSER_OK;
        case TOKEN_KW_R4:
            *reg = REG_R4;
            return PARSER_OK;
        case TOKEN_KW_R5:
            *reg = REG_R5;
            return PARSER_OK;
        case TOKEN_KW_R6:
            *reg = REG_R6;
            return PARSER_OK;
        case TOKEN_KW_R7:
            *reg = REG_R7;
            return PARSER_OK;
        case TOKEN_KW_R8:
            *reg = REG_R8;
            return PARSER_OK;
        case TOKEN_KW_R9:
            *reg = REG_R9;
            return PARSER_OK;
        case TOKEN_KW_SL:
        case TOKEN_KW_R10:
            *reg = REG_R10;
            return PARSER_OK;
        case TOKEN_KW_FP:
        case TOKEN_KW_R11:
            *reg = REG_R11;
            return PARSER_OK;
        case TOKEN_KW_IP:
        case TOKEN_KW_R12:
            *reg = REG_R12;
            return PARSER_OK;
        case TOKEN_KW_SP:
        case TOKEN_KW_R13:
            *reg = REG_R13;
            return PARSER_OK;
        case TOKEN_KW_LR:
        case TOKEN_KW_R14:
            *reg = REG_R14;
            return PARSER_OK;
        case TOKEN_KW_PC:
        case TOKEN_KW_R15:
            *reg = REG_R15;
            return PARSER_OK;
        default:
            return PARSER_ERR_SYNTAX;
    }
}

static int parse_shift_type(token_type_t tok_type, shift_type_t *type)
{
    switch (tok_type) {
        case TOKEN_KW_ASL:
        case TOKEN_KW_LSL:
            *type = SHIFT_TYPE_LSL;
            return PARSER_OK;
        case TOKEN_KW_LSR:
            *type = SHIFT_TYPE_LSR;
            return PARSER_OK;
        case TOKEN_KW_ASR:
            *type = SHIFT_TYPE_ASR;
            return PARSER_OK;
        case TOKEN_KW_ROR:
            *type = SHIFT_TYPE_ROR;
            return PARSER_OK;
        case TOKEN_KW_RRX:
            *type = SHIFT_TYPE_RRX;
            return PARSER_OK;
        default:
            return PARSER_ERR_SYNTAX;
    }
}

static int out_u32(FILE *f, uint32_t op)
{
    size_t ret = fwrite(&op, 4, 1, f);
    if (ret != 1) {
        fprintf(stderr, "failed to write opcode 0x%x\n", op);
    }
    return PARSER_OK;
}

static int parse_imm_val(parser_t *p, imm_val_t *imm_val)
{
    int rotate = 0;
    long int number;
    CHK(parse_number(p, &number));
    while (number & 0xffffff00) {
        number = (number & 0xc0000000) >> 30 | (number << 2);
        ++rotate;
        if (rotate > 0xf)
            return PARSER_ERR_INVALID_CONSTANT;
    }
    imm_val->imm = (uint8_t)number;
    imm_val->rotate = (uint8_t)rotate;
    return PARSER_OK;
}

static int parse_shift(parser_t *p, token_type_t tok_type, shift_t *shift)
{
    CHK(parse_reg(tok_type, &shift->rm));
    if (parser_next_token_type(p) == ',') {
        CHK(parse_shift_type(parser_next_token_type(p), &shift->type));
        if (shift->type == SHIFT_TYPE_RRX) {
            shift->type = SHIFT_TYPE_ROR;
            shift->is_register = 0;
            shift->amount = 0;
            return PARSER_OK;
        }
        token_type_t type = parser_next_token_type(p);
        if (type == '#') {
            long int number;
            CHK(parse_number(p, &number));
            if (number < 0 || number > 32 ||
                (number == 32 && shift->type != SHIFT_TYPE_LSR &&
                 shift->type != SHIFT_TYPE_ASR)) {
                return PARSER_ERR_SHIFT;
            }
            if (number == 0)
                shift->type = SHIFT_TYPE_LSL;
            shift->is_register = 0;
            shift->amount = (uint8_t)number;
        } else {
            shift->is_register = 1;
            CHK(parse_reg(type, &shift->rs));
        }
    } else {
        shift->is_register = 0;
        shift->type = 0;
        shift->amount = 0;
    }
    return PARSER_OK;
}

static int parse_oper2(parser_t *p, oper2_t *oper2)
{
    token_type_t tok_type = parser_next_token_type(p);
    if (tok_type == '#') {
        oper2->is_imm_val = true;
        return parse_imm_val(p, &oper2->imm_val);
    } else {
        oper2->is_imm_val = false;
        return parse_shift(p, tok_type, &oper2->shift);
    }
    return PARSER_OK;
}

static int parse_cmd_dp(parser_t *p, uint32_t opcode, bool s)
{
    reg_t rd, rn;
    oper2_t oper2;

    CHK(parse_reg(parser_next_token_type(p), &rd));
    if (parser_next_token_type(p) != ',') {
        return PARSER_ERR_SYNTAX;
    }

    CHK(parse_reg(parser_next_token_type(p), &rn));
    if (parser_next_token_type(p) != ',') {
        return PARSER_ERR_SYNTAX;
    }

    CHK(parse_oper2(p, &oper2));
    if (parser_next_token_type(p) != TOKEN_END) {
        return PARSER_ERR_SYNTAX;
    }

    return out_u32(p->out, arm_enc_and_imm(COND_AL, opcode, s, rd, rn, &oper2));
}

static int parse_cmd(parser_t *p, token_t *tok)
{
    token_type_t tok_type = tok->type;
    token_destroy(tok);
    switch (tok_type) {
        case TOKEN_KW_AND:
            return parse_cmd_dp(p, 0x0, false);
        case TOKEN_KW_ANDS:
            return parse_cmd_dp(p, 0x0, true);
        case TOKEN_KW_EOR:
            return parse_cmd_dp(p, 0x1, false);
        case TOKEN_KW_EORS:
            return parse_cmd_dp(p, 0x1, true);
        case TOKEN_KW_SUB:
            return parse_cmd_dp(p, 0x2, false);
        case TOKEN_KW_SUBS:
            return parse_cmd_dp(p, 0x2, true);
        case TOKEN_KW_RSB:
            return parse_cmd_dp(p, 0x3, false);
        case TOKEN_KW_RSBS:
            return parse_cmd_dp(p, 0x3, true);
        case TOKEN_KW_ADD:
            return parse_cmd_dp(p, 0x4, false);
        case TOKEN_KW_ADDS:
            return parse_cmd_dp(p, 0x4, true);
        case TOKEN_KW_ADC:
            return parse_cmd_dp(p, 0x5, false);
        case TOKEN_KW_ADCS:
            return parse_cmd_dp(p, 0x5, true);
        case TOKEN_KW_SBC:
            return parse_cmd_dp(p, 0x6, false);
        case TOKEN_KW_SBCS:
            return parse_cmd_dp(p, 0x6, true);
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
        case PARSER_ERR_SHIFT:
            return "shift expression is too large";
        case PARSER_ERR_INVALID_CONSTANT:
            return "invalid constant after fixup";
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
