#pragma once

#define ALIGNMENT 8
#define ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define MIN_BLOCK_SIZE (sizeof(block_t) + sizeof(block_node_t))