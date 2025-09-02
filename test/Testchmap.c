#include "../unity/src/unity.h"
#include "../unity/src/unity_internals.h"
#include <stdio.h>


void setUp(void) {
    printf("setup");
}

void tearDown(void) {
    printf("teardown");
}

void test_function_prints(void) {
    printf("yippee");
}

void test_function_prints2(void) {
    printf("eeppiy");
}

int
main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_prints);
    RUN_TEST(test_function_prints2);
    return UNITY_END();
}
