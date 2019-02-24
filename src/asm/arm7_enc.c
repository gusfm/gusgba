#include "arm7_enc.h"

#define M2(val) ((uint32_t)val & 0x3u)
#define M4(val) ((uint32_t)val & 0xfu)
#define M5(val) ((uint32_t)val & 0x1fu)

uint32_t arm7_enc_and_imm(int cond, int rd, int rn, int rm, int shf_tp,
                          int shf_am)
{
    return M4(cond) << 28 | M4(rn) << 16 | M4(rd) << 12 | M5(shf_am) << 7 |
           M2(shf_tp) << 5 | M4(rm);
}
