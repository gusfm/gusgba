#ifndef MMU_H
#define MMU_H

#include <stdint.h>

uint32_t mmu_read_word(uint32_t addr);
uint16_t mmu_read_half_word(uint32_t addr);

#endif /* !MMU_H */
