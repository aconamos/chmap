#pragma once
#include <stddef.h>
#include <stdint.h>


struct __entry {
    int has_entry;
    size_t psl;
    uint64_t keyword;
    size_t backing_array_key;
};

/**
 * The main struct for a map. Contains the backing array, data sizes, and the array length.
 */
struct chmap {
    // TODO: key_size might be pointless
    size_t __key_size;
    size_t __item_size;
    // The number of elements stored in this map. Used to compute load factor, 
    // and for keeping track of the next index in the backing array
    size_t __used_size;
    size_t __array_size;
    struct __entry *__translation_array;
    void *__backing_array;
};


/**
 * Given an item_size, creates a new hashmap that can store items of item_size.
 * 
 * @param item_size size of data that will be stored in this hashmap
 */
struct chmap* chmap_new(const size_t item_size);

/**
 * Puts an item into the given map at the given key. Returns 1 if an item was overwritten,
 * or a 0 if the slot was empty.
 */
int chmap_put(
    struct chmap *map, 
    const void *key, 
    const size_t keysize, 
    const void *item, 
    const size_t itemsize
);

/**
 * Gets a pointer to the item associated with `key`, or `NULL` if not found.
 */
void* chmap_get(struct chmap *map, const void *key, const size_t keysize);

void
debug_map(struct chmap *map);
