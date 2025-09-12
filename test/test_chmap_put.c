#include "../unity/src/unity.h"
#include "../chmap_onefile.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}


void chmap_put_char(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, &put);
}

void chmap_put_overwrite(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, &put);

    put = 'Z';

    chmap_put(map, &key, &put);
}

void chmap_put_many(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, &val);
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char * got = chmap_get(map, &key);

        TEST_ASSERT_EQUAL_UINT8(key + 25, *got);
    }
}

void chmap_put_large_key(void) {
    const char * key = "According to all known laws of aviation, there is no way a bee should be able to fly.";
    struct chmap * map = chmap_new(sizeof(char), strlen(key));
    char val = 'a';

    chmap_put(map, key, &val);

    const char * got = chmap_get(map, key);

    TEST_ASSERT_EQUAL_UINT8(*got, 'a');
}

void chmap_put_string_val(void) {
    const char * value = "According to all known laws of aviation, there is no way a bee should be able to fly.";
    const size_t value_len = strlen(value);
    
    struct chmap * map = chmap_new(sizeof(size_t), sizeof(char));

    char key = 'A';

    chmap_put(
        map,
        &key,
        &value
    );

    const char ** got = chmap_get(map, &key);

    TEST_ASSERT_EQUAL_STRING(value, *got);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_put_char);
    RUN_TEST(chmap_put_overwrite);
    RUN_TEST(chmap_put_many);
    RUN_TEST(chmap_put_large_key);
    RUN_TEST(chmap_put_string_val);
    return UNITY_END();
}
