#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_IDENT = 0x100,
    TOKEN_CHAR,
    TOKEN_CONSTANT,
    TOKEN_END,
    TOKEN_KW_R0,
    TOKEN_KW_R1,
    TOKEN_KW_R2,
    TOKEN_KW_R3,
    TOKEN_KW_R4,
    TOKEN_KW_R5,
    TOKEN_KW_R6,
    TOKEN_KW_R7,
    TOKEN_KW_R8,
    TOKEN_KW_R9,
    TOKEN_KW_R10,
    TOKEN_KW_R11,
    TOKEN_KW_R12,
    TOKEN_KW_R13,
    TOKEN_KW_R14,
    TOKEN_KW_R15,
    TOKEN_KW_SL,
    TOKEN_KW_FP,
    TOKEN_KW_IP,
    TOKEN_KW_SP,
    TOKEN_KW_LR,
    TOKEN_KW_PC,
    TOKEN_KW_ADC,
    TOKEN_KW_ADD,
    TOKEN_KW_AND,
    TOKEN_KW_ANDS,
    TOKEN_KW_BIC,
    TOKEN_KW_CMN,
    TOKEN_KW_CMP,
    TOKEN_KW_EOR,
    TOKEN_KW_EORS,
    TOKEN_KW_MOV,
    TOKEN_KW_MVN,
    TOKEN_KW_ORR,
    TOKEN_KW_RSB,
    TOKEN_KW_RSBS,
    TOKEN_KW_RSC,
    TOKEN_KW_SBC,
    TOKEN_KW_SUB,
    TOKEN_KW_SUBS,
    TOKEN_KW_TEQ,
    TOKEN_KW_TST,
    TOKEN_KW_ASL,
    TOKEN_KW_LSL,
    TOKEN_KW_LSR,
    TOKEN_KW_ASR,
    TOKEN_KW_ROR,
    TOKEN_KW_RRX,
    TOKEN_KW_INVALID
} token_type_t;

typedef struct {
    token_type_t type;
    char *s;
} token_t;

token_t *token_create(token_type_t type);
token_t *token_create_string(token_type_t type, char *s);
void token_destroy(token_t *t);

#endif /* TOKEN_H */
