/* 
    avlaccess.h 

    c 1994 Walter Fontana  
 */

#ifndef	__AVLACCESS_H
#define	__AVLACCESS_H

#include "include.h"
#include "utilities.h"
#include "structs.h"
#include "avl.h"

#define DELETE	1
#define FIND	2
#define INSERT	3
#define EMPTY	4
#define KILL	5

extern void clear_node (REC * node);
extern void free_node (REC * node);
extern AVL_TREE *initialize_tree (void);
extern REC *AVL (int cmd, AVL_TREE * tree, char *v1, int *flag);
extern void traverse_AVL (AVL_TREE * tree, void (*visit_function) ());

#endif /* __AVLACCESS_H */
