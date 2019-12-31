#include "arm_isa.h"
#include "arm.h"

/* Get register from opcode offset. */
#define OPCODE_REG(offset) ((opcode >> offset) & 0xfu)

/* Data processing operations. */
#define DP_OPER_AND(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] & func(opcode))
#define DP_OPER_EOR(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] ^ func(opcode))
#define DP_OPER_SUB(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] - func(opcode))
#define DP_OPER_RSB(func) \
    (arm.r[OPCODE_REG(12)] = func(opcode) - arm.r[OPCODE_REG(16)])
#define DP_OPER_ADD(func) \
    (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] + func(opcode))

/* Set condition codes for data processing operation. */
#define DP_OPER_CC(val)                  \
    do {                                 \
        arm.shift_carry = 0;             \
        arm_psr_set_nzc(&arm.cpsr, val); \
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

static inline void arm_psr_set_nzc(arm_psr_t *psr, uint32_t d)
{
    uint32_t n = d & ARM_PSR_NEGATIVE;
    uint32_t z = !d << ARM_PSR_ZERO_SHIFT;
    uint32_t c = arm.shift_carry << ARM_PSR_CARRY_SHIFT;
    psr->psr = n | z | c | (psr->psr & 0x1fffffff);
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
static void ands_lsl_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_lsl_imm)); }
/* ANDS Rd, Rn, Rm, LSR # */
static void ands_lsr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_lsr_imm)); }
/* ANDS Rd, Rn, Rm, ASR # */
static void ands_asr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_asr_imm)); }
/* ANDS Rd, Rn, Rm, ROR # */
static void ands_ror_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_ror_imm)); }
/* ANDS Rd, Rn, Rm, LSL Rs */
static void ands_lsl_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_lsl_reg)); }
/* ANDS Rd, Rn, Rm, LSR Rs */
static void ands_lsr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_lsr_reg)); }
/* ANDS Rd, Rn, Rm, ASR Rs */
static void ands_asr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_asr_reg)); }
/* ANDS Rd, Rn, Rm, ROR Rs */
static void ands_ror_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_AND(dp_ror_reg)); }
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
static void eors_lsl_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_lsl_imm)); }
/* EORS Rd, Rn, Rm, LSR # */
static void eors_lsr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_lsr_imm)); }
/* EORS Rd, Rn, Rm, ASR # */
static void eors_asr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_asr_imm)); }
/* EORS Rd, Rn, Rm, ROR # */
static void eors_ror_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_ror_imm)); }
/* EORS Rd, Rn, Rm, LSL Rs */
static void eors_lsl_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_lsl_reg)); }
/* EORS Rd, Rn, Rm, LSR Rs */
static void eors_lsr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_lsr_reg)); }
/* EORS Rd, Rn, Rm, ASR Rs */
static void eors_asr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_asr_reg)); }
/* EORS Rd, Rn, Rm, ROR Rs */
static void eors_ror_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_EOR(dp_ror_reg)); }
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
static void subs_lsl_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_lsl_imm)); }
/* SUBS Rd, Rn, Rm, LSR # */
static void subs_lsr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_lsr_imm)); }
/* SUBS Rd, Rn, Rm, ASR # */
static void subs_asr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_asr_imm)); }
/* SUBS Rd, Rn, Rm, ROR # */
static void subs_ror_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_ror_imm)); }
/* SUBS Rd, Rn, Rm, LSL Rs */
static void subs_lsl_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_lsl_reg)); }
/* SUBS Rd, Rn, Rm, LSR Rs */
static void subs_lsr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_lsr_reg)); }
/* SUBS Rd, Rn, Rm, ASR Rs */
static void subs_asr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_asr_reg)); }
/* SUBS Rd, Rn, Rm, ROR Rs */
static void subs_ror_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_SUB(dp_ror_reg)); }
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
static void rsbs_lsl_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_lsl_imm)); }
/* RSBS Rd, Rn, Rm, LSR # */
static void rsbs_lsr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_lsr_imm)); }
/* RSBS Rd, Rn, Rm, ASR # */
static void rsbs_asr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_asr_imm)); }
/* RSBS Rd, Rn, Rm, ROR # */
static void rsbs_ror_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_ror_imm)); }
/* RSBS Rd, Rn, Rm, LSL Rs */
static void rsbs_lsl_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_lsl_reg)); }
/* RSBS Rd, Rn, Rm, LSR Rs */
static void rsbs_lsr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_lsr_reg)); }
/* RSBS Rd, Rn, Rm, ASR Rs */
static void rsbs_asr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_asr_reg)); }
/* RSBS Rd, Rn, Rm, ROR Rs */
static void rsbs_ror_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_RSB(dp_ror_reg)); }
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
static void adds_lsl_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_lsl_imm)); }
/* ADDS Rd, Rn, Rm, LSR # */
static void adds_lsr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_lsr_imm)); }
/* ADDS Rd, Rn, Rm, ASR # */
static void adds_asr_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_asr_imm)); }
/* ADDS Rd, Rn, Rm, ROR # */
static void adds_ror_imm(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_ror_imm)); }
/* ADDS Rd, Rn, Rm, LSL Rs */
static void adds_lsl_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_lsl_reg)); }
/* ADDS Rd, Rn, Rm, LSR Rs */
static void adds_lsr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_lsr_reg)); }
/* ADDS Rd, Rn, Rm, ASR Rs */
static void adds_asr_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_asr_reg)); }
/* ADDS Rd, Rn, Rm, ROR Rs */
static void adds_ror_reg(uint32_t opcode) { DP_OPER_CC(DP_OPER_ADD(dp_ror_reg)); }

/* clang-format on */

arm_instr_t arm_instr[0xfff] = {
    /* 0x000 */
    INSTR_DP_REG(and),
    /* 0x020 */
    INSTR_DP_REG(eor),
    /* 0x040 */
    INSTR_DP_REG(sub),
    /* 0x060 */
    INSTR_DP_REG(rsb),
    /* 0x080 */
    INSTR_DP_REG(add),
};
