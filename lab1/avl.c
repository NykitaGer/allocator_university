#include "avl.h"
#include <stdio.h>

static int height(avl_node_t* node) {
    return node ? node->height : 0;
}

static int balance_factor(avl_node_t* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

static void update_height(avl_node_t* node) {
    if (!node) return;
    int hl = height(node->left), hr = height(node->right);
    node->height = (hl > hr ? hl : hr) + 1;
}

static avl_node_t* rotate_right(avl_node_t* y) {
    if (!y || !y->left) return y;
    avl_node_t* x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

static avl_node_t* rotate_left(avl_node_t* x) {
    if (!x || !x->right) return x;
    avl_node_t* y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return x;
}

static avl_node_t* balance(avl_node_t* node) {
    if (!node) return NULL;
    update_height(node);
    int bf = balance_factor(node);
    if (bf > 1) {
        if (balance_factor(node->left) < 0) node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (bf < -1) {
        if (balance_factor(node->right) > 0) node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

avl_node_t* avl_insert(avl_tree_t* tree, avl_node_t* node) {
    if (!tree || !node || !node->key) {
        return NULL;
    }
    if (node->left || node->right || node->prev || node->next) {
        node->left = node->right = node->prev = node->next = NULL;
    }
    node->height = 1;
    if (!tree->root) {
        tree->root = node;
        return node;
    }
    avl_node_t* current = tree->root, * parent = NULL;
    int depth = 0;
    while (current) {
        parent = current;
        if (node->key < current->key) {
            current = current->left;
        }
        else if (node->key > current->key) {
            current = current->right;
        }
        else {
            node->prev = current;
            node->next = current->next;
            if (current->next) current->next->prev = node;
            current->next = node;
            return node;
        }
        depth++;
        if (depth > 1000) {
            return NULL;
        }
    }
    if (node->key < parent->key) {
        parent->left = node;
    }
    else {
        parent->right = node;
    }
    avl_node_t* p = parent;
    depth = 0;
    while (p) {
        update_height(p);
        avl_node_t* new_p = balance(p);
        if (p == tree->root) {
            tree->root = new_p;
        }
        else if (parent && parent->left == p) {
            parent->left = new_p;
        }
        else if (parent && parent->right == p) {
            parent->right = new_p;
        }
        parent = p;
        p = new_p;
        p = p->left && p->left != parent ? p->left : (p->right && p->right != parent ? p->right : NULL);
        depth++;
        if (depth > 1000) {
            return NULL;
        }
    }
    return node;
}

avl_node_t* avl_delete(avl_tree_t* tree, size_t key) {
    if (!tree || !tree->root) {
        return NULL;
    }
    avl_node_t* current = tree->root, * parent = NULL;
    int depth = 0;
    while (current && current->key != key) {
        parent = current;
        current = key < current->key ? current->left : current->right;
        depth++;
        if (depth > 1000) {
            return NULL;
        }
    }
    if (!current) {
        return NULL;
    }
    if (current->next) {
        avl_node_t* next = current->next;
        next->left = current->left;
        next->right = current->right;
        next->height = current->height;
        if (!parent) {
            tree->root = next;
        }
        else if (parent->left == current) {
            parent->left = next;
        }
        else {
            parent->right = next;
        }
        return current;
    }
    avl_node_t* replacement = current->left ? current->left : current->right;
    if (!parent) {
        tree->root = replacement;
    }
    else if (parent->left == current) {
        parent->left = replacement;
    }
    else {
        parent->right = replacement;
    }
    avl_node_t* p = parent;
    depth = 0;
    while (p) {
        update_height(p);
        avl_node_t* new_p = balance(p);
        if (p == tree->root) {
            tree->root = new_p;
        }
        else if (parent && parent->left == p) {
            parent->left = new_p;
        }
        else if (parent && parent->right == p) {
            parent->right = new_p;
        }
        parent = p;
        p = new_p;
        p = p->left && p->left != parent ? p->left : (p->right && p->right != parent ? p->right : NULL);
        depth++;
        if (depth > 1000) {
            return NULL;
        }
    }
    return current;
}

void avl_walk(avl_tree_t* tree, void (*func)(avl_node_t*)) {
    if (!tree || !func) {
        return;
    }
    if (!tree->root) {
        return;
    }
    avl_node_t* stack[64], * current = tree->root;
    int top = -1, depth = 0;
    stack[++top] = current;
    while (top >= 0) {
        current = stack[top--];
        if (!current || !current->key || current->height <= 0) {
            continue;
        }
        for (avl_node_t* node = current; node; node = node->next) {
            if (!node->key) {
                continue;
            }
            func(node);
        }
        if (current->right) stack[++top] = current->right;
        if (current->left) stack[++top] = current->left;
        depth++;
        if (depth > 1000) {
            return;
        }
    }
}