#pragma once
#include <stddef.h>

void* kernel_alloc(size_t size);
void kernel_free(void* ptr, size_t size);
void kernel_madvise(void* addr, size_t length);
