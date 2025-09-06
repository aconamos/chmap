#include "../unity/src/unity.h"
#include "../src/chmap.h"
#include <stdint.h>


void setUp(void) {}
void tearDown(void) {}


void chmap_put_can_grow(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (char key = 'A'; key < 'Z'; key = (char) key + 1) {
        char val = key + 25;

        printf("associating: %c -> %c\n", key, val);

        chmap_put(map, &key, &val);
    }

    printf("done putting\n");

    debug_map(map);

    for (char key = 'A'; key < 'Z'; key = (char) key + 1) {
        printf("getting %c\n", key);
        const char * got = chmap_get(map, &key);

        TEST_ASSERT_NOT_NULL_MESSAGE(got, "got a NULL pointer when we shouldn't have!");

        TEST_ASSERT_EQUAL_UINT8(key + 25, *got);
    }
}

int main(void) {
    printf("this has to work\n");
    UNITY_BEGIN();
    RUN_TEST(chmap_put_can_grow);
    return UNITY_END();
}
