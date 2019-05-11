#include "arm7_enc.h"

#define M1(val) (val & 0x1u)
#define M2(val) (val & 0x3u)
#define M4(val) (val & 0xfu)
#define M5(val) (val & 0x1fu)

static uint32_t enc_shift(shift_t *s)
{
    uint32_t val = M2(s->type) << 5 | M1(s->is_register) << 4 | M4(s->rm);
    if (s->is_register)
        val |= M4(s->rs) << 8;
    else
        val |= M5(s->amount) << 7;
    return val;
}

uint32_t arm7_enc_and_imm(cond_t cond, reg_t rd, reg_t rn, shift_t *s)
{
    uint32_t shift = enc_shift(s);
    return M4(cond) << 28 | M4(rn) << 16 | M4(rd) << 12 | shift;
}
