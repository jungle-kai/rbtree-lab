#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>

/* ---------- START TREE ---------------------------------------- */

rbtree *new_rbtree(void) {
    rbtree *newTree = (rbtree *)calloc(1, sizeof(rbtree));

    newTree->nil = (node_t *)malloc(sizeof(node_t));                                // Define Sentinel
    newTree->nil->color = RBTREE_BLACK;                                             // Nil nodes always black
    newTree->nil->left = newTree->nil->right = newTree->nil->parent = newTree->nil; // Start by pointing to self
    newTree->nil->key = 0;                                                          // Just in case (shouldn't happen
    newTree->root = newTree->nil;                                                   // Initiate with root pointing at nil

    return newTree;
}

/* ---------- DELETE TREE ---------------------------------------- */
// free_subtree as helper to the main delete function

void free_subtree(rbtree *t, node_t *node) {
    if (node == t->nil) { // No need to clear sentinel at this stage
        return;
    }
    free_subtree(t, node->left);
    free_subtree(t, node->right);
    free(node);
}

void delete_rbtree(rbtree *t) {
    free_subtree(t, t->root); // Recursion to free nodes
    free(t->nil);             // Clear sentinel at this point
    free(t);                  // Release tree at this point
}

/* ---------- INSERT ---------------------------------------- */
// left_rotate, right_rotate, rbtree_insert_fixup as helpers to main insert function

void left_rotate(rbtree *t, node_t *x) {
    node_t *y = x->right;
    x->right = y->left;
    if (y->left != t->nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == t->nil) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void right_rotate(rbtree *t, node_t *y) {
    node_t *x = y->left;
    y->left = x->right;
    if (x->right != t->nil) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == t->nil) {
        t->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}

void rbtree_insert_fixup(rbtree *t, node_t *z) {
    // While the parent of z is red, we have a violation
    while (z->parent->color == RBTREE_RED) {
        if (z->parent == z->parent->parent->left) {
            node_t *y = z->parent->parent->right; // Uncle node

            if (y->color == RBTREE_RED) {
                z->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(t, z);
                }
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                right_rotate(t, z->parent->parent);
            }
        } else {                                 // Mirror the above case
            node_t *y = z->parent->parent->left; // Uncle node

            if (y->color == RBTREE_RED) {
                z->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(t, z);
                }
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                left_rotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = RBTREE_BLACK; // Ensure root is black
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
    node_t *z = (node_t *)malloc(sizeof(node_t));
    z->key = key;
    z->left = z->right = t->nil;
    z->color = RBTREE_RED; // New nodes are always red initially

    node_t *y = t->nil; // Trailing pointer, will end up being the parent of `z`
    node_t *x = t->root;

    // Standard binary search tree insertion
    while (x != t->nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;

    if (y == t->nil) { // Tree was empty
        t->root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    // Now fix the Red-Black properties
    rbtree_insert_fixup(t, z);

    return z;
}

/* ---------- SEARCH ---------------------------------------- */
// _rbtree_min_from_node to assist with rbtree_erase (node)

node_t *rbtree_find(const rbtree *t, const key_t key) {
    node_t *x = t->root;

    // Traverse the tree
    while (x != t->nil && x->key != key) {
        if (key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // If x == t->nil, the key isn't in the tree, so return NULL.
    if (x == t->nil) {
        return NULL;
    }
    return x;
}

node_t *_rbtree_min_from_node(const rbtree *t, node_t *x) {
    while (x->left != t->nil) {
        x = x->left;
    }
    return x;
}

node_t *rbtree_min(const rbtree *t) { return _rbtree_min_from_node(t, t->root); }

node_t *rbtree_max(const rbtree *t) {
    node_t *x = t->root;

    // Traverse to the rightmost node
    while (x->right != t->nil) {
        x = x->right;
    }

    return x; // This will return nil if the tree is empty
}

/* ---------- DELETE NODE ---------------------------------------- */
// rbtree_transplant and delete_fixup as helpers to the main erase function

void rbtree_transplant(rbtree *t, node_t *u, node_t *v) {
    if (u->parent == t->nil) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }

    v->parent = u->parent;
}

void rbtree_delete_fixup(rbtree *t, node_t *x) {
    while (x != t->root && x->color == RBTREE_BLACK) {
        if (x == x->parent->left) {
            node_t *w = x->parent->right;

            if (w->color == RBTREE_RED) {
                w->color = RBTREE_BLACK;
                x->parent->color = RBTREE_RED;
                left_rotate(t, x->parent);
                w = x->parent->right;
            }

            if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK) {
                w->color = RBTREE_RED;
                x = x->parent;
            } else {
                if (w->right->color == RBTREE_BLACK) {
                    w->left->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    right_rotate(t, w);
                    w = x->parent->right;
                }

                w->color = x->parent->color;
                x->parent->color = RBTREE_BLACK;
                w->right->color = RBTREE_BLACK;
                left_rotate(t, x->parent);
                x = t->root;
            }
        } else {
            // Mirror of the above code with "right" and "left" swapped.
            node_t *w = x->parent->left;

            if (w->color == RBTREE_RED) {
                w->color = RBTREE_BLACK;
                x->parent->color = RBTREE_RED;
                right_rotate(t, x->parent);
                w = x->parent->left;
            }

            if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK) {
                w->color = RBTREE_RED;
                x = x->parent;
            } else {
                if (w->left->color == RBTREE_BLACK) {
                    w->right->color = RBTREE_BLACK;
                    w->color = RBTREE_RED;
                    left_rotate(t, w);
                    w = x->parent->left;
                }

                w->color = x->parent->color;
                x->parent->color = RBTREE_BLACK;
                w->left->color = RBTREE_BLACK;
                right_rotate(t, x->parent);
                x = t->root;
            }
        }
    }

    x->color = RBTREE_BLACK;
}

int rbtree_erase(rbtree *t, node_t *z) {
    if (!z || z == t->nil)
        return 0;

    node_t *y = z, *x;
    color_t original_color = y->color;

    if (z->left == t->nil) {
        x = z->right;
        rbtree_transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        rbtree_transplant(t, z, z->left);
    } else {
        y = _rbtree_min_from_node(t, z->right);
        original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            rbtree_transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        rbtree_transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (original_color == RBTREE_BLACK) {
        rbtree_delete_fixup(t, x);
    }

    free(z);
    return 1;
}

/* ---------- INTO ARRAY ----------------------------------------*/
// inorder_traversal as static (helper to rbtree_to_array)

static size_t inorder_traversal(const rbtree *t, node_t *node, key_t *arr, size_t index, const size_t n) {
    if (node == t->nil) {
        return index;
    }

    if (node->left != t->nil) {
        index = inorder_traversal(t, node->left, arr, index, n);
    }

    if (index < n) {
        arr[index++] = node->key;
    }

    if (node->right != t->nil) {
        index = inorder_traversal(t, node->right, arr, index, n);
    }

    return index;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) { return 0; }
