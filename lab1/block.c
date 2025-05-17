#include "block.h"
#include "allocator_impl.h"

void block_split(block_header* b, size_t size) {
	size_t aligned = ALIGN(size + sizeof(block_header));
	if (b->size >= aligned + sizeof(block_header) + ALIGNMENT) {
		block_header* new_block = (block_header*)((char*)b + aligned);
		set_block_size(new_block, b->size - aligned);
		set_prev_block_size(new_block, aligned);
		set_block_flags(new_block, 0);
		set_block_size(b, aligned);
	}
}

void block_merge(block_header* b) {
	block_header* next = block_next(b);
	if (!(next->size & BLOCK_BUSY)) {
		b->size += next->size;
	}
}