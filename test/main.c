#include "test.h"
#include "ut.h"

struct ut unit_test;

int main(void)
{
    lex_test();
    parser_test();
    arm_test();
    ut_result();
    return 0;
}
