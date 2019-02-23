#ifndef ARM7_ENC_H
#define ARM7_ENC_H

#include <stdint.h>

uint32_t arm7_enc_cond(int cond);
uint32_t arm7_enc_and_imm(int rd, int rn, int rm, int shf_tp, int shf_am);

#endif /* ARM7_ENC_H */
