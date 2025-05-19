#include "allocator.h"
#include "block.h"
#include "kernel.h"
#include "config.h"
#include "allocator_impl.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>

static block_header* arena = NULL;

void* mem_alloc(size_t size) {
	if (size > SIZE_MAX - sizeof(block_header)) return NULL;
	size_t total_size = ALIGN(size + sizeof(block_header));
	block_header* curr = arena;

	while (curr) {
		size_t block_size = get_block_size(curr);
		if (!(get_block_flags(curr) & BLOCK_BUSY) && block_size >= total_size) {
			block_split(curr, size);
			set_block_flags(curr, get_block_flags(curr) | BLOCK_BUSY);
			return block_to_payload(curr);
		}
		if (get_block_flags(curr) & BLOCK_LAST) break;
		curr = block_next(curr);
	}

	size_t arena_size = DEFAULT_ARENA_SIZE;
	if (!arena) {
		curr = kernel_alloc(arena_size);
		set_block_size(curr, arena_size);
		set_prev_block_size(curr, 0);
		set_block_flags(curr, BLOCK_FIRST | BLOCK_LAST);
		arena = curr;
		return mem_alloc(size);
	}

	return NULL;
}

void mem_free(void* ptr) {
	if (!ptr) return;
	block_header* b = payload_to_block(ptr);
	if (!(b->size & BLOCK_BUSY)) return;
	b->size &= ~BLOCK_BUSY;

	block_header* next = block_next(b);
	if (!(get_block_flags(next) & BLOCK_BUSY)) block_merge(b);

	block_header* prev = block_prev(b);
	if (!(get_block_flags(prev) & (BLOCK_BUSY | BLOCK_FIRST))) block_merge(prev);
}

void* mem_realloc(void* ptr, size_t size) {
	if (!ptr) return mem_alloc(size);
	if (size == 0) {
		mem_free(ptr);
		return NULL;
	}

	block_header* block = payload_to_block(ptr);
	size_t current_size = get_block_size(block);
	size_t total_size = ALIGN(size + sizeof(block_header));

	if (total_size <= current_size) {
		block_split(block, size);
		return ptr;
	}

	block_header* next = block_next(block);
	if (!(get_block_flags(next) & BLOCK_BUSY)) {
		size_t full_size = current_size + get_block_size(next);
		if (full_size >= total_size) {
			block_merge(block);
			block_split(block, size);
			return ptr;
		}
	}

	char* new_ptr = mem_alloc(size);
	if (!new_ptr) return NULL;
	memcpy(new_ptr, ptr, current_size - sizeof(block_header));
	mem_free(ptr);
	return new_ptr;
}

void mem_show(void) {
	block_header* curr = arena;
	printf("=== MEMORY BLOCKS ===\n");
	while (curr) {
		printf("Block at %p: size=%zu, prev_size=%zu, flags=0x%x\n",
			(void*)curr, get_block_size(curr), get_prev_block_size(curr), (uint8_t)get_block_flags(curr));
		if (get_block_flags(curr) & BLOCK_LAST) break;
		curr = block_next(curr);
	}
}