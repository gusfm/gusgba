#include "arm.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmu.h"

#define CPU_DEBUG /* debugs are enabled for now */
#ifdef CPU_DEBUG
#include "arm_debug.h"
#endif

arm_t arm;

typedef enum {
    ARM_COND_EQ, /* Z set */
    ARM_COND_NE, /* Z clear */
    ARM_COND_CS, /* C set */
    ARM_COND_CC, /* C clear */
    ARM_COND_MI, /* N set */
    ARM_COND_PL, /* N clear */
    ARM_COND_VS, /* V set */
    ARM_COND_VC, /* V clear */
    ARM_COND_HI, /* C set and Z clear */
    ARM_COND_LS, /* C clear or Z set */
    ARM_COND_GE, /* N equals V */
    ARM_COND_LT, /* N not equal to V */
    ARM_COND_GT, /* Z clear AND (N equals V) */
    ARM_COND_LE, /* Z set OR (N not equal to V) */
    ARM_COND_AL, /* (ignored) */
    ARM_COND_RS, /* reserved */
} arm_condition_t;

#define ARM_PSR_EQ(psr) (psr & ARM_PSR_ZERO)
#define ARM_PSR_NE(psr) !(psr & ARM_PSR_ZERO)
#define ARM_PSR_CS(psr) (psr & ARM_PSR_CARRY)
#define ARM_PSR_CC(psr) !(psr & ARM_PSR_CARRY)
#define ARM_PSR_MI(psr) (psr & ARM_PSR_NEGATIVE)
#define ARM_PSR_PL(psr) !(psr & ARM_PSR_NEGATIVE)
#define ARM_PSR_VS(psr) (psr & ARM_PSR_OVERFLOW)
#define ARM_PSR_VC(psr) !(psr & ARM_PSR_OVERFLOW)
#define ARM_PSR_GE(psr) (ARM_PSR_MI(psr) == ARM_PSR_VS(psr))
#define ARM_PSR_LT(psr) (ARM_PSR_MI(psr) != ARM_PSR_VS(psr))

#define ARM_PSR_IS_SET(psr, flag) (psr & flag)
#define ARM_PSR_SET(psr, flag) (psr |= (flag))
#define ARM_PSR_CLEAR(psr, flag) (psr &= ~(flag))

#define OPCODE_REG(offset) ((opcode >> offset) & 0xfu)
#define RM(opcode) OPCODE_REG(0)
#define RS(opcode) OPCODE_REG(8)
#define OPER_DP_AND(func) (arm.r[OPCODE_REG(12)] = arm.r[OPCODE_REG(16)] & func(opcode))

#define INSTR_DP_REG(op)                                                      \
    op##_lsl_imm, op##_lsl_reg, op##_lsr_imm, op##_lsr_reg, op##_asr_imm,     \
        op##_asr_reg, op##_ror_imm, op##_ror_reg, op##_lsl_imm, op##_lsl_reg, \
        op##_lsr_imm, op##_lsr_reg, op##_asr_imm, op##_asr_reg, op##_ror_imm, \
        op##_ror_reg

typedef void (*instr_t)(uint32_t opcode);


static uint32_t asr_mask[32]
    = { 0xffffffff, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000,
        0xfc000000, 0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
        0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000,
        0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
        0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8,
        0xfffffffc, 0xfffffffe };

static uint32_t ror_mask[32]
    = { 0x00000000, 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f,
        0x0000003f, 0x0000007f, 0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
        0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff, 0x0001ffff,
        0x0003ffff, 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
        0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff,
        0x3fffffff, 0x7fffffff };

static bool evaluate_cond(arm_condition_t cond, uint32_t cpsr)
{
    switch (cond) {
        case ARM_COND_EQ:
            return ARM_PSR_EQ(cpsr);
        case ARM_COND_NE:
            return ARM_PSR_NE(cpsr);
        case ARM_COND_CS:
            return ARM_PSR_CS(cpsr);
        case ARM_COND_CC:
            return ARM_PSR_CC(cpsr);
        case ARM_COND_MI:
            return ARM_PSR_MI(cpsr);
        case ARM_COND_PL:
            return ARM_PSR_PL(cpsr);
        case ARM_COND_VS:
            return ARM_PSR_VS(cpsr);
        case ARM_COND_VC:
            return ARM_PSR_VC(cpsr);
        case ARM_COND_HI:
            return ARM_PSR_CS(cpsr) && ARM_PSR_NE(cpsr);
        case ARM_COND_LS:
            return ARM_PSR_CC(cpsr) || ARM_PSR_EQ(cpsr);
        case ARM_COND_GE:
            return ARM_PSR_GE(cpsr);
        case ARM_COND_LT:
            return ARM_PSR_LT(cpsr);
        case ARM_COND_GT:
            return ARM_PSR_NE(cpsr) && ARM_PSR_GE(cpsr);
        case ARM_COND_LE:
            return ARM_PSR_EQ(cpsr) || ARM_PSR_LT(cpsr);
        case ARM_COND_AL:
            return true;
        case ARM_COND_RS:
        default:
            abort();
    }
}

