#include "arm.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arm_isa.h"
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

static bool evaluate_cond(arm_condition_t cond, arm_psr_t cpsr)
{
    switch (cond) {
        case ARM_COND_EQ:
            return cpsr.z;
        case ARM_COND_NE:
            return !cpsr.z;
        case ARM_COND_CS:
            return cpsr.c;
        case ARM_COND_CC:
            return !cpsr.c;
        case ARM_COND_MI:
            return cpsr.n;
        case ARM_COND_PL:
            return !cpsr.n;
        case ARM_COND_VS:
            return cpsr.v;
        case ARM_COND_VC:
            return !cpsr.v;
        case ARM_COND_HI:
            return cpsr.c && !cpsr.z;
        case ARM_COND_LS:
            return !cpsr.c || cpsr.z;
        case ARM_COND_GE:
            return cpsr.n == cpsr.v;
        case ARM_COND_LT:
            return cpsr.n != cpsr.v;
        case ARM_COND_GT:
            return !cpsr.z && cpsr.n == cpsr.v;
        case ARM_COND_LE:
            return cpsr.z || cpsr.n != cpsr.v;
        case ARM_COND_AL:
            return true;
        case ARM_COND_RS:
        default:
            abort();
    }
}

void arm_init(void)
{
    memset(&arm, 0, sizeof(arm));
    arm_reset();
}

void arm_reset(void)
{
    arm.r14_svc = arm.r[PC];
    arm.spsr_svc = arm.cpsr;
    arm.cpsr.psr = ARM_PSR_IRQ_DISABLE | ARM_PSR_FIQ_DISABLE | ARM_PSR_SVC_MODE;
    arm.r[PC] = 0;
}

static uint32_t arm_fetch(void)
{
    uint32_t pc = arm.r[PC];
    uint32_t addr = arm.cpsr.t ? pc >> 1 : pc >> 2;
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
        arm_instr[code](opcode);
    }
}

void arm_step(void)
{
    arm_execute();
}
