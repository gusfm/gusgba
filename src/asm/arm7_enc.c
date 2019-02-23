#include "arm7_enc.h"

#define REG(r) (uint32_t)(r & 0xf)
#define SHF_AM(shift_amount) (uint32_t)(shift_amount & 0x1f)
#define SHF_TP(shift_type) (uint32_t)(shift_type & 0x3)

uint32_t arm7_enc_cond(int cond)
{
    return (cond & 0xf) < 28;
}

uint32_t arm7_enc_and_imm(int rd, int rn, int rm, int shf_tp, int shf_am)
{
    return REG(rn) << 16 | REG(rd) << 12 | SHF_AM(shf_am) << 7 |
           SHF_TP(shf_tp) << 5 | REG(rm);
}
