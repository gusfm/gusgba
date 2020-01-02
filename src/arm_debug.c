#include "arm_debug.h"

#include <stdio.h>
#include <stdlib.h>

static const char *arm_debug_dp_get_code(uint32_t opcode)
{
    switch ((opcode >> 21) & 0xf) {
        case 0:
            return "and";
        case 1:
            return "eor";
        case 2:
            return "sub";
        case 3:
            return "rsb";
        case 4:
            return "add";
        case 5:
            return "adc";
        case 6:
            return "sbc";
        case 7:
            return "rsc";
        case 8:
            return "tst";
        case 9:
            return "teq";
        case 10:
            return "cmp";
        case 11:
            return "cmn";
        case 12:
            return "";
        case 13:
            return "";
        case 14:
            return "";
        case 15:
            return "";
        default:
            abort();
    }
}

static const char *shift_type(uint32_t opcode)
{
    switch ((opcode >> 5) & 3) {
        case 0:
            return "lsl";
        case 1:
            return "lsr";
        case 2:
            return "asr";
        case 3:
            return "ror";
        default:
            abort();
    }
}

static void arm_debug_dp(uint32_t opcode)
{
    const char *code = arm_debug_dp_get_code(opcode);
    const char *shift_tp = shift_type(opcode);
    const char *s = opcode & 0x100000 && ((opcode >> 21) & 0xf) < 8 ? "s" : "";
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    uint32_t rm = opcode & 0xf;
    char operand2[20];
    if (opcode & 0x10) {
        uint32_t rs = (opcode >> 8) & 0xf;
        snprintf(operand2, sizeof(operand2), "r%u, %s r%u\n", rm, shift_tp, rs);
    } else {
        uint32_t shift = (opcode >> 7) & 0x1f;
        if (shift == 0 && opcode & 0x60)
            shift = 32;
        if (shift == 0 && (opcode & 0x60) == 0)
            snprintf(operand2, sizeof(operand2), "r%u\n", rm);
        else
            snprintf(operand2, sizeof(operand2), "r%u, %s #%u\n", rm, shift_tp,
                     shift);
    }
    printf("%s%s r%u, r%u, %s", code, s, rd, rn, operand2);
}

static void (*instr_debug[0xfff])(uint32_t opcode) = {
    [0x000 ... 0x17f] = arm_debug_dp,
};

void arm_debug(uint32_t opcode)
{
    uint32_t code = ((opcode >> 16) & 0xff0) | ((opcode >> 4) & 0x0f);
    printf("[0x%.8x][0x%.3x] ", opcode, code);
    instr_debug[code](opcode);
}
