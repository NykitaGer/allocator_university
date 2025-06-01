#include <stdlib.h>
#include <string.h>
#include "kernel.h"

void* kernel_alloc(size_t size) {
    return malloc(size);
}

void kernel_free(void* ptr, size_t size) {
    (void)size;
    free(ptr);
}

void kernel_madvise(void* addr, size_t length) {
    // Simulate madvise by filling with random data
    for (size_t i = 0; i < length; i++) {
        ((char*)addr)[i] = rand() % 256;
    }
}