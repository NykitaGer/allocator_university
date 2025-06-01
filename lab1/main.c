#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "allocator.h"
#include "config.h"

static void fill_random(void* ptr, size_t size) {
    if (!ptr) {
        printf("fill_random: NULL pointer\n");
        return;
    }
    unsigned char* data = ptr;
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() % 256;
    }
    printf("fill_random: Filled %zu bytes at %p\n", size, ptr);
}

int main() {
    printf("Starting Manual Tests:\n");
    void* p1 = mem_alloc(100);
    if (p1) fill_random(p1, 100);
    else printf("Manual Test: Failed to allocate p1\n");
    mem_show();

    void* p2 = mem_alloc(200);
    if (p2) fill_random(p2, 200);
    else printf("Manual Test: Failed to allocate p2\n");
    mem_show();

    printf("Manual Test: Freeing p1\n");
    mem_free(p1);
    mem_show();

    printf("Manual Test: Reallocating p2 to 300 bytes\n");
    void* p3 = mem_realloc(p2, 300);
    if (p3) fill_random(p3, 300);
    else printf("Manual Test: Failed to reallocate p2\n");
    mem_show();

    printf("Manual Test: Freeing p3\n");
    mem_free(p3);
    mem_show();

    printf("Manual Test: Allocating SIZE_MAX\n");
    void* p4 = mem_alloc(SIZE_MAX);
    if (!p4) printf("Manual Test: SIZE_MAX allocation failed as expected\n");
    else printf("Manual Test: Unexpected success for SIZE_MAX allocation\n");
    mem_show();

    printf("\nStarting Automatic Tester:\n");
    void* ptrs[100] = { 0 };
    size_t sizes[100] = { 0 };
    int ptr_count = 0;
    for (int i = 0; i < 10000; i++) {
        int op = rand() % 3;
        if (op == 0 && ptr_count < 100) {
            size_t size = (size_t)rand() % (MAX_BLOCK_SIZE * 2) + 1;
            printf("Automatic Test: Allocating %zu bytes (iteration %d)\n", size, i);
            void* ptr = mem_alloc(size);
            if (ptr) {
                fill_random(ptr, size);
                ptrs[ptr_count] = ptr;
                sizes[ptr_count] = size;
                ptr_count++;
            }
        }
        else if (op == 1 && ptr_count > 0) {
            int idx = rand() % ptr_count;
            printf("Automatic Test: Freeing ptrs[%d] at %p (iteration %d)\n", idx, ptrs[idx], i);
            mem_free(ptrs[idx]);
            ptrs[idx] = ptrs[ptr_count - 1];
            sizes[idx] = sizes[ptr_count - 1];
            ptr_count--;
        }
        else if (op == 2 && ptr_count > 0) {
            int idx = rand() % ptr_count;
            size_t new_size = (size_t)rand() % (MAX_BLOCK_SIZE * 2) + 1;
            printf("Automatic Test: Reallocating ptrs[%d] at %p to %zu bytes (iteration %d)\n",
                idx, ptrs[idx], new_size, i);
            void* new_ptr = mem_realloc(ptrs[idx], new_size);
            if (new_ptr) {
                ptrs[idx] = new_ptr;
                sizes[idx] = new_size;
            }
        }
    }
    printf("Automatic Test: Final state\n");
    mem_show();
    for (int i = 0; i < ptr_count; i++) {
        printf("Automatic Test: Freeing ptrs[%d] at %p\n", i, ptrs[i]);
        mem_free(ptrs[i]);
    }
    printf("Tests completed\n");
    return 0;
}