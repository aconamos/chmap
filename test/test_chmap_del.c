#include "../unity/src/unity.h"
#include "../src/chmap.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}


void chmap_del_one_char(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, &put);

    chmap_del(map, &key);

    const char * got = chmap_get(map, &key);

    TEST_ASSERT_NULL(got);
}

void chmap_del_overwritten(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, &put);

    put = 'Z';

    chmap_put(map, &key, &put);

    chmap_del(map, &key);

    const char * got = chmap_get(map, &key);

    TEST_ASSERT_NULL(got);
}

void chmap_del_many_sequential(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, &val);
        chmap_del(map, &key);
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key);

        TEST_ASSERT_NULL(got);
    }
}

void chmap_del_many_bulk(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, &val);
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        chmap_del(map, &key);
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key);

        TEST_ASSERT_NULL(got);
    }
}

void chmap_del_one_from_many(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, &val);
    }

    const char del_key = 'F';

    chmap_del(map, &del_key);

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key);


        if (key == del_key) {
            TEST_ASSERT_NULL(got);
        } else {
            TEST_ASSERT_EQUAL(key + 25, *got);
        }
    }
}

void chmap_del_large_key(void) {
    const char * key = "According to all known laws of aviation, there is no way a bee should be able to fly.";
    struct chmap * map = chmap_new(sizeof(char), strlen(key));
    char val = 'a';

    chmap_put(map, key, &val);

    chmap_del(map, key);

    const char * got = chmap_get(map, key);

    TEST_ASSERT_NULL(got);
}

void chmap_del_many_repeatedly(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (int i = 0; i < 20; i++) {
        for (char key = 'A'; key < 'L'; key = (char) key + 1) {
            char val = key + 25;

            chmap_put(map, &key, &val);
            chmap_del(map, &key);
        }
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key);

        TEST_ASSERT_NULL(got);
    }

    TEST_ASSERT_EQUAL_size_t(0, map->used_size);
}

void chmap_del_after_growing(void) {
    struct chmap * map = chmap_new(sizeof(char), sizeof(char));

    for (char key = 0; key < (char)0xFF; key++) {
        char put = key + 25;

        chmap_put(map, &key, &put);
    }

    for (char key = 0; key < (char)0xFF; key++) {
        chmap_del(map, &key);
    }

    for (char key = 0; key < (char)0xFF; key++) {
        const char * got = chmap_get(map, &key);

        TEST_ASSERT_NULL(got);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(chmap_del_one_char);
    RUN_TEST(chmap_del_overwritten);
    RUN_TEST(chmap_del_many_sequential);
    RUN_TEST(chmap_del_many_bulk);
    RUN_TEST(chmap_del_one_from_many);
    RUN_TEST(chmap_del_large_key);
    RUN_TEST(chmap_del_many_repeatedly);
    RUN_TEST(chmap_del_after_growing);
    return UNITY_END();
}
