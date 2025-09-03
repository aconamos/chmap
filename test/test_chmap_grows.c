#include "../unity/src/unity.h"
#include "../src/chmap.h"
#include <stdint.h>


void setUp(void) {}
void tearDown(void) {}


void chmap_put_can_grow(void) {
    struct chmap * map = chmap_new(sizeof(char));

    for (char key = 'A'; key < 'z'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, 1, &val, sizeof(char));
    }

    for (char key = 'A'; key < 'z'; key = (char) key + 1) {
        char *got = chmap_get(map, &key, 1);

        TEST_ASSERT_EQUAL_UINT8(key + 25, *got);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_put_can_grow);
    return UNITY_END();
}
