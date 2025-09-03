#include "../unity/src/unity.h"
#include "../src/chmap.h"

void setUp(void) {}
void tearDown(void) {}


void chmap_get_char(void) {
    struct chmap * map = chmap_new(sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, sizeof(char), &put, sizeof(char));

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_EQUAL_UINT8('A', *got);
}

void chmap_get_null(void) {
    struct chmap * map = chmap_new(sizeof(char));

    char key = 'B';

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_NULL(got);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_get_char);
    RUN_TEST(chmap_get_null);
    return UNITY_END();
}
