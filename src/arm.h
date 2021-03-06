#ifndef ARM_H
#define ARM_H

#include <stdint.h>

typedef union {
    struct {
        unsigned int mode : 5;
        unsigned int t : 1;
        unsigned int f : 1;
        unsigned int i : 1;
        unsigned int reserved : 20;
        unsigned int v : 1;
        unsigned int c : 1;
        unsigned int z : 1;
        unsigned int n : 1;
    };
    uint32_t psr;
} arm_psr_t;

typedef struct {
    /* ARM-state general registers */
    uint32_t r[16];
    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
    uint32_t r13_fiq;
    uint32_t r14_fiq;
    uint32_t r13_svc;
    uint32_t r14_svc;
    uint32_t r13_abt;
    uint32_t r14_abt;
    uint32_t r13_irq;
    uint32_t r14_irq;
    uint32_t r13_und;
    uint32_t r14_und;
    /* ARM-state program status registers */
    arm_psr_t cpsr;
    arm_psr_t spsr_fiq;
    arm_psr_t spsr_svc;
    arm_psr_t spsr_abt;
    arm_psr_t spsr_irq;
    arm_psr_t spsr_und;
    /* Internal variables */
    uint32_t shift_carry;
} arm_t;

extern arm_t arm;

/* Registers */
#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define SP 13
#define LR 14
#define PC 15

/* Control bits */
#define ARM_PSR_USR_MODE (0x10)
#define ARM_PSR_FIQ_MODE (0x11)
#define ARM_PSR_IRQ_MODE (0x12)
#define ARM_PSR_SVC_MODE (0x13)
#define ARM_PSR_ABT_MODE (0x17)
#define ARM_PSR_UND_MODE (0x1b)
#define ARM_PSR_SYS_MODE (0x1f)
#define ARM_PSR_STATE_BIT (1u << 5)
#define ARM_PSR_FIQ_DISABLE (1u << 6)
#define ARM_PSR_IRQ_DISABLE (1u << 7)

/* PSR condition code bits */
#define ARM_PSR_OVERFLOW_SHIFT 28
#define ARM_PSR_CARRY_SHIFT 29
#define ARM_PSR_ZERO_SHIFT 30
#define ARM_PSR_NEGATIVE_SHIFT 31

#define ARM_PSR_OVERFLOW (1u << ARM_PSR_OVERFLOW_SHIFT)
#define ARM_PSR_CARRY (1u << ARM_PSR_CARRY_SHIFT)
#define ARM_PSR_ZERO (1u << ARM_PSR_ZERO_SHIFT)
#define ARM_PSR_NEGATIVE (1u << ARM_PSR_NEGATIVE_SHIFT)

void arm_init(void);
void arm_reset(void);
void arm_step(void);

#endif /* !ARM_H */
