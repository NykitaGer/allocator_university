#pragma once
#include "avl.h"

typedef avl_tree_t block_tree_t;
typedef avl_node_t block_node_t;

#define block_tree_insert(t, n) avl_insert((t), (n))
#define block_tree_delete(t, k) avl_delete((t), (k))
#define block_tree_walk(t, f) avl_walk((t), (f))