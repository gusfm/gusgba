#include "arm_isa.h"
#include "arm.h"

/* Get register from opcode offset. */
#define OPCODE_REG(offset) ((opcode >> offset) & 0xfu)

/* Logical data processing operations. */
#define DP_OPER_AND(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] & func(opcode))
#define DP_OPER_EOR(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] ^ func(opcode))
#define DP_OPER_TST(func) (arm.r[OPCODE_REG(16)] & func(opcode))
#define DP_OPER_TEQ(func) (arm.r[OPCODE_REG(16)] ^ func(opcode))
#define DP_OPER_ORR(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] | func(opcode))
#define DP_OPER_MOV(func) (arm.r[OPCODE_REG(12)] = func(opcode))
#define DP_OPER_BIC(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] & ~func(opcode))
#define DP_OPER_MVN(func) (arm.r[OPCODE_REG(12)] = ~func(opcode))

/* Set condition codes for logical data processing operation. */
#define DP_CCL(val)                      \
    do {                                 \
        arm.shift_carry = 0;             \
        arm_psr_logical(&arm.cpsr, val); \
    } while (0)

/* Arithmetic data processing operations. */
#define DP_OPER_SUB(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] - func(opcode))
#define DP_OPER_SUBS(func)                              \
    do {                                                \
        uint32_t op1 = arm.r[OPCODE_REG(16)];           \
        uint32_t op2 = func(opcode);                    \
        uint64_t result = (uint64_t)op1 - op2;          \
        arm.r[OPCODE_REG(12)] = (uint32_t)result;       \
        arm_psr_sub_arith(&arm.cpsr, op1, op2, result); \
    } while (0)
#define DP_OPER_RSB(func) \
    (arm.r[OPCODE_REG(12)] = func(opcode) - arm.r[OPCODE_REG(16)])
#define DP_OPER_RSBS(func)                              \
    do {                                                \
        uint32_t op1 = arm.r[OPCODE_REG(16)];           \
        uint32_t op2 = func(opcode);                    \
        uint64_t result = (uint64_t)op2 - op1;          \
        arm.r[OPCODE_REG(12)] = (uint32_t)result;       \
        arm_psr_sub_arith(&arm.cpsr, op2, op1, result); \
    } while (0)
#define DP_OPER_ADD(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] + func(opcode))
#define DP_OPER_ADDS(func)                              \
    do {                                                \
        uint32_t op1 = arm.r[OPCODE_REG(16)];           \
        uint32_t op2 = func(opcode);                    \
        uint64_t result = (uint64_t)op1 + op2;          \
        arm.r[OPCODE_REG(12)] = (uint32_t)result;       \
        arm_psr_add_arith(&arm.cpsr, op1, op2, result); \
    } while (0)
#define DP_OPER_ADC(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] + func(opcode) + arm.cpsr.c)
#define DP_OPER_ADCS(func)                                  \
    do {                                                    \
        uint32_t op1 = arm.r[OPCODE_REG(16)];               \
        uint32_t op2 = func(opcode);                        \
        uint64_t result = (uint64_t)op1 + op2 + arm.cpsr.c; \
        arm.r[OPCODE_REG(12)] = (uint32_t)result;           \
        arm_psr_add_arith(&arm.cpsr, op1, op2, result);     \
    } while (0)
#define DP_OPER_SBC(func)    \
    (arm.r[OPCODE_REG(12)] = \
         arm.r[OPCODE_REG(16)] - func(opcode) + arm.cpsr.c - 1)
#define DP_OPER_SBCS(func)                                      \
    do {                                                        \
        uint32_t op1 = arm.r[OPCODE_REG(16)];                   \
        uint32_t op2 = func(opcode);                            \
        uint64_t result = (uint64_t)op1 - op2 + arm.cpsr.c - 1; \
        arm.r[OPCODE_REG(12)] = (uint32_t)result;               \
        arm_psr_sub_arith(&arm.cpsr, op1, op2, result);         \
    } while (0)
#define DP_OPER_RSC(func)    \
    (arm.r[OPCODE_REG(12)] = \
         func(opcode) - arm.r[OPCODE_REG(16)] + arm.cpsr.c - 1)
#define DP_OPER_RSCS(func)                                      \
    do {                                                        \
        uint32_t op1 = arm.r[OPCODE_REG(16)];                   \
        uint32_t op2 = func(opcode);                            \
        uint64_t result = (uint64_t)op2 - op1 + arm.cpsr.c - 1; \
        arm.r[OPCODE_REG(12)] = (uint32_t)result;               \
        arm_psr_sub_arith(&arm.cpsr, op2, op1, result);         \
    } while (0)
#define DP_OPER_CMP(func)                               \
    do {                                                \
        uint32_t op1 = arm.r[OPCODE_REG(16)];           \
        uint32_t op2 = func(opcode);                    \
        uint64_t result = (uint64_t)op1 - op2;          \
        arm_psr_sub_arith(&arm.cpsr, op1, op2, result); \
    } while (0)
#define DP_OPER_CMN(func)                               \
    do {                                                \
        uint32_t op1 = arm.r[OPCODE_REG(16)];           \
        uint32_t op2 = func(opcode);                    \
        uint64_t result = (uint64_t)op1 + op2;          \
        arm_psr_add_arith(&arm.cpsr, op1, op2, result); \
    } while (0)

/* Data processing function declaration. */
#define INSTR_DP_REG(op)                                                      \
    op##_lsl_imm, op##_lsl_reg, op##_lsr_imm, op##_lsr_reg, op##_asr_imm,     \
        op##_asr_reg, op##_ror_imm, op##_ror_reg, op##_lsl_imm, op##_lsl_reg, \
        op##_lsr_imm, op##_lsr_reg, op##_asr_imm, op##_asr_reg, op##_ror_imm, \
        op##_ror_reg, op##s_lsl_imm, op##s_lsl_reg, op##s_lsr_imm,            \
        op##s_lsr_reg, op##s_asr_imm, op##s_asr_reg, op##s_ror_imm,           \
        op##s_ror_reg, op##s_lsl_imm, op##s_lsl_reg, op##s_lsr_imm,           \
        op##s_lsr_reg, op##s_asr_imm, op##s_asr_reg, op##s_ror_imm,           \
        op##s_ror_reg

#define INSTR_DP_REG_NO_RD(op)                                                \
    op##_lsl_imm, op##_lsl_reg, op##_lsr_imm, op##_lsr_reg, op##_asr_imm,     \
        op##_asr_reg, op##_ror_imm, op##_ror_reg, op##_lsl_imm, op##_lsl_reg, \
        op##_lsr_imm, op##_lsr_reg, op##_asr_imm, op##_asr_reg, op##_ror_imm, \
        op##_ror_reg, op##_lsl_imm, op##_lsl_reg, op##_lsr_imm, op##_lsr_reg, \
        op##_asr_imm, op##_asr_reg, op##_ror_imm, op##_ror_reg, op##_lsl_imm, \
        op##_lsl_reg, op##_lsr_imm, op##_lsr_reg, op##_asr_imm, op##_asr_reg, \
        op##_ror_imm, op##_ror_reg

