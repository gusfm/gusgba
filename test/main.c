#include "ut.h"
#include "test.h"

struct ut unit_test;

int main(void)
{
    lex_test();
    parser_test();
    arm7_test();
    ut_result();
    return 0;
}
