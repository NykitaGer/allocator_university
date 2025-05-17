#include "allocator.h"
#include "block.h"
#include "kernel.h"
#include "config.h"
#include "allocator_impl.h"
#include <stdio.h>
#include <limits.h>

static block_header* arena = NULL;

void* mem_alloc(size_t size) {
	if (size > SIZE_MAX - sizeof(block_header)) return NULL;
	size_t total_size = ALIGN(size + sizeof(block_header));
	block_header* curr = arena;

	while (curr) {
		if (!(curr->size & BLOCK_BUSY) && curr->size >= total_size) {
			block_split(curr, size);
			curr->size |= BLOCK_BUSY;
			return block_to_payload(curr);
		}
		if (curr->size & BLOCK_LAST) break;
		curr = block_next(curr);
	}

	size_t arena_size = DEFAULT_ARENA_SIZE;
	if (!arena) {
		curr = kernel_alloc(arena_size);
		curr->size = arena_size;
		curr->prev_size = 0;
		curr->size |= BLOCK_FIRST | BLOCK_LAST; // check this
		arena = curr;
		return mem_alloc(size);
	}

	return NULL;
}

void mem_free(void* ptr) {
	if (!ptr) return;
	block_header* b = payload_to_block(ptr);
	b->size &= ~BLOCK_BUSY;

	block_header* next = block_next(b);
	if (!(next->size) & BLOCK_BUSY) block_merge(b);

	block_header* prev = block_prev(b);
	if (!(prev->size) & BLOCK_BUSY) block_merge(b);
}

void mem_show(void) {
	block_header* curr = arena;
	printf("=== MEMORY BLOCKS ===\n");
	while (curr) {
		printf("Block at %p: size=%zu, prev_size=%zu, flags=0x%x\n",
			(void*)curr, curr->size & ~(ALIGNMENT - 1), curr->prev_size & ~(ALIGNMENT - 1), (uint8_t)(curr->size & 7));
		if (curr->size & BLOCK_LAST) break;
		curr = block_next(curr);
	}
}