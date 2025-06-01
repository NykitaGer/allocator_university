#pragma once
#include <stddef.h>

typedef struct avl_node {
    size_t key;
    struct avl_node* left, * right, * prev, * next;
    int height;
} avl_node_t;

typedef struct avl_tree {
    avl_node_t* root;
} avl_tree_t;

avl_node_t* avl_insert(avl_tree_t* tree, avl_node_t* node);
avl_node_t* avl_delete(avl_tree_t* tree, size_t key);
void avl_walk(avl_tree_t* tree, void (*func)(avl_node_t*));