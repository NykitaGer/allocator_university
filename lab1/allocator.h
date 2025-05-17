#pragma once
#include <stddef.h>

void* mem_alloc(size_t size);
void mem_free(void* ptr);
void mem_show(void);