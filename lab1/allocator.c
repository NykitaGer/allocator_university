#include "allocator.h"
#include "block.h"
#include "kernel.h"
#include "config.h"
#include "tree.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct arena {
    block_t* start;
    size_t size;
    struct arena* next;
} arena_t;

static arena_t* arenas = NULL;
static block_tree_t free_tree = { NULL };

static arena_t* create_arena(size_t size) {
    arena_t* arena = kernel_alloc(sizeof(arena_t));
    if (!arena) {
        return NULL;
    }
    arena->start = kernel_alloc(size);
    if (!arena->start) {
        kernel_free(arena, sizeof(arena_t));
        return NULL;
    }
    arena->size = size;
    arena->next = arenas;
    arenas = arena;
    block_t* block = arena->start;
    block_set_size(block, size);
    block_set_prev_size(block, 0);
    block_set_offset(block, 0);
    block_set_busy(block, 0);
    block_set_first(block, 1);
    block_set_last(block, 1);
    block_node_t* node = block_to_node(block);
    if (!node) {
        kernel_free(arena->start, size);
        kernel_free(arena, sizeof(arena_t));
        return NULL;
    }
    node->key = block_get_size(block);
    node->left = node->right = node->prev = node->next = NULL;
    if (!block_tree_insert(&free_tree, node)) {
        kernel_free(arena->start, size);
        kernel_free(arena, sizeof(arena_t));
        return NULL;
    }
    return arena;
}

static void init_arena() {
    if (!arenas) {
        create_arena(DEFAULT_ARENA_SIZE);
    }
}

static void free_arena(arena_t* arena) {
    if (!arena || !arena->start) {
        return;
    }
    block_t* block = arena->start;
    if (!block_is_busy(block) && block_is_first(block) && block_is_last(block)) {
        avl_node_t* node = block_to_node(block);
        if (node) {
            block_tree_delete(&free_tree, block_get_size(block));
        }
        kernel_madvise(block, arena->size);
        kernel_free(block, arena->size);
        arena_t** curr = &arenas;
        while (*curr && *curr != arena) curr = &(*curr)->next;
        if (*curr) {
            *curr = arena->next;
        }
        kernel_free(arena, sizeof(arena_t));
    }
}

