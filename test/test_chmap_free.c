#include "../unity/src/unity.h"
#include "../chmap_onefile.h"
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

/**
 * There isn't a very good way to test the amount of memory your program holds in the program.
 * To test this, just run this build through valgrind :)
 */

void chmap_free_char(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, &put);

    const char * got = chmap_get(map, &key);

    TEST_ASSERT_EQUAL_UINT8('A', *got);

    chmap_free(map);
}

void chmap_free_uint64(void) {
    struct chmap * map = chmap_new(sizeof(uint64_t), sizeof(char));

    char key = 'B';
    uint64_t x = 40184308235;

    chmap_put(map, &key, &x);

    const uint64_t * got = chmap_get(map, &key);

    chmap_free(map);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_free_char);
    RUN_TEST(chmap_free_uint64);
    return UNITY_END();
}
