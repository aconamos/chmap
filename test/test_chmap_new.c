#include "../unity/src/unity.h"
#include "../src/chmap.h"

void setUp(void) {}
void tearDown(void) {}


void chmap_new_char(void) {
    chmap_new(sizeof(char));
}

void chmap_new_uint64(void) {
    chmap_new(sizeof(uint64_t));
}

void chmap_new_large_struct(void) {
    chmap_new(500);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_new_char);
    RUN_TEST(chmap_new_uint64);
    RUN_TEST(chmap_new_large_struct);
    return UNITY_END();
}
