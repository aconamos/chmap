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

static size_t * 
init_bais_stack(
    size_t numentries
);

static inline void *
get_ba_ptr(
    struct chmap * map,
    size_t index
);

static inline void * 
get_ba_ptr_arr(
    void * ba,
    size_t isize,
    size_t index
);

/**
 * Takes an entry and a location and tries to insert it at the location, performing
 * robinhood shuffling if necessary to maintain low PSL or whatever.
 */
static void
bubble_up(struct chmap * map, const struct __entry inentry, size_t ind) {
    struct __entry grabbed_entry = inentry;

    do {
        struct __entry working_entry = map->__translation_array[ind];

        if (working_entry.has_entry == 0) {
            // We've encountered an empty spot, and can insert and jump ship.
            map->__translation_array[ind] = grabbed_entry;
            return;
        }

        if (working_entry.psl < grabbed_entry.psl) {
            // Take from the rich, and give to the poor - this means,
            // if an entry has a lower PSL than our current one, swap.
            map->__translation_array[ind] = grabbed_entry;
            grabbed_entry = working_entry;
        }

        grabbed_entry.psl++;
        ind = (ind + 1) % map->__array_size;
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
    size_t new_size = map->__array_size * ARRAY_GROW_FACTOR;
    size_t old_size = map->__array_size;

    void * old_backing_array = map->__backing_array;
    size_t * old_bais = map->__backing_array_index_stack;
    struct __entry * old_translation_array = map->__translation_array;

    void * new_backing_array = malloc(map->__item_size * new_size);
    size_t * new_bais = init_bais_stack(new_size);
    struct __entry * new_translation_array = init_translation_array(new_size);
    
    // Instead of writing some jank code, we'll just reuse the put item operation.
    // This requires us to act like there's no items in the array.
    map->__backing_array = new_backing_array;
    map->__translation_array = new_translation_array;
    map->__array_size = new_size;
    // This is so we can reset backing array indices.
    map->__used_size = 0;
    map->__bais_idx = new_size - 1;
    map->__backing_array_index_stack = new_bais;

    debug_map_params(map);
    for (size_t i = 0; i < old_size; i++) {
        struct __entry entry = old_translation_array[i];
        if (entry.has_entry) {
            void * ba_ptr = get_ba_ptr_arr(old_backing_array, map->__item_size, entry.backing_array_key);
            chmap_put_hash(map, entry.keyword, ba_ptr);
        }
    }

    free(old_backing_array);
    free(old_translation_array);
    free(old_bais);
}

static size_t * 
init_bais_stack(
    size_t numentries
) {
    size_t * stack = calloc(numentries, sizeof(size_t));

    for (size_t i = 0; i < numentries; i++) {
        stack[i] = numentries - i;
    }

    return stack;
}

static size_t
pop_bais_idx(
    struct chmap * map
) {
    size_t bais_idx = map->__bais_idx;

    #ifdef DEBUG
    assert(bais_idx >= 0);
    #endif

    size_t ret = map->__backing_array_index_stack[bais_idx];
    map->__bais_idx--;
    return ret;
}

static void
push_bais_idx(
    struct chmap * map,
    size_t val
) {
    map->__bais_idx++;

    map->__backing_array_index_stack[map->__bais_idx] = val;
}

static inline void * 
get_ba_ptr_arr(
    void * ba,
    size_t isize,
    size_t index
) {
    return ((char*)ba) + index * isize; 
}

static inline void *
get_ba_ptr(
    struct chmap * map,
    size_t index
) {
    return ((char*)map->__backing_array) + index * map->__item_size;
}

struct chmap *
chmap_new(
    const size_t item_size,
    const size_t key_size
) {
    void * backing_array = calloc(DEFAULT_BACKING_ARRAY_LENGTH, sizeof(item_size));
    struct chmap * map = malloc(sizeof(struct chmap));

    map->__bais_idx = DEFAULT_BACKING_ARRAY_LENGTH - 1;
    map->__key_size = key_size;
    map->__item_size = item_size;
    map->__used_size = 0;
    map->__array_size = DEFAULT_BACKING_ARRAY_LENGTH;
    map->__translation_array = init_translation_array(DEFAULT_BACKING_ARRAY_LENGTH);
    map->__backing_array_index_stack = init_bais_stack(DEFAULT_BACKING_ARRAY_LENGTH);
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
        size_t bak = pop_bais_idx(map);
        map->__used_size = (map->__used_size + 1);
        struct __entry new_entry = {
            1,
            insert_at.psl,
            .backing_array_key = bak,
            .keyword = hash,
        };

        
        map->__translation_array[insert_at.index] = new_entry;

        void * ba_ptr = get_ba_ptr(map, bak);

        memcpy(ba_ptr, item, itemsize);
    } else if (looking_at.keyword == hash) {
        // This key already is associated - overwrite it
        void * ba_ptr = get_ba_ptr(map, looking_at.backing_array_key);

        memcpy(ba_ptr, item, itemsize);

        return 1;
    } else {
        // We need to now swap the two out, and put the next one somewhere else down in the array.
        size_t bak = pop_bais_idx(map);
        map->__used_size = (map->__used_size + 1);
        struct __entry new_entry = {
            1,
            insert_at.psl,
            .backing_array_key = bak,
            .keyword = hash,
        };

        void * ba_ptr = get_ba_ptr(map, bak);

        memcpy(ba_ptr, item, itemsize);

        bubble_up(map, new_entry, insert_at.index);
    }

    return 0;
}

int
chmap_put(
    struct chmap * map,
    const void * key,
    const void * item
) {
    // This is used in place of a uint8_t[8] to provide the same 8 bytes
    // but in a format easier to use as a key.
    uint64_t outword;

    if (map->__used_size >= map->__array_size * MAX_LOAD_FACTOR) {
        grow_map(map);
    }

    siphash(key, map->__key_size, SIPHASH_KEY, (uint8_t*)&outword, 8);

    return chmap_put_hash(map, outword, item);
}

const void *
chmap_get(
    struct chmap * map,
    const void * key
) {
    uint64_t outword;

    siphash(key, map->__key_size, SIPHASH_KEY, (uint8_t*)&outword, 8);

    size_t working_index = outword % map->__array_size;

    struct __entry maybe = map->__translation_array[working_index];
    while (maybe.has_entry && maybe.keyword != outword) {
        working_index = (working_index + 1) % map->__array_size;
        maybe = map->__translation_array[working_index];
    };

    if (maybe.has_entry) {
        return (get_ba_ptr(map, maybe.backing_array_key));
    } else {
        return NULL;
    }
}


const void * chmap_del(struct chmap * map, const void * key) {
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
    printf("item_size: %lu\nused_size: %lu\narray_size: %lu\ntarray_addr: %p\nbarray_addr: %p\n",
        map->__item_size,
        map->__used_size,
        map->__array_size,
        map->__translation_array,
        map->__backing_array
    );
}
