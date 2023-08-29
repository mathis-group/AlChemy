/*
 *  avl.h
 */

#ifndef	__AVL_H
#define	__AVL_H

#include "include.h"

/* The three traversal orders supported */

#define	PREORDER	0
#define	INORDER		1
#define	POSTORDER	2

typedef struct AVL_BUCKET
  {
    struct AVL_BUCKET *left;	/* Pointer to left subtree  */
    struct AVL_BUCKET *right;	/* Pointer to right subtree */
    short bal;			/* Balance factor */
  }
AVL_BUCKET;

typedef struct
  {
    int count;			/* Number of elements currently in tree */
    int (*cmp) ();		/* Compare two nodes */
    AVL_BUCKET *root;		/* Pointer to root node of AVL tree */
  }
AVL_TREE;

#endif /* __AVL_H */
