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
            return "orr";
        case 13:
            return "mov";
        case 14:
            return "bic";
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

static void arm_debug_dp_get_oper2(uint32_t opcode, char *oper2, size_t size)
{
    const char *shift_tp = shift_type(opcode);
    uint32_t rm = opcode & 0xf;
    if (opcode & 0x10) {
        uint32_t rs = (opcode >> 8) & 0xf;
        snprintf(oper2, size, "r%u, %s r%u", rm, shift_tp, rs);
    } else {
        uint32_t shift = (opcode >> 7) & 0x1f;
        if (shift == 0 && opcode & 0x60)
            shift = 32;
        if (shift == 0 && (opcode & 0x60) == 0)
            snprintf(oper2, size, "r%u", rm);
        else
            snprintf(oper2, size, "r%u, %s #%u", rm, shift_tp, shift);
    }
}

static void arm_debug_dp_rd_rn(uint32_t opcode)
{
    const char *code = arm_debug_dp_get_code(opcode);
    const char *s = opcode & 0x100000 ? "s" : "";
    uint32_t rd = (opcode >> 12) & 0xf;
    uint32_t rn = (opcode >> 16) & 0xf;
    char operand2[20];
    arm_debug_dp_get_oper2(opcode, operand2, sizeof(operand2));
    printf("%s%s r%u, r%u, %s\n", code, s, rd, rn, operand2);
}

static void arm_debug_dp_rn(uint32_t opcode)
{
    const char *code = arm_debug_dp_get_code(opcode);
    uint32_t rn = (opcode >> 16) & 0xf;
    char operand2[20];
    arm_debug_dp_get_oper2(opcode, operand2, sizeof(operand2));
    printf("%s r%u, %s\n", code, rn, operand2);
}

static void arm_debug_dp_rd(uint32_t opcode)
{
    const char *code = arm_debug_dp_get_code(opcode);
    const char *s = opcode & 0x100000 ? "s" : "";
    uint32_t rd = (opcode >> 12) & 0xf;
    char operand2[20];
    arm_debug_dp_get_oper2(opcode, operand2, sizeof(operand2));
    printf("%s%s r%u, %s\n", code, s, rd, operand2);
}

static void (*instr_debug[0xfff])(uint32_t opcode) = {
    [0x000 ... 0x0ff] = arm_debug_dp_rd_rn,
    [0x100 ... 0x17f] = arm_debug_dp_rn,
    [0x180 ... 0x19f] = arm_debug_dp_rd_rn,
    [0x1a0 ... 0x1bf] = arm_debug_dp_rd,
    [0x1c0 ... 0x1df] = arm_debug_dp_rd_rn,
};

void arm_debug(uint32_t opcode)
{
    uint32_t code = ((opcode >> 16) & 0xff0) | ((opcode >> 4) & 0x0f);
    printf("[0x%.8x][0x%.3x] ", opcode, code);
    instr_debug[code](opcode);
}