static uint32_t asr_mask[32] = {
    0xffffffff, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000,
    0xfc000000, 0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
    0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000,
    0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
    0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8,
    0xfffffffc, 0xfffffffe};

static uint32_t ror_mask[32] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f,
    0x0000003f, 0x0000007f, 0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff, 0x0001ffff,
    0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff,
    0x3fffffff, 0x7fffffff};

static inline void arm_psr_logical(arm_psr_t *psr, uint32_t result)
{
    uint32_t fn = result & ARM_PSR_NEGATIVE;
    uint32_t fz = !result << ARM_PSR_ZERO_SHIFT;
    uint32_t fc = arm.shift_carry << ARM_PSR_CARRY_SHIFT;
    psr->psr = fn | fz | fc | (psr->psr & 0x1fffffff);
}

static inline void arm_psr_sub_arith(arm_psr_t *psr, uint32_t op1, uint32_t op2,
                                     uint64_t result)
{
    uint32_t d32 = (uint32_t)result;
    uint32_t fn = d32 & ARM_PSR_NEGATIVE;
    uint32_t fz = !d32 << ARM_PSR_ZERO_SHIFT;
    uint32_t fc = ((uint32_t)(result >> 32) & 1) << ARM_PSR_CARRY_SHIFT;
    uint32_t fv = (((op1 ^ op2) & (op1 ^ d32)) >> 31) << ARM_PSR_OVERFLOW_SHIFT;
    psr->psr = fn | fz | fc | fv | (psr->psr & 0x0fffffff);
}

static inline void arm_psr_add_arith(arm_psr_t *psr, uint32_t op1, uint32_t op2,
                                     uint64_t result)
{
    uint32_t d32 = (uint32_t)result;
    uint32_t fn = d32 & ARM_PSR_NEGATIVE;
    uint32_t fz = !d32 << ARM_PSR_ZERO_SHIFT;
    uint32_t fc = (uint32_t)(result >> 32) << ARM_PSR_CARRY_SHIFT;
    uint32_t fv = (((~(op1 ^ op2)) & (op1 ^ d32)) >> 31)
                  << ARM_PSR_OVERFLOW_SHIFT;
    psr->psr = fn | fz | fc | fv | (psr->psr & 0x0fffffff);
}

static inline uint32_t dp_lsl(uint32_t opcode, uint32_t shift)
{
    uint32_t val = arm.r[opcode & 0xf];
    shift &= 0x1f;
    if (!shift) {
        arm.shift_carry = arm.cpsr.c;
        return val;
    } else {
        arm.shift_carry = (val >> (32 - shift)) & 1;
        return val << shift;
    }
}

static inline uint32_t dp_lsr(uint32_t opcode, uint32_t shift)
{
    uint32_t val = arm.r[opcode & 0xf];
    shift &= 0x1f;
    if (!shift) {
        arm.shift_carry = val >> 31;
        return 0;
    } else {
        arm.shift_carry = (val >> (shift - 1)) & 1;
        return val >> shift;
    }
}

static inline uint32_t dp_asr(uint32_t opcode, uint32_t shift)
{
    uint32_t val = arm.r[opcode & 0xf];
    shift &= 0x1f;
    uint32_t ret = val >> shift;
    if (val & 0x80000000)
        ret |= asr_mask[shift];
    arm.shift_carry = val >> 31;
    return ret;
}

static inline uint32_t dp_ror(uint32_t opcode, uint32_t shift)
{
    uint32_t val = arm.r[opcode & 0xf];
    shift &= 0x1f;
    uint32_t carry = val & ror_mask[shift];
    arm.shift_carry = (val >> (shift - 1)) & 1;
    return (carry << (32 - shift)) | (val >> shift);
}

/* Data processing logical shift left immediate */
static inline uint32_t dp_lsl_imm(uint32_t opcode)
{
    return dp_lsl(opcode, opcode >> 7);
}

/* Data processing logical shift right immediate */
static inline uint32_t dp_lsr_imm(uint32_t opcode)
{
    return dp_lsr(opcode, opcode >> 7);
}

/* Data processing arithmetic shift right immediate */
static inline uint32_t dp_asr_imm(uint32_t opcode)
{
    return dp_asr(opcode, opcode >> 7);
}

/* Data processing rotate right immediate */
static inline uint32_t dp_ror_imm(uint32_t opcode)
{
    return dp_ror(opcode, opcode >> 7);
}

/* Data processing logical shift left reg */
static inline uint32_t dp_lsl_reg(uint32_t opcode)
{
    return dp_lsl(opcode, arm.r[(opcode >> 8) & 0xf]);
}

/* Data processing logical shift right reg */
static inline uint32_t dp_lsr_reg(uint32_t opcode)
{
    return dp_lsr(opcode, arm.r[(opcode >> 8) & 0xf]);
}

/* Data processing arithmetic shift right reg */
static inline uint32_t dp_asr_reg(uint32_t opcode)
{
    return dp_asr(opcode, arm.r[(opcode >> 8) & 0xf]);
}

/* Data processing rotate right reg */
static inline uint32_t dp_ror_reg(uint32_t opcode)
{
    return dp_ror(opcode, arm.r[(opcode >> 8) & 0xf]);
}

static inline uint32_t dp_imm(uint32_t opcode)
{
    uint32_t rotate = (opcode & 0xf00) >> 7;
    uint32_t imm = opcode & 0xff;
    return ((imm >> rotate) | (imm << (32 - rotate)));
}

/* clang-format off */

