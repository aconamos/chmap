#include "../unity/src/unity.h"
#include "../src/chmap.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}


void chmap_del_one_char(void) {
    struct chmap * map = chmap_new(sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, sizeof(char), &put);

    chmap_del(map, &key, sizeof(char));

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_NULL(got);
}

void chmap_del_overwritten(void) {
    struct chmap * map = chmap_new(sizeof(char));

    char put = 'A';
    char key = 'B';

    chmap_put(map, &key, sizeof(char), &put);

    put = 'Z';

    chmap_put(map, &key, sizeof(char), &put);

    chmap_del(map, &key, sizeof(char));

    const char * got = chmap_get(map, &key, sizeof(char));

    TEST_ASSERT_NULL(got);
}

void chmap_del_many_sequential(void) {
    struct chmap * map = chmap_new(sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, 1, &val);
        chmap_del(map, &key, sizeof(char));
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key, 1);

        TEST_ASSERT_NULL(got);
    }
}

void chmap_del_many_bulk(void) {
    struct chmap * map = chmap_new(sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, 1, &val);
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        chmap_del(map, &key, 1);
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key, 1);

        TEST_ASSERT_NULL(got);
    }
}

void chmap_del_one_from_many(void) {
    struct chmap * map = chmap_new(sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 25;

        chmap_put(map, &key, 1, &val);
    }

    const char del_key = 'F';

    chmap_del(map, &del_key, sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key, 1);


        if (key == del_key) {
            TEST_ASSERT_NULL(got);
        } else {
            TEST_ASSERT_EQUAL(key + 25, *got);
        }
    }
}

void chmap_del_large_key(void) {
    struct chmap * map = chmap_new(sizeof(char));

    const char * key = "According to all known laws of aviation, there is no way a bee should be able to fly.";
    char val = 'a';

    chmap_put(map, key, strlen(key), &val);

    chmap_del(map, key, strlen(key));

    const char * got = chmap_get(map, key, strlen(key));

    TEST_ASSERT_EQUAL_UINT8(*got, 'a');
}

void chmap_del_many_repeatedly(void) {
    struct chmap * map = chmap_new(sizeof(char));

    for (int i = 0; i < 20; i++) {
        for (char key = 'A'; key < 'L'; key = (char) key + 1) {
            char val = key + 25;

            chmap_put(map, &key, 1, &val);
            chmap_del(map, &key, sizeof(char));
        }
    }

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        const char * got = chmap_get(map, &key, 1);

        TEST_ASSERT_NULL(got);
    }

    TEST_ASSERT_EQUAL_size_t(0, map->__used_size);
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
    return UNITY_END();
}
