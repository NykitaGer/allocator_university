#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"
#include <time.h>

int main() {
    srand((unsigned)time(NULL));

    for (int i = 0; i < 5; i++) {
        size_t size = (size_t)16 + (rand() % 128);
        char* block = mem_alloc(size);
        if (block) {
            for (size_t j = 0; j < size; j++) {
                block[j] = rand() % 256;
            }
        }
    }

    mem_show();

    // Overflow check
    printf("\n== Overflow checks ==\n");
    printf("mem_alloc(SIZE_MAX): %p\n", mem_alloc(SIZE_MAX));
    printf("mem_alloc(SIZE_MAX - 1): %p\n", mem_alloc(SIZE_MAX - 1));

    mem_show();

    return 0;
}