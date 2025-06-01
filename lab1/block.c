#include "block.h"
#include <stdio.h>

void block_split(block_t* block, size_t size) {
    if (!block) return;
    size_t aligned_size = (size + sizeof(block_t) + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t curr_size = block_get_size(block);
    if (curr_size < aligned_size + sizeof(block_t) + ALIGNMENT) {
        printf("block_split: Block too small to split (size=%zu, requested=%zu)\n", curr_size, aligned_size);
        return;
    }
    block_t* new_block = (block_t*)((char*)block + aligned_size);
    block_set_size(new_block, curr_size - aligned_size);
    block_set_prev_size(new_block, aligned_size);
    block_set_offset(new_block, block_get_offset(block) + aligned_size);
    block_set_busy(new_block, 0);
    block_set_first(new_block, 0);
    block_set_last(new_block, block_is_last(block));
    block_set_size(block, aligned_size);
    block_set_busy(block, block_is_busy(block));
    block_set_last(block, 0);
    block_t* next = block_next(new_block);
    if (next) {
        block_set_prev_size(next, block_get_size(new_block));
    }
    printf("block_split: Split block at %p into %zu and %zu\n", block, aligned_size, curr_size - aligned_size);
}

void block_merge(block_t* block) {
    if (!block) return;
    block_t* next = block_next(block);
    if (!next || block_is_busy(next)) {
        printf("block_merge: Cannot merge at %p (next=%p, busy=%d, next_busy=%d)\n",
            block, next, block_is_busy(block), next ? block_is_busy(next) : 0);
        return;
    }
    size_t new_size = block_get_size(block) + block_get_size(next);
    block_set_size(block, new_size);
    block_set_last(block, block_is_last(next));
    block_t* next_next = block_next(block);
    if (next_next) {
        block_set_prev_size(next_next, new_size);
    }
    printf("block_merge: Merged block at %p, new size=%zu\n", block, new_size);
}