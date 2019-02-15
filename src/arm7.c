#include "arm7.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mmu.h"

arm7_t arm;

typedef enum {
    ARM7_COND_EQ, /* Z set */
    ARM7_COND_NE, /* Z clear */
    ARM7_COND_CS, /* C set */
    ARM7_COND_CC, /* C clear */
    ARM7_COND_MI, /* N set */
    ARM7_COND_PL, /* N clear */
    ARM7_COND_VS, /* V set */
    ARM7_COND_VC, /* V clear */
    ARM7_COND_HI, /* C set and Z clear */
    ARM7_COND_LS, /* C clear or Z set */
    ARM7_COND_GE, /* N equals V */
    ARM7_COND_LT, /* N not equal to V */
    ARM7_COND_GT, /* Z clear AND (N equals V) */
    ARM7_COND_LE, /* Z set OR (N not equal to V) */
    ARM7_COND_AL, /* (ignored) */
    ARM7_COND_RS, /* reserved */
} arm7_condition_t;

#define ARM7_PSR_EQ(psr) (psr & ARM7_PSR_ZERO)
#define ARM7_PSR_NE(psr) !(psr & ARM7_PSR_ZERO)
#define ARM7_PSR_CS(psr) (psr & ARM7_PSR_CARRY)
#define ARM7_PSR_CC(psr) !(psr & ARM7_PSR_CARRY)
#define ARM7_PSR_MI(psr) (psr & ARM7_PSR_NEGATIVE)
#define ARM7_PSR_PL(psr) !(psr & ARM7_PSR_NEGATIVE)
#define ARM7_PSR_VS(psr) (psr & ARM7_PSR_OVERFLOW)
#define ARM7_PSR_VC(psr) !(psr & ARM7_PSR_OVERFLOW)
#define ARM7_PSR_GE(psr) (ARM7_PSR_MI(psr) == ARM7_PSR_VS(psr))
#define ARM7_PSR_LT(psr) (ARM7_PSR_MI(psr) != ARM7_PSR_VS(psr))

static bool evaluate_cond(arm7_condition_t cond, uint32_t cpsr)
{
    switch (cond) {
        case ARM7_COND_EQ:
            return ARM7_PSR_EQ(cpsr);
        case ARM7_COND_NE:
            return ARM7_PSR_NE(cpsr);
        case ARM7_COND_CS:
            return ARM7_PSR_CS(cpsr);
        case ARM7_COND_CC:
            return ARM7_PSR_CC(cpsr);
        case ARM7_COND_MI:
            return ARM7_PSR_MI(cpsr);
        case ARM7_COND_PL:
            return ARM7_PSR_PL(cpsr);
        case ARM7_COND_VS:
            return ARM7_PSR_VS(cpsr);
        case ARM7_COND_VC:
            return ARM7_PSR_VC(cpsr);
        case ARM7_COND_HI:
            return ARM7_PSR_CS(cpsr) && ARM7_PSR_NE(cpsr);
        case ARM7_COND_LS:
            return ARM7_PSR_CC(cpsr) || ARM7_PSR_EQ(cpsr);
        case ARM7_COND_GE:
            return ARM7_PSR_GE(cpsr);
        case ARM7_COND_LT:
            return ARM7_PSR_LT(cpsr);
        case ARM7_COND_GT:
            return ARM7_PSR_NE(cpsr) && ARM7_PSR_GE(cpsr);
        case ARM7_COND_LE:
            return ARM7_PSR_EQ(cpsr) || ARM7_PSR_LT(cpsr);
        case ARM7_COND_AL:
            return true;
        case ARM7_COND_RS:
        default:
            abort();
    }
}

void arm7_init(void)
{
    memset(&arm, 0, sizeof(arm));
    arm7_reset();
}

void arm7_reset(void)
{
    arm.r14_svc = arm.r[PC];
    arm.spsr_svc = arm.cpsr;
    arm.cpsr = ARM7_PSR_IRQ_DISABLE | ARM7_PSR_FIQ_DISABLE | ARM7_PSR_SVC_MODE;
    arm.r[PC] = 0;
}

static uint32_t arm7_fetch(void)
{
    uint32_t pc = arm.r[PC];
    uint32_t addr = arm.cpsr & ARM7_PSR_STATE_BIT ? pc >> 1 : pc >> 2;
    return mmu_read_word(addr);
}

static void arm7_execute(void)
{
    uint32_t opcode = arm7_fetch();
    if (evaluate_cond(opcode >> 28, arm.cpsr)) {
    }
}

void arm7_step(void)
{
    arm7_execute();
}
