#include "arm_enc.h"

#define M1(val) ((uint32_t)val & 0x1u)
#define M2(val) ((uint32_t)val & 0x3u)
#define M4(val) ((uint32_t)val & 0xfu)
#define M5(val) ((uint32_t)val & 0x1fu)
#define M8(val) ((uint32_t)val & 0xffu)

static uint32_t enc_shift(shift_t *s)
{
    uint32_t val = M2(s->type) << 5 | M1(s->is_register) << 4 | M4(s->rm);
    if (s->is_register)
        val |= M4(s->rs) << 8;
    else
        val |= M5(s->amount) << 7;
    return val;
}

static uint32_t enc_imm_oper(imm_val_t *i)
{
    return M4(i->rotate) << 8 | M8(i->imm);
}

uint32_t arm_enc_and_imm(cond_t cond, reg_t rd, reg_t rn, oper2_t *o)
{
    uint32_t oper2 =
        o->is_imm_val ? enc_imm_oper(&o->imm_val) : enc_shift(&o->shift);
    return M4(cond) << 28 | M1(o->is_imm_val) << 25 | M4(rn) << 16 |
           M4(rd) << 12 | oper2;
}
