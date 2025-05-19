#include "block.h"
#include "allocator_impl.h"
#include <stdio.h>

void block_split(block_header* b, size_t size) {
	size_t aligned = ALIGN(size + sizeof(block_header));
	size_t block_size = get_block_size(b);

	if (block_size >= aligned + sizeof(block_header) + ALIGNMENT) {
		block_header* new_block = (block_header*)((char*)b + aligned);
		set_block_size(new_block, block_size - aligned);
		set_prev_block_size(new_block, aligned);
		set_block_flags(new_block, get_block_flags(b) & (BLOCK_LAST | BLOCK_BUSY));
		set_block_size(b, aligned);
		set_block_flags(b, get_block_flags(b) & (BLOCK_FIRST | BLOCK_BUSY));
	}
}

void block_merge(block_header* b) {
	block_header* next = block_next(b);
	if (!(next->size & BLOCK_BUSY)) {
		b->size += get_block_size(next);
		set_block_flags(b, get_block_flags(b) | get_block_flags(next));
	}
}