/* AND Rd, Rn, Rm, LSL # */
static void and_lsl_imm(uint32_t opcode) { DP_OPER_AND(dp_lsl_imm); }
/* AND Rd, Rn, Rm, LSR # */
static void and_lsr_imm(uint32_t opcode) { DP_OPER_AND(dp_lsr_imm); }
/* AND Rd, Rn, Rm, ASR # */
static void and_asr_imm(uint32_t opcode) { DP_OPER_AND(dp_asr_imm); }
/* AND Rd, Rn, Rm, ROR # */
static void and_ror_imm(uint32_t opcode) { DP_OPER_AND(dp_ror_imm); }
/* AND Rd, Rn, Rm, LSL Rs */
static void and_lsl_reg(uint32_t opcode) { DP_OPER_AND(dp_lsl_reg); }
/* AND Rd, Rn, Rm, LSR Rs */
static void and_lsr_reg(uint32_t opcode) { DP_OPER_AND(dp_lsr_reg); }
/* AND Rd, Rn, Rm, ASR Rs */
static void and_asr_reg(uint32_t opcode) { DP_OPER_AND(dp_asr_reg); }
/* AND Rd, Rn, Rm, ROR Rs */
static void and_ror_reg(uint32_t opcode) { DP_OPER_AND(dp_ror_reg); }
/* ANDS Rd, Rn, Rm, LSL # */
static void ands_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_lsl_imm)); }
/* ANDS Rd, Rn, Rm, LSR # */
static void ands_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_lsr_imm)); }
/* ANDS Rd, Rn, Rm, ASR # */
static void ands_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_asr_imm)); }
/* ANDS Rd, Rn, Rm, ROR # */
static void ands_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_ror_imm)); }
/* ANDS Rd, Rn, Rm, LSL Rs */
static void ands_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_lsl_reg)); }
/* ANDS Rd, Rn, Rm, LSR Rs */
static void ands_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_lsr_reg)); }
/* ANDS Rd, Rn, Rm, ASR Rs */
static void ands_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_asr_reg)); }
/* ANDS Rd, Rn, Rm, ROR Rs */
static void ands_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_ror_reg)); }
/* EOR Rd, Rn, Rm, LSL # */
static void eor_lsl_imm(uint32_t opcode) { DP_OPER_EOR(dp_lsl_imm); }
/* EOR Rd, Rn, Rm, LSR # */
static void eor_lsr_imm(uint32_t opcode) { DP_OPER_EOR(dp_lsr_imm); }
/* EOR Rd, Rn, Rm, ASR # */
static void eor_asr_imm(uint32_t opcode) { DP_OPER_EOR(dp_asr_imm); }
/* EOR Rd, Rn, Rm, ROR # */
static void eor_ror_imm(uint32_t opcode) { DP_OPER_EOR(dp_ror_imm); }
/* EOR Rd, Rn, Rm, LSL Rs */
static void eor_lsl_reg(uint32_t opcode) { DP_OPER_EOR(dp_lsl_reg); }
/* EOR Rd, Rn, Rm, LSR Rs */
static void eor_lsr_reg(uint32_t opcode) { DP_OPER_EOR(dp_lsr_reg); }
/* EOR Rd, Rn, Rm, ASR Rs */
static void eor_asr_reg(uint32_t opcode) { DP_OPER_EOR(dp_asr_reg); }
/* EOR Rd, Rn, Rm, ROR Rs */
static void eor_ror_reg(uint32_t opcode) { DP_OPER_EOR(dp_ror_reg); }
/* EORS Rd, Rn, Rm, LSL # */
static void eors_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_lsl_imm)); }
/* EORS Rd, Rn, Rm, LSR # */
static void eors_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_lsr_imm)); }
/* EORS Rd, Rn, Rm, ASR # */
static void eors_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_asr_imm)); }
/* EORS Rd, Rn, Rm, ROR # */
static void eors_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_ror_imm)); }
/* EORS Rd, Rn, Rm, LSL Rs */
static void eors_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_lsl_reg)); }
/* EORS Rd, Rn, Rm, LSR Rs */
static void eors_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_lsr_reg)); }
/* EORS Rd, Rn, Rm, ASR Rs */
static void eors_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_asr_reg)); }
/* EORS Rd, Rn, Rm, ROR Rs */
static void eors_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_ror_reg)); }
/* SUB Rd, Rn, Rm, LSL # */
static void sub_lsl_imm(uint32_t opcode) { DP_OPER_SUB(dp_lsl_imm); }
/* SUB Rd, Rn, Rm, LSR # */
static void sub_lsr_imm(uint32_t opcode) { DP_OPER_SUB(dp_lsr_imm); }
/* SUB Rd, Rn, Rm, ASR # */
static void sub_asr_imm(uint32_t opcode) { DP_OPER_SUB(dp_asr_imm); }
/* SUB Rd, Rn, Rm, ROR # */
static void sub_ror_imm(uint32_t opcode) { DP_OPER_SUB(dp_ror_imm); }
/* SUB Rd, Rn, Rm, LSL Rs */
static void sub_lsl_reg(uint32_t opcode) { DP_OPER_SUB(dp_lsl_reg); }
/* SUB Rd, Rn, Rm, LSR Rs */
static void sub_lsr_reg(uint32_t opcode) { DP_OPER_SUB(dp_lsr_reg); }
/* SUB Rd, Rn, Rm, ASR Rs */
static void sub_asr_reg(uint32_t opcode) { DP_OPER_SUB(dp_asr_reg); }
/* SUB Rd, Rn, Rm, ROR Rs */
static void sub_ror_reg(uint32_t opcode) { DP_OPER_SUB(dp_ror_reg); }
/* SUBS Rd, Rn, Rm, LSL # */
static void subs_lsl_imm(uint32_t opcode) { DP_OPER_SUBS(dp_lsl_imm); }
/* SUBS Rd, Rn, Rm, LSR # */
static void subs_lsr_imm(uint32_t opcode) { DP_OPER_SUBS(dp_lsr_imm); }
/* SUBS Rd, Rn, Rm, ASR # */
static void subs_asr_imm(uint32_t opcode) { DP_OPER_SUBS(dp_asr_imm); }
/* SUBS Rd, Rn, Rm, ROR # */
static void subs_ror_imm(uint32_t opcode) { DP_OPER_SUBS(dp_ror_imm); }
/* SUBS Rd, Rn, Rm, LSL Rs */
static void subs_lsl_reg(uint32_t opcode) { DP_OPER_SUBS(dp_lsl_reg); }
/* SUBS Rd, Rn, Rm, LSR Rs */
static void subs_lsr_reg(uint32_t opcode) { DP_OPER_SUBS(dp_lsr_reg); }
/* SUBS Rd, Rn, Rm, ASR Rs */
static void subs_asr_reg(uint32_t opcode) { DP_OPER_SUBS(dp_asr_reg); }
/* SUBS Rd, Rn, Rm, ROR Rs */
static void subs_ror_reg(uint32_t opcode) { DP_OPER_SUBS(dp_ror_reg); }
/* RSB Rd, Rn, Rm, LSL # */
static void rsb_lsl_imm(uint32_t opcode) { DP_OPER_RSB(dp_lsl_imm); }
/* RSB Rd, Rn, Rm, LSR # */
static void rsb_lsr_imm(uint32_t opcode) { DP_OPER_RSB(dp_lsr_imm); }
/* RSB Rd, Rn, Rm, ASR # */
static void rsb_asr_imm(uint32_t opcode) { DP_OPER_RSB(dp_asr_imm); }
/* RSB Rd, Rn, Rm, ROR # */
static void rsb_ror_imm(uint32_t opcode) { DP_OPER_RSB(dp_ror_imm); }
/* RSB Rd, Rn, Rm, LSL Rs */
static void rsb_lsl_reg(uint32_t opcode) { DP_OPER_RSB(dp_lsl_reg); }
/* RSB Rd, Rn, Rm, LSR Rs */
static void rsb_lsr_reg(uint32_t opcode) { DP_OPER_RSB(dp_lsr_reg); }
/* RSB Rd, Rn, Rm, ASR Rs */
static void rsb_asr_reg(uint32_t opcode) { DP_OPER_RSB(dp_asr_reg); }
/* RSB Rd, Rn, Rm, ROR Rs */
static void rsb_ror_reg(uint32_t opcode) { DP_OPER_RSB(dp_ror_reg); }
/* RSBS Rd, Rn, Rm, LSL # */
static void rsbs_lsl_imm(uint32_t opcode) { DP_OPER_RSBS(dp_lsl_imm); }
/* RSBS Rd, Rn, Rm, LSR # */
static void rsbs_lsr_imm(uint32_t opcode) { DP_OPER_RSBS(dp_lsr_imm); }
/* RSBS Rd, Rn, Rm, ASR # */
static void rsbs_asr_imm(uint32_t opcode) { DP_OPER_RSBS(dp_asr_imm); }
/* RSBS Rd, Rn, Rm, ROR # */
static void rsbs_ror_imm(uint32_t opcode) { DP_OPER_RSBS(dp_ror_imm); }
/* RSBS Rd, Rn, Rm, LSL Rs */
static void rsbs_lsl_reg(uint32_t opcode) { DP_OPER_RSBS(dp_lsl_reg); }
/* RSBS Rd, Rn, Rm, LSR Rs */
static void rsbs_lsr_reg(uint32_t opcode) { DP_OPER_RSBS(dp_lsr_reg); }
/* RSBS Rd, Rn, Rm, ASR Rs */
static void rsbs_asr_reg(uint32_t opcode) { DP_OPER_RSBS(dp_asr_reg); }
/* RSBS Rd, Rn, Rm, ROR Rs */
static void rsbs_ror_reg(uint32_t opcode) { DP_OPER_RSBS(dp_ror_reg); }
/* ADD Rd, Rn, Rm, LSL # */
static void add_lsl_imm(uint32_t opcode) { DP_OPER_ADD(dp_lsl_imm); }
/* ADD Rd, Rn, Rm, LSR # */
static void add_lsr_imm(uint32_t opcode) { DP_OPER_ADD(dp_lsr_imm); }
/* ADD Rd, Rn, Rm, ASR # */
static void add_asr_imm(uint32_t opcode) { DP_OPER_ADD(dp_asr_imm); }
/* ADD Rd, Rn, Rm, ROR # */
static void add_ror_imm(uint32_t opcode) { DP_OPER_ADD(dp_ror_imm); }
/* ADD Rd, Rn, Rm, LSL Rs */
static void add_lsl_reg(uint32_t opcode) { DP_OPER_ADD(dp_lsl_reg); }
/* ADD Rd, Rn, Rm, LSR Rs */
static void add_lsr_reg(uint32_t opcode) { DP_OPER_ADD(dp_lsr_reg); }
/* ADD Rd, Rn, Rm, ASR Rs */
static void add_asr_reg(uint32_t opcode) { DP_OPER_ADD(dp_asr_reg); }
/* ADD Rd, Rn, Rm, ROR Rs */
static void add_ror_reg(uint32_t opcode) { DP_OPER_ADD(dp_ror_reg); }
/* ADDS Rd, Rn, Rm, LSL # */
static void adds_lsl_imm(uint32_t opcode) { DP_OPER_ADDS(dp_lsl_imm); }
/* ADDS Rd, Rn, Rm, LSR # */
static void adds_lsr_imm(uint32_t opcode) { DP_OPER_ADDS(dp_lsr_imm); }
/* ADDS Rd, Rn, Rm, ASR # */
static void adds_asr_imm(uint32_t opcode) { DP_OPER_ADDS(dp_asr_imm); }
/* ADDS Rd, Rn, Rm, ROR # */
static void adds_ror_imm(uint32_t opcode) { DP_OPER_ADDS(dp_ror_imm); }
/* ADDS Rd, Rn, Rm, LSL Rs */
static void adds_lsl_reg(uint32_t opcode) { DP_OPER_ADDS(dp_lsl_reg); }
/* ADDS Rd, Rn, Rm, LSR Rs */
static void adds_lsr_reg(uint32_t opcode) { DP_OPER_ADDS(dp_lsr_reg); }
/* ADDS Rd, Rn, Rm, ASR Rs */
static void adds_asr_reg(uint32_t opcode) { DP_OPER_ADDS(dp_asr_reg); }
/* ADDS Rd, Rn, Rm, ROR Rs */
static void adds_ror_reg(uint32_t opcode) { DP_OPER_ADDS(dp_ror_reg); }
/* ADC Rd, Rn, Rm, LSL # */
static void adc_lsl_imm(uint32_t opcode) { DP_OPER_ADC(dp_lsl_imm); }
/* ADC Rd, Rn, Rm, LSR # */
static void adc_lsr_imm(uint32_t opcode) { DP_OPER_ADC(dp_lsr_imm); }
/* ADC Rd, Rn, Rm, ASR # */
static void adc_asr_imm(uint32_t opcode) { DP_OPER_ADC(dp_asr_imm); }
/* ADC Rd, Rn, Rm, ROR # */
static void adc_ror_imm(uint32_t opcode) { DP_OPER_ADC(dp_ror_imm); }
/* ADC Rd, Rn, Rm, LSL Rs */
static void adc_lsl_reg(uint32_t opcode) { DP_OPER_ADC(dp_lsl_reg); }
/* ADC Rd, Rn, Rm, LSR Rs */
static void adc_lsr_reg(uint32_t opcode) { DP_OPER_ADC(dp_lsr_reg); }
/* ADC Rd, Rn, Rm, ASR Rs */
static void adc_asr_reg(uint32_t opcode) { DP_OPER_ADC(dp_asr_reg); }
/* ADC Rd, Rn, Rm, ROR Rs */
static void adc_ror_reg(uint32_t opcode) { DP_OPER_ADC(dp_ror_reg); }
/* ADCS Rd, Rn, Rm, LSL # */
static void adcs_lsl_imm(uint32_t opcode) { DP_OPER_ADCS(dp_lsl_imm); }
/* ADCS Rd, Rn, Rm, LSR # */
static void adcs_lsr_imm(uint32_t opcode) { DP_OPER_ADCS(dp_lsr_imm); }
/* ADCS Rd, Rn, Rm, ASR # */
static void adcs_asr_imm(uint32_t opcode) { DP_OPER_ADCS(dp_asr_imm); }
/* ADCS Rd, Rn, Rm, ROR # */
static void adcs_ror_imm(uint32_t opcode) { DP_OPER_ADCS(dp_ror_imm); }
/* ADCS Rd, Rn, Rm, LSL Rs */
static void adcs_lsl_reg(uint32_t opcode) { DP_OPER_ADCS(dp_lsl_reg); }
/* ADCS Rd, Rn, Rm, LSR Rs */
static void adcs_lsr_reg(uint32_t opcode) { DP_OPER_ADCS(dp_lsr_reg); }
/* ADCS Rd, Rn, Rm, ASR Rs */
static void adcs_asr_reg(uint32_t opcode) { DP_OPER_ADCS(dp_asr_reg); }
/* ADCS Rd, Rn, Rm, ROR Rs */
static void adcs_ror_reg(uint32_t opcode) { DP_OPER_ADCS(dp_ror_reg); }
/* SBC Rd, Rn, Rm, LSL # */
static void sbc_lsl_imm(uint32_t opcode) { DP_OPER_SBC(dp_lsl_imm); }
/* SBC Rd, Rn, Rm, LSR # */
static void sbc_lsr_imm(uint32_t opcode) { DP_OPER_SBC(dp_lsr_imm); }
/* SBC Rd, Rn, Rm, ASR # */
static void sbc_asr_imm(uint32_t opcode) { DP_OPER_SBC(dp_asr_imm); }
/* SBC Rd, Rn, Rm, ROR # */
static void sbc_ror_imm(uint32_t opcode) { DP_OPER_SBC(dp_ror_imm); }
/* SBC Rd, Rn, Rm, LSL Rs */
static void sbc_lsl_reg(uint32_t opcode) { DP_OPER_SBC(dp_lsl_reg); }
/* SBC Rd, Rn, Rm, LSR Rs */
static void sbc_lsr_reg(uint32_t opcode) { DP_OPER_SBC(dp_lsr_reg); }
/* SBC Rd, Rn, Rm, ASR Rs */
static void sbc_asr_reg(uint32_t opcode) { DP_OPER_SBC(dp_asr_reg); }
/* SBC Rd, Rn, Rm, ROR Rs */
static void sbc_ror_reg(uint32_t opcode) { DP_OPER_SBC(dp_ror_reg); }
/* SBCS Rd, Rn, Rm, LSL # */
static void sbcs_lsl_imm(uint32_t opcode) { DP_OPER_SBCS(dp_lsl_imm); }
/* SBCS Rd, Rn, Rm, LSR # */
static void sbcs_lsr_imm(uint32_t opcode) { DP_OPER_SBCS(dp_lsr_imm); }
/* SBCS Rd, Rn, Rm, ASR # */
static void sbcs_asr_imm(uint32_t opcode) { DP_OPER_SBCS(dp_asr_imm); }
/* SBCS Rd, Rn, Rm, ROR # */
static void sbcs_ror_imm(uint32_t opcode) { DP_OPER_SBCS(dp_ror_imm); }
/* SBCS Rd, Rn, Rm, LSL Rs */
static void sbcs_lsl_reg(uint32_t opcode) { DP_OPER_SBCS(dp_lsl_reg); }
/* SBCS Rd, Rn, Rm, LSR Rs */
static void sbcs_lsr_reg(uint32_t opcode) { DP_OPER_SBCS(dp_lsr_reg); }
/* SBCS Rd, Rn, Rm, ASR Rs */
static void sbcs_asr_reg(uint32_t opcode) { DP_OPER_SBCS(dp_asr_reg); }
/* SBCS Rd, Rn, Rm, ROR Rs */
static void sbcs_ror_reg(uint32_t opcode) { DP_OPER_SBCS(dp_ror_reg); }
/* RSC Rd, Rn, Rm, LSL # */
static void rsc_lsl_imm(uint32_t opcode) { DP_OPER_RSC(dp_lsl_imm); }
/* RSC Rd, Rn, Rm, LSR # */
static void rsc_lsr_imm(uint32_t opcode) { DP_OPER_RSC(dp_lsr_imm); }
/* RSC Rd, Rn, Rm, ASR # */
static void rsc_asr_imm(uint32_t opcode) { DP_OPER_RSC(dp_asr_imm); }
/* RSC Rd, Rn, Rm, ROR # */
static void rsc_ror_imm(uint32_t opcode) { DP_OPER_RSC(dp_ror_imm); }
/* RSC Rd, Rn, Rm, LSL Rs */
static void rsc_lsl_reg(uint32_t opcode) { DP_OPER_RSC(dp_lsl_reg); }
/* RSC Rd, Rn, Rm, LSR Rs */
static void rsc_lsr_reg(uint32_t opcode) { DP_OPER_RSC(dp_lsr_reg); }
/* RSC Rd, Rn, Rm, ASR Rs */
static void rsc_asr_reg(uint32_t opcode) { DP_OPER_RSC(dp_asr_reg); }
/* RSC Rd, Rn, Rm, ROR Rs */
static void rsc_ror_reg(uint32_t opcode) { DP_OPER_RSC(dp_ror_reg); }
/* RSCS Rd, Rn, Rm, LSL # */
static void rscs_lsl_imm(uint32_t opcode) { DP_OPER_RSCS(dp_lsl_imm); }
/* RSCS Rd, Rn, Rm, LSR # */
static void rscs_lsr_imm(uint32_t opcode) { DP_OPER_RSCS(dp_lsr_imm); }
/* RSCS Rd, Rn, Rm, ASR # */
static void rscs_asr_imm(uint32_t opcode) { DP_OPER_RSCS(dp_asr_imm); }
/* RSCS Rd, Rn, Rm, ROR # */
static void rscs_ror_imm(uint32_t opcode) { DP_OPER_RSCS(dp_ror_imm); }
/* RSCS Rd, Rn, Rm, LSL Rs */
static void rscs_lsl_reg(uint32_t opcode) { DP_OPER_RSCS(dp_lsl_reg); }
/* RSCS Rd, Rn, Rm, LSR Rs */
static void rscs_lsr_reg(uint32_t opcode) { DP_OPER_RSCS(dp_lsr_reg); }
/* RSCS Rd, Rn, Rm, ASR Rs */
static void rscs_asr_reg(uint32_t opcode) { DP_OPER_RSCS(dp_asr_reg); }
/* RSCS Rd, Rn, Rm, ROR Rs */
static void rscs_ror_reg(uint32_t opcode) { DP_OPER_RSCS(dp_ror_reg); }
/* TST Rn, Rm, LSL # */
static void tst_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_lsl_imm)); }
/* TST Rn, Rm, LSR # */
static void tst_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_lsr_imm)); }
/* TST Rn, Rm, ASR # */
static void tst_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_asr_imm)); }
/* TST Rn, Rm, ROR # */
static void tst_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_ror_imm)); }
/* TST Rn, Rm, LSL Rs */
static void tst_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_lsl_reg)); }
/* TST Rn, Rm, LSR Rs */
static void tst_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_lsr_reg)); }
/* TST Rn, Rm, ASR Rs */
static void tst_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_asr_reg)); }
/* TST Rn, Rm, ROR Rs */
static void tst_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_ror_reg)); }
/* TEQ Rn, Rm, LSL # */
static void teq_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_lsl_imm)); }
/* TEQ Rn, Rm, LSR # */
static void teq_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_lsr_imm)); }
/* TEQ Rn, Rm, ASR # */
static void teq_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_asr_imm)); }
/* TEQ Rn, Rm, ROR # */
static void teq_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_ror_imm)); }
/* TEQ Rn, Rm, LSL Rs */
static void teq_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_lsl_reg)); }
/* TEQ Rn, Rm, LSR Rs */
static void teq_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_lsr_reg)); }
/* TEQ Rn, Rm, ASR Rs */
static void teq_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_asr_reg)); }
/* TEQ Rn, Rm, ROR Rs */
static void teq_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_ror_reg)); }
/* CMP Rn, Rm, LSL # */
static void cmp_lsl_imm(uint32_t opcode) { DP_OPER_CMP(dp_lsl_imm); }
/* CMP Rn, Rm, LSR # */
static void cmp_lsr_imm(uint32_t opcode) { DP_OPER_CMP(dp_lsr_imm); }
/* CMP Rn, Rm, ASR # */
static void cmp_asr_imm(uint32_t opcode) { DP_OPER_CMP(dp_asr_imm); }
/* CMP Rn, Rm, ROR # */
static void cmp_ror_imm(uint32_t opcode) { DP_OPER_CMP(dp_ror_imm); }
/* CMP Rn, Rm, LSL Rs */
static void cmp_lsl_reg(uint32_t opcode) { DP_OPER_CMP(dp_lsl_reg); }
/* CMP Rn, Rm, LSR Rs */
static void cmp_lsr_reg(uint32_t opcode) { DP_OPER_CMP(dp_lsr_reg); }
/* CMP Rn, Rm, ASR Rs */
static void cmp_asr_reg(uint32_t opcode) { DP_OPER_CMP(dp_asr_reg); }
/* CMP Rn, Rm, ROR Rs */
static void cmp_ror_reg(uint32_t opcode) { DP_OPER_CMP(dp_ror_reg); }
/* CMN Rn, Rm, LSL # */
static void cmn_lsl_imm(uint32_t opcode) { DP_OPER_CMN(dp_lsl_imm); }
/* CMN Rn, Rm, LSR # */
static void cmn_lsr_imm(uint32_t opcode) { DP_OPER_CMN(dp_lsr_imm); }
/* CMN Rn, Rm, ASR # */
static void cmn_asr_imm(uint32_t opcode) { DP_OPER_CMN(dp_asr_imm); }
/* CMN Rn, Rm, ROR # */
static void cmn_ror_imm(uint32_t opcode) { DP_OPER_CMN(dp_ror_imm); }
/* CMN Rn, Rm, LSL Rs */
static void cmn_lsl_reg(uint32_t opcode) { DP_OPER_CMN(dp_lsl_reg); }
/* CMN Rn, Rm, LSR Rs */
static void cmn_lsr_reg(uint32_t opcode) { DP_OPER_CMN(dp_lsr_reg); }
/* CMN Rn, Rm, ASR Rs */
static void cmn_asr_reg(uint32_t opcode) { DP_OPER_CMN(dp_asr_reg); }
/* CMN Rn, Rm, ROR Rs */
static void cmn_ror_reg(uint32_t opcode) { DP_OPER_CMN(dp_ror_reg); }
/* ORR Rd, Rn, Rm, LSL # */
static void orr_lsl_imm(uint32_t opcode) { DP_OPER_ORR(dp_lsl_imm); }
/* ORR Rd, Rn, Rm, LSR # */
static void orr_lsr_imm(uint32_t opcode) { DP_OPER_ORR(dp_lsr_imm); }
/* ORR Rd, Rn, Rm, ASR # */
static void orr_asr_imm(uint32_t opcode) { DP_OPER_ORR(dp_asr_imm); }
/* ORR Rd, Rn, Rm, ROR # */
static void orr_ror_imm(uint32_t opcode) { DP_OPER_ORR(dp_ror_imm); }
/* ORR Rd, Rn, Rm, LSL Rs */
static void orr_lsl_reg(uint32_t opcode) { DP_OPER_ORR(dp_lsl_reg); }
/* ORR Rd, Rn, Rm, LSR Rs */
static void orr_lsr_reg(uint32_t opcode) { DP_OPER_ORR(dp_lsr_reg); }
/* ORR Rd, Rn, Rm, ASR Rs */
static void orr_asr_reg(uint32_t opcode) { DP_OPER_ORR(dp_asr_reg); }
/* ORR Rd, Rn, Rm, ROR Rs */
static void orr_ror_reg(uint32_t opcode) { DP_OPER_ORR(dp_ror_reg); }
/* ORRS Rd, Rn, Rm, LSL # */
static void orrs_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_lsl_imm)); }
/* ORRS Rd, Rn, Rm, LSR # */
static void orrs_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_lsr_imm)); }
/* ORRS Rd, Rn, Rm, ASR # */
static void orrs_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_asr_imm)); }
/* ORRS Rd, Rn, Rm, ROR # */
static void orrs_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_ror_imm)); }
/* ORRS Rd, Rn, Rm, LSL Rs */
static void orrs_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_lsl_reg)); }
/* ORRS Rd, Rn, Rm, LSR Rs */
static void orrs_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_lsr_reg)); }
/* ORRS Rd, Rn, Rm, ASR Rs */
static void orrs_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_asr_reg)); }
/* ORRS Rd, Rn, Rm, ROR Rs */
static void orrs_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_ror_reg)); }
/* MOV Rd, Rm, LSL # */
static void mov_lsl_imm(uint32_t opcode) { DP_OPER_MOV(dp_lsl_imm); }
/* MOV Rd, Rm, LSR # */
static void mov_lsr_imm(uint32_t opcode) { DP_OPER_MOV(dp_lsr_imm); }
/* MOV Rd, Rm, ASR # */
static void mov_asr_imm(uint32_t opcode) { DP_OPER_MOV(dp_asr_imm); }
/* MOV Rd, Rm, ROR # */
static void mov_ror_imm(uint32_t opcode) { DP_OPER_MOV(dp_ror_imm); }
/* MOV Rd, Rm, LSL Rs */
static void mov_lsl_reg(uint32_t opcode) { DP_OPER_MOV(dp_lsl_reg); }
/* MOV Rd, Rm, LSR Rs */
static void mov_lsr_reg(uint32_t opcode) { DP_OPER_MOV(dp_lsr_reg); }
/* MOV Rd, Rm, ASR Rs */
static void mov_asr_reg(uint32_t opcode) { DP_OPER_MOV(dp_asr_reg); }
/* MOV Rd, Rm, ROR Rs */
static void mov_ror_reg(uint32_t opcode) { DP_OPER_MOV(dp_ror_reg); }
/* MOVS Rd, Rm, LSL # */
static void movs_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_lsl_imm)); }
/* MOVS Rd, Rm, LSR # */
static void movs_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_lsr_imm)); }
/* MOVS Rd, Rm, ASR # */
static void movs_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_asr_imm)); }
/* MOVS Rd, Rm, ROR # */
static void movs_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_ror_imm)); }
/* MOVS Rd, Rm, LSL Rs */
static void movs_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_lsl_reg)); }
/* MOVS Rd, Rm, LSR Rs */
static void movs_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_lsr_reg)); }
/* MOVS Rd, Rm, ASR Rs */
static void movs_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_asr_reg)); }
/* MOVS Rd, Rm, ROR Rs */
static void movs_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_ror_reg)); }
/* BIC Rd, Rn, Rm, LSL # */
static void bic_lsl_imm(uint32_t opcode) { DP_OPER_BIC(dp_lsl_imm); }
/* BIC Rd, Rn, Rm, LSR # */
static void bic_lsr_imm(uint32_t opcode) { DP_OPER_BIC(dp_lsr_imm); }
/* BIC Rd, Rn, Rm, ASR # */
static void bic_asr_imm(uint32_t opcode) { DP_OPER_BIC(dp_asr_imm); }
/* BIC Rd, Rn, Rm, ROR # */
static void bic_ror_imm(uint32_t opcode) { DP_OPER_BIC(dp_ror_imm); }
/* BIC Rd, Rn, Rm, LSL Rs */
static void bic_lsl_reg(uint32_t opcode) { DP_OPER_BIC(dp_lsl_reg); }
/* BIC Rd, Rn, Rm, LSR Rs */
static void bic_lsr_reg(uint32_t opcode) { DP_OPER_BIC(dp_lsr_reg); }
/* BIC Rd, Rn, Rm, ASR Rs */
static void bic_asr_reg(uint32_t opcode) { DP_OPER_BIC(dp_asr_reg); }
/* BIC Rd, Rn, Rm, ROR Rs */
static void bic_ror_reg(uint32_t opcode) { DP_OPER_BIC(dp_ror_reg); }
/* BICS Rd, Rn, Rm, LSL # */
static void bics_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_lsl_imm)); }
/* BICS Rd, Rn, Rm, LSR # */
static void bics_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_lsr_imm)); }
/* BICS Rd, Rn, Rm, ASR # */
static void bics_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_asr_imm)); }
/* BICS Rd, Rn, Rm, ROR # */
static void bics_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_ror_imm)); }
/* BICS Rd, Rn, Rm, LSL Rs */
static void bics_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_lsl_reg)); }
/* BICS Rd, Rn, Rm, LSR Rs */
static void bics_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_lsr_reg)); }
/* BICS Rd, Rn, Rm, ASR Rs */
static void bics_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_asr_reg)); }
/* BICS Rd, Rn, Rm, ROR Rs */
static void bics_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_ror_reg)); }
/* MVN Rd, Rm, LSL # */
static void mvn_lsl_imm(uint32_t opcode) { DP_OPER_MVN(dp_lsl_imm); }
/* MVN Rd, Rm, LSR # */
static void mvn_lsr_imm(uint32_t opcode) { DP_OPER_MVN(dp_lsr_imm); }
/* MVN Rd, Rm, ASR # */
static void mvn_asr_imm(uint32_t opcode) { DP_OPER_MVN(dp_asr_imm); }
/* MVN Rd, Rm, ROR # */
static void mvn_ror_imm(uint32_t opcode) { DP_OPER_MVN(dp_ror_imm); }
/* MVN Rd, Rm, LSL Rs */
static void mvn_lsl_reg(uint32_t opcode) { DP_OPER_MVN(dp_lsl_reg); }
/* MVN Rd, Rm, LSR Rs */
static void mvn_lsr_reg(uint32_t opcode) { DP_OPER_MVN(dp_lsr_reg); }
/* MVN Rd, Rm, ASR Rs */
static void mvn_asr_reg(uint32_t opcode) { DP_OPER_MVN(dp_asr_reg); }
/* MVN Rd, Rm, ROR Rs */
static void mvn_ror_reg(uint32_t opcode) { DP_OPER_MVN(dp_ror_reg); }
/* MVNS Rd, Rm, LSL # */
static void mvns_lsl_imm(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_lsl_imm)); }
/* MVNS Rd, Rm, LSR # */
static void mvns_lsr_imm(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_lsr_imm)); }
/* MVNS Rd, Rm, ASR # */
static void mvns_asr_imm(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_asr_imm)); }
/* MVNS Rd, Rm, ROR # */
static void mvns_ror_imm(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_ror_imm)); }
/* MVNS Rd, Rm, LSL Rs */
static void mvns_lsl_reg(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_lsl_reg)); }
/* MVNS Rd, Rm, LSR Rs */
static void mvns_lsr_reg(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_lsr_reg)); }
/* MVNS Rd, Rm, ASR Rs */
static void mvns_asr_reg(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_asr_reg)); }
/* MVNS Rd, Rm, ROR Rs */
static void mvns_ror_reg(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_ror_reg)); }
/* AND Rd, Rn, #<immediate value> */
static void and_imm(uint32_t opcode) { DP_OPER_AND(dp_imm); }
/* ANDS Rd, Rn, #<immediate value> */
static void ands_imm(uint32_t opcode) { DP_CCL(DP_OPER_AND(dp_imm)); }
/* EOR Rd, Rn, #<immediate value> */
static void eor_imm(uint32_t opcode) { DP_OPER_EOR(dp_imm); }
/* EORS Rd, Rn, #<immediate value> */
static void eors_imm(uint32_t opcode) { DP_CCL(DP_OPER_EOR(dp_imm)); }
/* SUB Rd, Rn, #<immediate value> */
static void sub_imm(uint32_t opcode) { DP_OPER_SUB(dp_imm); }
/* SUBS Rd, Rn, #<immediate value> */
static void subs_imm(uint32_t opcode) { DP_OPER_SUBS(dp_imm); }
/* RSB Rd, Rn, #<immediate value> */
static void rsb_imm(uint32_t opcode) { DP_OPER_RSB(dp_imm); }
/* RSBS Rd, Rn, #<immediate value> */
static void rsbs_imm(uint32_t opcode) { DP_OPER_RSBS(dp_imm); }
/* ADD Rd, Rn, #<immediate value> */
static void add_imm(uint32_t opcode) { DP_OPER_ADD(dp_imm); }
/* ADDS Rd, Rn, #<immediate value> */
static void adds_imm(uint32_t opcode) { DP_OPER_ADDS(dp_imm); }
/* ADC Rd, Rn, #<immediate value> */
static void adc_imm(uint32_t opcode) { DP_OPER_ADC(dp_imm); }
/* ADCS Rd, Rn, #<immediate value> */
static void adcs_imm(uint32_t opcode) { DP_OPER_ADCS(dp_imm); }
/* SBC Rd, Rn, #<immediate value> */
static void sbc_imm(uint32_t opcode) { DP_OPER_SBC(dp_imm); }
/* SBCS Rd, Rn, #<immediate value> */
static void sbcs_imm(uint32_t opcode) { DP_OPER_SBCS(dp_imm); }
/* RSC Rd, Rn, #<immediate value> */
static void rsc_imm(uint32_t opcode) { DP_OPER_RSC(dp_imm); }
/* RSCS Rd, Rn, #<immediate value> */
static void rscs_imm(uint32_t opcode) { DP_OPER_RSCS(dp_imm); }
/* TST Rn, #<immediate value> */
static void tst_imm(uint32_t opcode) { DP_CCL(DP_OPER_TST(dp_imm)); }
/* TEQ Rn, #<immediate value> */
static void teq_imm(uint32_t opcode) { DP_CCL(DP_OPER_TEQ(dp_imm)); }
/* CMP Rn, #<immediate value> */
static void cmp_imm(uint32_t opcode) { DP_OPER_CMP(dp_imm); }
/* CMN Rn, #<immediate value> */
static void cmn_imm(uint32_t opcode) { DP_OPER_CMN(dp_imm); }
/* ORR Rd, Rn, #<immediate value> */
static void orr_imm(uint32_t opcode) { DP_OPER_ORR(dp_imm); }
/* ORRS Rd, Rn, #<immediate value> */
static void orrs_imm(uint32_t opcode) { DP_CCL(DP_OPER_ORR(dp_imm)); }
/* MOV Rd, #<immediate value> */
static void mov_imm(uint32_t opcode) { DP_OPER_MOV(dp_imm); }
/* MOVS Rd, #<immediate value> */
static void movs_imm(uint32_t opcode) { DP_CCL(DP_OPER_MOV(dp_imm)); }
/* BIC Rd, Rn, #<immediate value> */
static void bic_imm(uint32_t opcode) { DP_OPER_BIC(dp_imm); }
/* BICS Rd, Rn, #<immediate value> */
static void bics_imm(uint32_t opcode) { DP_CCL(DP_OPER_BIC(dp_imm)); }
/* MVN Rd, #<immediate value> */
static void mvn_imm(uint32_t opcode) { DP_OPER_MVN(dp_imm); }
/* MVNS Rd, #<immediate value> */
static void mvns_imm(uint32_t opcode) { DP_CCL(DP_OPER_MVN(dp_imm)); }