void* mem_alloc(size_t size) {
    if (size == 0 || size > SIZE_MAX - sizeof(block_t) - ALIGNMENT) {
        return NULL;
    }
    init_arena();
    size_t aligned_size = (size + sizeof(block_t) + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    arena_t* arena = size > MAX_BLOCK_SIZE ? create_arena(aligned_size) : arenas;
    if (!arena) {
        return NULL;
    }
    block_node_t* best = NULL, * current = free_tree.root;
    while (current) {
        block_t* block = node_to_block(current);
        if (!block) {
            current = current->next ? current->next : (current->key < aligned_size ? current->right : current->left);
            continue;
        }
        arena_t* a = arenas;
        int valid_arena = 0;
        while (a) {
            if (block >= a->start && block < (block_t*)((char*)a->start + a->size)) {
                valid_arena = 1;
                break;
            }
            a = a->next;
        }
        if (!valid_arena) {
            current = current->next ? current->next : (current->key < aligned_size ? current->right : current->left);
            continue;
        }
        if (current->key >= aligned_size && (!best || current->key < best->key) &&
            (size <= MAX_BLOCK_SIZE || a->size == aligned_size)) {
            best = current;
        }
        current = current->next ? current->next : (current->key < aligned_size ? current->right : current->left);
    }
    if (!best) {
        if (size > MAX_BLOCK_SIZE) return mem_alloc(size);
        return NULL;
    }
    block_t* block = node_to_block(best);
    if (!block_tree_delete(&free_tree, block_get_size(block))) {
        return NULL;
    }
    block_split(block, size);
    block_set_busy(block, 1);
    if (!block_is_busy(block)) {
        return NULL;
    }
    block_t* next_block = block_next(block);
    if (next_block && !block_is_busy(next_block)) {
        block_node_t* new_node = block_to_node(next_block);
        if (new_node) {
            new_node->key = block_get_size(next_block);
            new_node->left = new_node->right = new_node->prev = new_node->next = NULL;
            block_tree_insert(&free_tree, new_node);
        }
    }
    return block_to_payload(block);
}

void mem_free(void* ptr) {
    if (!ptr) {
        return;
    }
    block_t* block = payload_to_block(ptr);
    if (!block) {
        return;
    }
    arena_t* arena = arenas;
    while (arena && (block < arena->start || block >= (block_t*)((char*)arena->start + arena->size))) {
        arena = arena->next;
    }
    if (!arena) {
        return;
    }
    block_set_busy(block, 0);
    block_t* next = block_next(block);
    if (next && !block_is_busy(next)) {
        block_tree_delete(&free_tree, block_get_size(next));
        block_merge(block);
    }
    block_t* prev = block_prev(block);
    if (prev && !block_is_busy(prev)) {
        block_tree_delete(&free_tree, block_get_size(prev));
        block_merge(prev);
        block = prev;
    }
    block_node_t* node = block_to_node(block);
    if (!node) {
        return;
    }
    node->key = block_get_size(block);
    node->left = node->right = node->prev = node->next = NULL;
    block_tree_insert(&free_tree, node);
    free_arena(arena);
}

void* mem_realloc(void* ptr, size_t size) {
    if (!ptr) return mem_alloc(size);
    if (size == 0) {
        mem_free(ptr);
        return NULL;
    }
    block_t* block = payload_to_block(ptr);
    if (!block) {
        return NULL;
    }
    size_t aligned_size = (size + sizeof(block_t) + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t curr_size = block_get_size(block);
    if (aligned_size <= curr_size && (curr_size - aligned_size) < MIN_BLOCK_SIZE) {
        return ptr;
    }
    if (aligned_size <= curr_size) {
        block_split(block, size);
        block_t* next_block = block_next(block);
        if (next_block && !block_is_busy(next_block)) {
            block_node_t* new_node = block_to_node(next_block);
            if (new_node) {
                new_node->key = block_get_size(next_block);
                new_node->left = new_node->right = new_node->prev = new_node->next = NULL;
                block_tree_insert(&free_tree, new_node);
            }
        }
        return ptr;
    }
    block_t* next = block_next(block);
    if (next && !block_is_busy(next) && curr_size + block_get_size(next) >= aligned_size) {
        block_tree_delete(&free_tree, block_get_size(next));
        block_merge(block);
        block_split(block, size);
        block_t* next_block = block_next(block);
        if (next_block && !block_is_busy(next_block)) {
            block_node_t* new_node = block_to_node(next_block);
            if (new_node) {
                new_node->key = block_get_size(next_block);
                new_node->left = new_node->right = new_node->prev = new_node->next = NULL;
                block_tree_insert(&free_tree, new_node);
            }
        }
        return ptr;
    }
    void* new_ptr = mem_alloc(size);
    if (!new_ptr) {
        return NULL;
    }
    size_t copy_size = curr_size < aligned_size ? curr_size - sizeof(block_t) : size;
    memcpy(new_ptr, ptr, copy_size);
    mem_free(ptr);
    return new_ptr;
}

static void print_block(avl_node_t* node) {
    if (!node) {
        return;
    }
    block_t* block = node_to_block(node);
    if (!block) {
        return;
    }
    printf("Block at %p: size=%zu, prev_size=%zu, offset=%zu, busy=%d, first=%d, last=%d\n",
        block, block_get_size(block), block_get_prev_size(block),
        block_get_offset(block), block_is_busy(block),
        block_is_first(block), block_is_last(block));
}

void mem_show(void) {
    block_tree_walk(&free_tree, print_block);
}