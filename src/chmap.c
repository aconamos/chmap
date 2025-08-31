#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "siphash.h"
#include "chmap.h"

#define DEFAULT_BACKING_ARRAY_LENGTH 20
#define MAX_LOAD_FACTOR 0.9f


static const char* SIPHASH_KEY = "abcdef9876543210";

struct __psl_ind { 
    uint64_t index; 
    size_t psl;
};

struct __entry*
__bubble_up(struct chmap *map, const uint64_t key, struct __psl_ind loc) {
    struct __entry working_entry = map->__translation_array[loc.index];
    struct __entry buffer_entry;
    struct __entry entry_to_insert = {
        .has_entry = 1,
        .keyword = key,
        .psl = loc.psl,
    };
}

/**
 * Find the next spot for a key
 */
struct __psl_ind
__find_spot(struct chmap *map, const uint64_t key, size_t psl, uint64_t working_index) {
    struct __entry working_entry = map->__translation_array[working_index];

    while (working_entry.has_entry == 1 && working_entry.keyword != key && working_entry.psl >= psl) {
        working_entry = map->__translation_array[working_index];
        working_index = (working_index + 1) % map->__array_size;
        psl++;
    }

    return (struct __psl_ind){ working_index, psl};
}

/**
 * Given a chmap and a key, returns the index where that key should be inserted and the PSL.
 */
struct __psl_ind
__address_array(struct chmap *map, const uint64_t key) 
{
    uint64_t working_index = key % map->__array_size;
    size_t psl = 0;

    return __find_spot(map, key, psl, working_index);
}

struct __entry*
__init_translation_array(
    const size_t numentries
) {
    struct __entry *entries = calloc(numentries, sizeof(struct __entry));
    struct __entry empty = {
        .has_entry = 0,
    };

    for (size_t i = 0; i < numentries; i++) {
        entries[i] = empty;
    }

    return entries;
}


struct chmap*
chmap_new(
    const size_t item_size
) {
    void *backing_array = malloc(sizeof(item_size) * DEFAULT_BACKING_ARRAY_LENGTH);
    struct chmap *map = malloc(sizeof(struct chmap));

    map->__key_size = 0;
    map->__item_size = item_size;
    map->__used_size = 0;
    map->__array_size = DEFAULT_BACKING_ARRAY_LENGTH;
    map->__translation_array = __init_translation_array(DEFAULT_BACKING_ARRAY_LENGTH);
    map->__backing_array = backing_array;

    return map;
}

int
chmap_put(
    struct chmap *map,
    const void *key,
    const size_t keysize,
    const void *item,
    const size_t itemsize
) {
    // This is used in place of a uint8_t[8] to provide the same 8 bytes
    // but in a format easier to use as a key.
    uint64_t outword;

    assert(map->__used_size < map->__array_size * MAX_LOAD_FACTOR);

    siphash(key, keysize, SIPHASH_KEY, (uint8_t*)&outword, 8);

    struct __psl_ind insert_at = __address_array(map, outword);
    const struct __entry looking_at = map->__translation_array[insert_at.index];

    if (looking_at.has_entry == 0) {
        // We found an empty spot - put it in, no fuss
        size_t bak = map->__used_size;
        map->__used_size = (map->__used_size + 1) % map->__array_size;
        struct __entry new_entry = {
            1,
            insert_at.psl,
            .backing_array_key = bak,
            .keyword = outword,
        };

        
        map->__translation_array[insert_at.index] = new_entry;

        void *ba_ptr = (size_t*)map->__backing_array + bak * itemsize;

        memcpy(ba_ptr, item, itemsize);
    } else if (looking_at.keyword == outword) {
        // This key already is associated - overwrite it
        void *ba_ptr = (size_t*)map->__backing_array + looking_at.backing_array_key * itemsize;

        memcpy(ba_ptr, item, itemsize);
    } else {
        // We need to now swap the two out, and put the next one somewhere else down in the array.
    }
    printf("idx is: %2lu; psl: %2lu; hash is %lu \n", insert_at.index, insert_at.psl, outword);

    return 0;
}

void*
chmap_get(
    struct chmap *map,
    const void *key,
    const size_t keysize
) {
    uint64_t outword;

    siphash(key, keysize, SIPHASH_KEY, (uint8_t*)&outword, 8);

    struct __psl_ind loc = __address_array(map, outword);
    struct __entry maybe = map->__translation_array[loc.index];

    if (maybe.has_entry) {
        return ((size_t*)map->__backing_array + maybe.backing_array_key * map->__item_size);
    } else {
        return NULL;
    }
}

void
debug_map(
    struct chmap *map
) {
    for (size_t i = 0; i < map->__array_size; i++) {
        struct __entry entry = map->__translation_array[i];

        if (entry.has_entry)
        printf("\
entry.backing_array_key: %lu\n\
entry.has_entry: %d\n\
entry.keyword: %lu\n\
entry.psl: %lu\n\n",
        entry.backing_array_key, 
        entry.has_entry, 
        entry.keyword, 
        entry.psl
        );
    }
}
