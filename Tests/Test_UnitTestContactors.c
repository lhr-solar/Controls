#include "../Unity/src/unity.h"
#include "fff.h"
#include "scalar.h"


DEFINE_FFF_GLOBALS;

void setUp(void) {}

void tearDown(void) {}

FAKE_VALUE_FUNC2(int, perform_addition, int, int);
void test_unittest(void){
    int num1 = 1;
    int num2 = 2;
    perform_addition_fake.return_val = 6;
    TEST_ASSERT_EQUAL(6, perform_scaling(num1, num2));
}


