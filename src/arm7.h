#ifndef ARM7_H
#define ARM7_H

#include <stdint.h>

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
    uint32_t cpsr;
    uint32_t spsr_fiq;
    uint32_t spsr_svc;
    uint32_t spsr_abt;
    uint32_t spsr_irq;
    uint32_t spsr_und;
} arm7_t;

/* Registers */
#define SP 13
#define LR 14
#define PC 15

/* Control bits */
#define ARM7_PSR_USR_MODE (0x10)
#define ARM7_PSR_FIQ_MODE (0x11)
#define ARM7_PSR_IRQ_MODE (0x12)
#define ARM7_PSR_SVC_MODE (0x13)
#define ARM7_PSR_ABT_MODE (0x17)
#define ARM7_PSR_UND_MODE (0x1b)
#define ARM7_PSR_SYS_MODE (0x1f)
#define ARM7_PSR_STATE_BIT (1u << 5)
#define ARM7_PSR_FIQ_DISABLE (1u << 6)
#define ARM7_PSR_IRQ_DISABLE (1u << 7)

/* Condition code flags */
#define ARM7_PSR_OVERFLOW (1u << 28)
#define ARM7_PSR_CARRY (1u << 29)
#define ARM7_PSR_ZERO (1u << 30)
#define ARM7_PSR_NEGATIVE (1u << 31)

void arm7_init(void);
void arm7_reset(void);
void arm7_step(void);

#endif /* !ARM7_H */
