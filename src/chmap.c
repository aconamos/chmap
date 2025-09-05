#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "siphash.h"
#include "chmap.h"

#define DEFAULT_BACKING_ARRAY_LENGTH 20
#define ARRAY_GROW_FACTOR 2.0f
#define MAX_LOAD_FACTOR 0.9f


static const char * SIPHASH_KEY = "abcdef9876543210";

struct __psl_ind { 
    uint64_t index; 
    size_t psl;
};

/* FORWARD DECLS */

void debug_map_params(struct chmap * map);

/**
 * This puts in an item using a hash (instead of the key).
 */
static int
chmap_put_hash(
    struct chmap * map,
    const uint64_t hash,
    const void * item
);

/**
 * Takes an entry and a location and tries to insert it at the location, performing
 * robinhood shuffling if necessary to maintain low PSL or whatever.
 */
static void
bubble_up(struct chmap * map, const struct __entry inentry, size_t ind) {
    struct __entry buffer_entry;
    struct __entry grabbed_entry = inentry;

    do {
        struct __entry working_entry = map->__translation_array[ind];

        if (working_entry.has_entry == 0) {
            // We've encountered an empty spot, and can insert and jump ship.
            map->__translation_array[ind] = grabbed_entry;
            return;
        }

        if (working_entry.psl < grabbed_entry.psl) {
            // Take from the rich, and give to the poor.
            // TODO: Redudant copy here that we can get rid of
            buffer_entry = working_entry;
            map->__translation_array[ind] = grabbed_entry;
            grabbed_entry = buffer_entry;
        }

        grabbed_entry.psl++;
        ind = (ind + 1) % map->__array_size;
        printf("bubble (psl: %lu, idx: %lu)\n", grabbed_entry.psl, ind);
    } while (grabbed_entry.has_entry == 1);
}

/**
 * Given a chmap and a key, returns the index where that key should be inserted and the PSL.
 */
static struct __psl_ind
address_array(struct chmap *map, const uint64_t key) 
{
    uint64_t working_index = key % map->__array_size;
    size_t psl = 0;

    struct __entry working_entry = map->__translation_array[working_index];

    while (working_entry.has_entry == 1 && working_entry.keyword != key && working_entry.psl >= psl) {
        working_entry = map->__translation_array[working_index];
        working_index = (working_index + 1) % map->__array_size;
        psl++;
    }

    return (struct __psl_ind){ working_index, psl};
}

static struct __entry *
init_translation_array(
    const size_t numentries
) {
    struct __entry * entries = calloc(numentries, sizeof(struct __entry));
    struct __entry empty = {
        .has_entry = 0,
    };

    for (size_t i = 0; i < numentries; i++) {
        entries[i] = empty;
    }

    return entries;
}

static void
grow_map(struct chmap * map) 
{
    debug_map_params(map);
    printf("growing\n");
    size_t new_size = map->__array_size * ARRAY_GROW_FACTOR;
    size_t old_size = map->__array_size;

    void * old_backing_array = map->__backing_array;
    struct __entry * old_translation_array = map->__translation_array;

    void * new_backing_array = malloc(map->__item_size * new_size);
    struct __entry * new_translation_array = init_translation_array(new_size);
    
    debug_map(map);
    // Instead of writing some jank code, we'll just reuse the put item operation.
    // This requires us to act like there's no items in the array.
    map->__backing_array = new_backing_array;
    map->__translation_array = new_translation_array;
    map->__array_size = new_size;
    // This is so we can reset backing array indices.
    map->__used_size = 0;

    debug_map_params(map);
    for (size_t i = 0; i < old_size; i++) {
        struct __entry entry = old_translation_array[i];
        if (entry.has_entry) {
            void * ba_ptr = ((size_t*)old_backing_array) + (entry.backing_array_key * map->__item_size);
            printf("\n");
            printf("rehashing entry at index %lu (val %c); bak: %lu\n", i, *(char*)ba_ptr, entry.backing_array_key);
            chmap_put_hash(map, entry.keyword, ba_ptr);
        }
    }

    debug_map(map);

    // free(old_backing_array);
    // free(old_translation_array);
}

