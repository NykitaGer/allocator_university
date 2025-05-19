#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"
#include <time.h>
#include <stdint.h>

int main() {
    srand((unsigned)time(NULL));

    for (int i = 0; i < 5; i++) {
        size_t size = (size_t)16 + (rand() % 128);
        char* block = mem_alloc(size);
        printf("Allocated memory | size: %p | %zu | %d\n", block, size, i);
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
    printf("mem_alloc(SIZE_MAX - 1): %p\n\n", mem_alloc(SIZE_MAX - 1));

    mem_show();

    // Testing
    int* number = mem_alloc(sizeof(int));
    *number = 6;

    printf("Number: %d\n", *number);

    uint64_t* new_number = mem_realloc(number, sizeof(uint64_t));

    printf("Number: %zu\n", *new_number);

    mem_free(new_number);

    return 0;
}