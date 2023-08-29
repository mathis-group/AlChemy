/*
    avlaccess.c
    
    c 1994 Walter Fontana  
 */
  
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "avlaccess.h"

PUBLIC void clear_node (REC * node);
PUBLIC void free_node (REC * node);
PUBLIC AVL_TREE *initialize_tree (void);
PUBLIC REC *AVL (int cmd, AVL_TREE * tree, char *v1, int *flag);
PUBLIC void traverse_AVL (AVL_TREE * tree, void (*visit_function) ());

PRIVATE int my_cmp (REC * r1, REC * r2);

EXTERN void *avl_newnode (int size);
EXTERN void avl_freenode (void *node);
EXTERN AVL_TREE *avl_init (int (*cmp_function) ());
EXTERN void avl_empty (AVL_TREE * t, void (*freeNode) ());
EXTERN void *avl_insert (AVL_TREE * tree, void *node);
EXTERN void *avl_delete (AVL_TREE * tree, void *node);
EXTERN void avl_traverse (AVL_TREE * tree, int order, void (*visit) (), void *params);
EXTERN void avl_print (FILE * out, AVL_TREE * tree, void (*prnt) (), boolean graph_chars);
EXTERN void *avl_findsym (AVL_TREE * tree, void *node);
EXTERN void avl_range (AVL_TREE * tree, void *lower, void *upper, void (*visit) (), void *params);
EXTERN void *avl_delmin (AVL_TREE * tree);
EXTERN void *avl_delmax (AVL_TREE * tree);

/*-----------------------------------------------------------------*/

PRIVATE int
my_cmp (REC * r1, REC * r2)
{
  return strcmp (r1->object, r2->object);
}

/*-----------------------------------------------------------------*/

PUBLIC void
clear_node (REC * node)
{
  int i;

  node->object = NULL;
  node->type = NULL;
  node->instances = 0;
  node->sysid = -1;
}

/*-----------------------------------------------------------------*/

PUBLIC void
free_node (REC * node)
{
  if (node->object)
    free (node->object);
  if (node->type)
    free (node->type);
  avl_freenode (node);
}

/*-----------------------------------------------------------------*/

PUBLIC AVL_TREE *
initialize_tree (void)
{
  return (avl_init (my_cmp));
}

/*-----------------------------------------------------------------*/

PUBLIC REC *
AVL (int cmd, AVL_TREE * tree, char *item, int *found)
{
  REC *p, *p2;
  REC r;
  int j;

  switch (cmd)
    {
    case DELETE:
      r.object = item;
      if ((p = (REC *) avl_delete (tree, &r)) == NULL)
	nrerror ("AVL_DELETE: Node is NOT in tree");
      else
	return (p);
      break;

    case FIND:
      r.object = item;
      if ((p = (REC *) avl_findsym (tree, &r)) != NULL)
	return (p);
      else
	return (NULL);
      break;

    case INSERT:
      if (!(p = (REC *) avl_newnode (sizeof (REC))))
	nrerror ("AVL_INSERT: Out of memory");
      else
	{
	  clear_node (p);
	  p->object = item;
	  if ((p2 = (REC *) avl_insert (tree, p)) != NULL)
	    {
	      *found = 1;
	      free_node (p);	/* item is freed! */
	      return (p2);
	    }
	  else
	    {
	      *found = 0;
	      return (p);
	    }
	}
      break;

    case EMPTY:
      avl_empty (tree, free_node);
      tree->count = 0;
      return (NULL);
      break;

    case KILL:
      avl_empty (tree, free_node);
      free (tree);
      return (NULL);
      break;

    default:
      nrerror ("AVL: Invalid command...");
    }
}

/*-----------------------------------------------------------------*/

PUBLIC void
traverse_AVL (AVL_TREE * tree, void (*visit_function) ())
{
  avl_traverse (tree, 1, visit_function, NULL);
}

/*-----------------------------------------------------------------*/