/* clang-format on */

arm_instr_t arm_instr[0xfff] = {
    /* 0x000 ... 0x01f */ INSTR_DP_REG(and),
    /* 0x020 ... 0x03f */ INSTR_DP_REG(eor),
    /* 0x040 ... 0x05f */ INSTR_DP_REG(sub),
    /* 0x060 ... 0x07f */ INSTR_DP_REG(rsb),
    /* 0x080 ... 0x09f */ INSTR_DP_REG(add),
    /* 0x0a0 ... 0x0bf */ INSTR_DP_REG(adc),
    /* 0x0c0 ... 0x0df */ INSTR_DP_REG(sbc),
    /* 0x0e0 ... 0x0ff */ INSTR_DP_REG(rsc),
    /* 0x100 ... 0x11f */ INSTR_DP_REG_NO_RD(tst),
    /* 0x120 ... 0x13f */ INSTR_DP_REG_NO_RD(teq),
    /* 0x140 ... 0x15f */ INSTR_DP_REG_NO_RD(cmp),
    /* 0x160 ... 0x17f */ INSTR_DP_REG_NO_RD(cmn),
    /* 0x180 ... 0x19f */ INSTR_DP_REG(orr),
    /* 0x1a0 ... 0x1bf */ INSTR_DP_REG(mov),
    /* 0x1c0 ... 0x1df */ INSTR_DP_REG(bic),
    /* 0x1e0 ... 0x1ff */ INSTR_DP_REG(mvn),
    [0x200 ... 0x20f] = and_imm,
    [0x210 ... 0x21f] = ands_imm,
    [0x220 ... 0x22f] = eor_imm,
    [0x230 ... 0x23f] = eors_imm,
    [0x240 ... 0x24f] = sub_imm,
    [0x250 ... 0x25f] = subs_imm,
    [0x260 ... 0x26f] = rsb_imm,
    [0x270 ... 0x27f] = rsbs_imm,
    [0x280 ... 0x28f] = add_imm,
    [0x290 ... 0x29f] = adds_imm,
    [0x2a0 ... 0x2af] = adc_imm,
    [0x2b0 ... 0x2bf] = adcs_imm,
    [0x2c0 ... 0x2cf] = sbc_imm,
    [0x2d0 ... 0x2df] = sbcs_imm,
    [0x2e0 ... 0x2ef] = rsc_imm,
    [0x2f0 ... 0x2ff] = rscs_imm,
    [0x300 ... 0x31f] = tst_imm,
    [0x320 ... 0x33f] = teq_imm,
    [0x340 ... 0x35f] = cmp_imm,
    [0x360 ... 0x37f] = cmn_imm,
    [0x380 ... 0x38f] = orr_imm,
    [0x390 ... 0x39f] = orrs_imm,
    [0x3a0 ... 0x3af] = mov_imm,
    [0x3b0 ... 0x3bf] = movs_imm,
    [0x3c0 ... 0x3cf] = bic_imm,
    [0x3d0 ... 0x3df] = bics_imm,
    [0x3e0 ... 0x3ef] = mvn_imm,
    [0x3f0 ... 0x3ff] = mvns_imm,
};
