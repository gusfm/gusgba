#ifndef ARM_ISA_H
#define ARM_ISA_H

#include <stdint.h>

typedef void (*arm_instr_t)(uint32_t opcode);

extern arm_instr_t arm_instr[0xfff];

#endif /* ARM_ISA_H */