struct chmap *
chmap_new(
    const size_t item_size
) {
    void * backing_array = malloc(sizeof(item_size) * DEFAULT_BACKING_ARRAY_LENGTH);
    struct chmap * map = malloc(sizeof(struct chmap));

    map->__item_size = item_size;
    map->__used_size = 0;
    map->__array_size = DEFAULT_BACKING_ARRAY_LENGTH;
    map->__translation_array = init_translation_array(DEFAULT_BACKING_ARRAY_LENGTH);
    map->__backing_array = backing_array;

    return map;
}

static int
chmap_put_hash(
    struct chmap * map,
    const uint64_t hash,
    const void * item
) {
    struct __psl_ind insert_at = address_array(map, hash);
    const struct __entry looking_at = map->__translation_array[insert_at.index];
    const size_t itemsize = map->__item_size;

    if (looking_at.has_entry == 0) {
        // We found an empty spot - put it in, no fuss
        size_t bak = map->__used_size;
        printf("(empty) bak: %lu;\n", bak);
        map->__used_size = (map->__used_size + 1);
        struct __entry new_entry = {
            1,
            insert_at.psl,
            .backing_array_key = bak,
            .keyword = hash,
        };

        
        map->__translation_array[insert_at.index] = new_entry;

        void * ba_ptr = (size_t*)map->__backing_array + bak * itemsize;

        memcpy(ba_ptr, item, itemsize);
    } else if (looking_at.keyword == hash) {
        // This key already is associated - overwrite it
        void * ba_ptr = (size_t*)map->__backing_array + looking_at.backing_array_key * itemsize;

        memcpy(ba_ptr, item, itemsize);

        printf("put (overwrite) \n");
        return 1;
    } else {
        // We need to now swap the two out, and put the next one somewhere else down in the array.
        size_t bak = map->__used_size;
        printf("(bubbling) bak: %lu;\n", bak);
        map->__used_size = (map->__used_size + 1);
        struct __entry new_entry = {
            1,
            insert_at.psl,
            .backing_array_key = bak,
            .keyword = hash,
        };

        void * ba_ptr = (size_t*)map->__backing_array + bak * itemsize;

        memcpy(ba_ptr, item, itemsize);

        bubble_up(map, new_entry, insert_at.index);
    }

    printf("put (not overwrite) \n");
    return 0;
}

int
chmap_put(
    struct chmap * map,
    const void * key,
    const size_t keysize,
    const void * item
) {
    // This is used in place of a uint8_t[8] to provide the same 8 bytes
    // but in a format easier to use as a key.
    uint64_t outword;

    printf("putting\n");
    if (map->__used_size >= map->__array_size * MAX_LOAD_FACTOR) {
        printf("hnnnrgh i'm gonna grow\n");
        grow_map(map);
    }

    siphash(key, keysize, SIPHASH_KEY, (uint8_t*)&outword, 8);

    return chmap_put_hash(map, outword, item);
}

const void *
chmap_get(
    struct chmap * map,
    const void * key,
    const size_t keysize
) {
    uint64_t outword;

    siphash(key, keysize, SIPHASH_KEY, (uint8_t*)&outword, 8);

    size_t working_index = outword % map->__array_size;

    struct __entry maybe = map->__translation_array[working_index];
    while (maybe.has_entry && maybe.keyword != outword) {
        working_index = (working_index + 1) % map->__array_size;
        maybe = map->__translation_array[working_index];
    };

    if (maybe.has_entry) {
        return ((size_t*)map->__backing_array + maybe.backing_array_key * map->__item_size);
    } else {
        return NULL;
    }
}


const void * chmap_del(struct chmap * map, const void * key, const size_t keysize) {
    return NULL;
}

void
debug_map(
    struct chmap * map
) {
    for (size_t i = 0; i < map->__array_size; i++) {
        struct __entry entry = map->__translation_array[i];

        if (entry.has_entry)
        printf("bak %3lu; psl: %3lu; tind: %3lu; val: %lu;\n",
            entry.backing_array_key, 
            entry.psl,
            i,
            *((size_t*)map->__backing_array + entry.backing_array_key * map->__item_size)
        );
    }
}

void
debug_map_params(
    struct chmap * map
) {
    printf("item_size: %lu\nused_size: %lu\narray_size: %lu\ntarray_addr: %lu\nbarray_addr: %lu\n",
        map->__item_size,
        map->__used_size,
        map->__array_size,
        map->__translation_array,
        map->__backing_array
    );
}
