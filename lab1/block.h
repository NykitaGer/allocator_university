#pragma once
#include <stdint.h>
#include <stddef.h>
#include "allocator_impl.h"

#define BLOCK_BUSY	0x1
#define BLOCK_FIRST 0x2
#define BLOCK_LAST	0x4

typedef struct block_header
{
	size_t size;
	size_t prev_size;

	//uint8_t flags;
} block_header, *block_header_ptr;

void block_split(block_header* b, size_t size);
void block_merge(block_header* b);

static inline void* block_to_payload(block_header* b) {
	return (void*)(b + 1);
}

static inline block_header* payload_to_block(void* payload) {
	return ((block_header*)payload) - 1;
}

static inline block_header* block_next(block_header* b) {
	return (block_header*)((char*)b + (uint8_t)(b->size & ~(ALIGNMENT - 1)));
}

static inline block_header* block_prev(block_header* b) {
	return (block_header*)((char*)b - (uint8_t)(b->prev_size & ~(ALIGNMENT - 1)));
}

static inline size_t get_block_size(block_header* b) {
	return b->size & ~(ALIGNMENT - 1);
}

static inline void set_block_size(block_header* b, size_t size) {
	b->size = size | (b->size & (ALIGNMENT - 1));
}

static inline size_t get_prev_block_size(block_header* b) {
	return b->prev_size & ~(ALIGNMENT - 1);
}

static inline void set_prev_block_size(block_header* b, size_t size) {
	b->prev_size = size;
}

static inline uint8_t get_block_flags(block_header* b) {
	return b->size & (ALIGNMENT - 1);
}

static inline void set_block_flags(block_header* b, uint8_t flags) {
	b->size = b->size & ~(ALIGNMENT - 1) | flags;
}