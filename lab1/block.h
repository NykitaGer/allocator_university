#pragma once
#include <stddef.h>
#include "allocator_impl.h"
#include "tree.h"

#define FLAG_BUSY 0x1
#define FLAG_FIRST 0x2
#define FLAG_LAST 0x4

typedef struct block {
    size_t size;        // Size with flags in lower bits
    size_t prev_size;   // Previous size with flags
    size_t offset;      // Offset from arena start
} block_t;

static inline void* block_to_payload(block_t* block) {
    return block ? (char*)block + sizeof(block_t) : NULL;
}

static inline block_t* payload_to_block(void* payload) {
    return payload ? (block_t*)((char*)payload - sizeof(block_t)) : NULL;
}

static inline block_node_t* block_to_node(block_t* block) {
    return block ? (block_node_t*)block_to_payload(block) : NULL;
}

static inline block_t* node_to_block(block_node_t* node) {
    return node ? payload_to_block(node) : NULL;
}

static inline block_t* block_next(block_t* block) {
    if (!block || (block->size & FLAG_LAST)) return NULL;
    return (block_t*)((char*)block + (block->size & ~(ALIGNMENT - 1)));
}

static inline block_t* block_prev(block_t* block) {
    if (!block || (block->size & FLAG_FIRST)) return NULL;
    return (block_t*)((char*)block - (block->prev_size & ~(ALIGNMENT - 1)));
}

static inline size_t block_get_size(block_t* block) { return block ? (block->size & ~(ALIGNMENT - 1)) : 0; }
static inline size_t block_get_prev_size(block_t* block) { return block ? (block->prev_size & ~(ALIGNMENT - 1)) : 0; }
static inline size_t block_get_offset(block_t* block) { return block ? block->offset : 0; }
static inline int block_is_busy(block_t* block) { return block ? (block->size & FLAG_BUSY) : 0; }
static inline int block_is_first(block_t* block) { return block ? (block->size & FLAG_FIRST) : 0; }
static inline int block_is_last(block_t* block) { return block ? (block->size & FLAG_LAST) : 0; }

static inline void block_set_size(block_t* block, size_t size) {
    if (block) block->size = (size & ~(ALIGNMENT - 1)) | (block->size & (ALIGNMENT - 1));
}
static inline void block_set_prev_size(block_t* block, size_t prev_size) {
    if (block) block->prev_size = (prev_size & ~(ALIGNMENT - 1)) | (block->prev_size & (ALIGNMENT - 1));
}
static inline void block_set_offset(block_t* block, size_t offset) { if (block) block->offset = offset; }
static inline void block_set_busy(block_t* block, int busy) {
    if (block) block->size = busy ? (block->size | FLAG_BUSY) : (block->size & ~FLAG_BUSY);
}
static inline void block_set_first(block_t* block, int first) {
    if (block) block->size = first ? (block->size | FLAG_FIRST) : (block->size & ~FLAG_FIRST);
}
static inline void block_set_last(block_t* block, int last) {
    if (block) block->size = last ? (block->size | FLAG_LAST) : (block->size & ~FLAG_LAST);
}

void block_split(block_t* block, size_t size);
void block_merge(block_t* block);