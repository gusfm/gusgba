#ifndef ARM_ENC_H
#define ARM_ENC_H

#include <stdbool.h>
#include <stdint.h>

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
} reg_t;

typedef enum {
    SHIFT_TYPE_LSL,
    SHIFT_TYPE_LSR,
    SHIFT_TYPE_ASR,
    SHIFT_TYPE_ROR,
    SHIFT_TYPE_RRX,
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

typedef struct {
    reg_t rm;
    shift_type_t type;
    bool is_register;
    union {
        uint8_t amount;
        reg_t rs;
    };
} shift_t;

typedef struct {
    uint8_t imm;
    uint8_t rotate;
} imm_val_t;

typedef struct {
    bool is_imm_val;
    union {
        shift_t shift;
        imm_val_t imm_val;
    };
} oper2_t;

uint32_t arm_enc_and_imm(cond_t cond, bool s, reg_t rd, reg_t rn, oper2_t *o);

#endif /* ARM_ENC_H */
