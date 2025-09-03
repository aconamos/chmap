#include "../unity/src/unity.h"
#include "../src/chmap.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}


void chmap_put_char(void) {
    struct chmap * map = chmap_new(sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, sizeof(char), &put, sizeof(char));
}

void chmap_put_overwrite(void) {
    struct chmap * map = chmap_new(sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, sizeof(char), &put, sizeof(char));

    put = 'Z';

    chmap_put(map, &key, sizeof(char), &put, sizeof(char));

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_EQUAL_UINT8('Z', *got);
}

void chmap_put_many(void) {
    struct chmap * map = chmap_new(sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, 1, &val, sizeof(char));
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char *got = chmap_get(map, &key, 1);

        TEST_ASSERT_EQUAL_UINT8(key + 25, *got);
    }
}

void chmap_put_large_key(void) {
    struct chmap * map = chmap_new(sizeof(char));

    const char * key = "According to all known laws of aviation, there is no way a bee should be able to fly.";
    char val = 'a';

    chmap_put(map, key, strlen(key), &val, sizeof(char));

    const char * got = chmap_get(map, key, strlen(key));

    TEST_ASSERT_EQUAL_UINT8(*got, 'a');
}

void chmap_put_bad_string_val(void) {
    struct chmap * map = chmap_new(50);

    char key = 'A';

    // This is a deliberate misuse of the map. It will only read 50 bytes, and there
    // will be no null byte to terminate the string. This is a great way to have a
    // buffer overflow.
    chmap_put(
        map,
        &key, sizeof(char),
        "According to all known laws of aviation, there is no way a bee should be able to fly.",
        50
    );

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_EQUAL_STRING_LEN("According to all known laws of aviation, there is ", got, 50);
}

void chmap_put_string_val(void) {
    const char * value = "According to all known laws of aviation, there is no way a bee should be able to fly.";
    const size_t value_len = strlen(value);
    
    struct chmap * map = chmap_new(value_len);

    char key = 'A';

    chmap_put(
        map,
        &key, sizeof(char),
        value,
        value_len
    );

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_EQUAL_STRING(value, got);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_put_char);
    RUN_TEST(chmap_put_overwrite);
    RUN_TEST(chmap_put_many);
    RUN_TEST(chmap_put_large_key);
    RUN_TEST(chmap_put_bad_string_val);
    RUN_TEST(chmap_put_string_val);
    return UNITY_END();
}
