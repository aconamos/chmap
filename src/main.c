#include "chmap.h"
#include <stdio.h>


int main(void) {
    struct chmap *map = chmap_new(sizeof(char));

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char val = key + 20;
        chmap_put(map, &key, 1, &val, sizeof(char));
    }

    debug_map(map);

    for (char key = 'A'; key < 'L'; key = (char) key + 1) {
        char *got = chmap_get(map, &key, 1);
        printf("got %d for key %d \n", *got, key);
    }
}
