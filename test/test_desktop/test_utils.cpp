#include <unity.h>
#include "utils.h"

void test_1MinusMax(void) {
    TEST_ASSERT_EQUAL_UINT32(2UL, 1UL - 0xffffffffUL);
}

void test_MaxMinusMax(void) {
    TEST_ASSERT_EQUAL_UINT32(0UL, ULONG_MAX - 0xffffffffUL);
}