/* Data processing logical shift left immediate */
static inline uint32_t dp_lsl_imm(uint32_t opcode)
{
    uint32_t shift = (opcode >> 7) & 0x1f;
    return arm.r[RM(opcode)] << shift;
}

/* Data processing logical shift right immediate */
static inline uint32_t dp_lsr_imm(uint32_t opcode)
{
    uint32_t shift = (opcode >> 7) & 0x1f;
    if (!shift) {
        return 0;
    }
    return arm.r[RM(opcode)] >> shift;
}

/* Data processing arithmetic shift right immediate */
static inline uint32_t dp_asr_imm(uint32_t opcode)
{
    uint32_t shift = (opcode >> 7) & 0x1f;
    uint32_t val = arm.r[RM(opcode)];
    uint32_t ret = val >> shift;
    if (val & 0x80000000)
        ret |= asr_mask[shift];
    return ret;
}

/* Data processing rotate right immediate */
static inline uint32_t dp_ror_imm(uint32_t opcode)
{
    uint32_t shift = (opcode >> 7) & 0x1f;
    uint32_t val = arm.r[RM(opcode)];
    uint32_t carry = val & ror_mask[shift];
    return (carry << (32 - shift)) | (val >> shift);
}

/* Data processing logical shift left reg */
static inline uint32_t dp_lsl_reg(uint32_t opcode)
{
    uint32_t shift = (arm.r[RS(opcode)] & 0x1f);
    return arm.r[RM(opcode)] << shift;
}

/* Data processing logical shift right reg */
static inline uint32_t dp_lsr_reg(uint32_t opcode)
{
    uint32_t shift = (arm.r[RS(opcode)] & 0x1f);
    return arm.r[RM(opcode)] >> shift;
}

/* Data processing arithmetic shift right reg */
static inline uint32_t dp_asr_reg(uint32_t opcode)
{
    uint32_t shift = (arm.r[RS(opcode)] & 0x1f);
    uint32_t val = arm.r[RM(opcode)];
    uint32_t ret = val >> shift;
    if (val & 0x80000000)
        ret |= asr_mask[shift];
    return ret;
}

/* Data processing rotate right reg */
static inline uint32_t dp_ror_reg(uint32_t opcode)
{
    uint32_t shift = (arm.r[RS(opcode)] & 0x1f);
    uint32_t val = arm.r[RM(opcode)];
    uint32_t carry = val & ror_mask[shift];
    return (carry << (32 - shift)) | (val >> shift);
}

/* AND Rd, Rn, Rm, LSL # */
static void and_lsl_imm(uint32_t opcode) { OPER_DP_AND(dp_lsl_imm); }
/* AND Rd, Rn, Rm, LSR # */
static void and_lsr_imm(uint32_t opcode) { OPER_DP_AND(dp_lsr_imm); }
/* AND Rd, Rn, Rm, ASR # */
static void and_asr_imm(uint32_t opcode) { OPER_DP_AND(dp_asr_imm); }
/* AND Rd, Rn, Rm, ROR # */
static void and_ror_imm(uint32_t opcode) { OPER_DP_AND(dp_ror_imm); }
/* AND Rd, Rn, Rm, LSL Rs */
static void and_lsl_reg(uint32_t opcode) { OPER_DP_AND(dp_lsl_reg); }
/* AND Rd, Rn, Rm, LSR Rs */
static void and_lsr_reg(uint32_t opcode) { OPER_DP_AND(dp_lsr_reg); }
/* AND Rd, Rn, Rm, ASR Rs */
static void and_asr_reg(uint32_t opcode) { OPER_DP_AND(dp_asr_reg); }
/* AND Rd, Rn, Rm, ROR Rs */
static void and_ror_reg(uint32_t opcode) { OPER_DP_AND(dp_ror_reg); }

static instr_t instr[0xfff] = {
    /* 0x000 */
    INSTR_DP_REG(and)
    /* 0x010 */
};

void arm_init(void)
{
    memset(&arm, 0, sizeof(arm));
    arm_reset();
}

void arm_reset(void)
{
    arm.r14_svc = arm.r[PC];
    arm.spsr_svc = arm.cpsr;
    arm.cpsr = ARM_PSR_IRQ_DISABLE | ARM_PSR_FIQ_DISABLE | ARM_PSR_SVC_MODE;
    arm.r[PC] = 0;
}

static uint32_t arm_fetch(void)
{
    uint32_t pc = arm.r[PC];
    uint32_t addr = arm.cpsr & ARM_PSR_STATE_BIT ? pc >> 1 : pc >> 2;
    return mmu_read_word(addr);
}

static void arm_execute(void)
{
    uint32_t opcode = arm_fetch();
    if (evaluate_cond(opcode >> 28, arm.cpsr)) {
        uint32_t code = ((opcode >> 16) & 0xff0) | ((opcode >> 4) & 0x0f);
#ifdef CPU_DEBUG
        arm_debug(opcode);
#endif
        instr[code](opcode);
    }
}

void arm_step(void)
{
    arm_execute();
}
