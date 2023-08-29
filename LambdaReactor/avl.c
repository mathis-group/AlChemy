/****************************************************************************
*
* Copyright (C) 1991 Kendall Bennett.
*
* Description:	Module to implement balanced AVL trees.
*
* Revision History:
* -----------------
*
* 92/07/20  21:45:05  WALTER FONTANA
* Three fatal bugs removed (in balance_right())
*
* 91/12/31  19:40:43  kjb
* Modified include file directories
* 
* 91/09/27  03:10:00  kjb
* Added stuff keep track of the number of nodes in the tree.
* 
* 91/09/27  02:50:49  kjb
* Initial revision
* 
****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <signal.h>
#include <stdlib.h>
#include "avl.h"

#define	MAXLEVEL	64	/* Maximum levels for avl_print */

/* The three balance factors stored in each subtree                                             */

#define	LH		0	/* Left subtree is larger */
#define	BAL		1	/* Subtree is balanced */
#define	RH		2	/* Right subtree is balanced */

/*------------------------------ Macros -----------------------------------*/
/*-------------------------------------------------------------------------*/

/* 
 * Return a pointer to the user space given the address of the header of
 * a node.
 */

#define	AVL_USERSPACE(h)    ((void*)((AVL_BUCKET*)(h) + 1))

/* 
 * Return a pointer to the header of a node, given the address of the
 * user space.
 */

#define	AVL_HEADER(n)	((AVL_BUCKET*)(n) - 1)
#define	AVL_EMPTY(t)	((t)->count == 0)

/*------------------------------ Globals ----------------------------------*/
/*-------------------------------------------------------------------------*/

PRIVATE AVL_TREE *Tree;		/* Pointer to tree we are working with  */
PRIVATE AVL_BUCKET *Node;	/* Pointer to node we are working with  */
PRIVATE void *Conflicting;	/* Conflicting node */
PRIVATE boolean Notfound;	/* Flags if a node cannot be found  */
PRIVATE void (*Visit) ();
PRIVATE void (*Prnt) ();
PRIVATE void *Params, *Lower, *Upper;
PRIVATE FILE *Out;
PRIVATE char Map[MAXLEVEL / 8];
PRIVATE char *Graph_chars[] =
{"�", "���", "���", "�Ĵ", "���", "�Ŀ"};
PRIVATE char *Norm_chars[] =
{"|", "+--", "+--", "--+", "--+", "--+"};
PRIVATE char **Cset;

/*----------------------------- Implementation ----------------------------*/
/*-------------------------------------------------------------------------*/

/****************************************************************************
*
* Function:	avl_newnode
* Parameters:	size - Amount of memory to allocate for node
* Returns:	Pointer to the allocated nodes user space.
*
* Description:	Allocates the memory required for a node, adding a small
*		header at the start of the node. We return a reference to
*		the user space of the node, as if it had be allocated via
*		malloc.
*
****************************************************************************/

PUBLIC void *
avl_newnode (int size)
{
  AVL_BUCKET *n;

  if (!(n = (AVL_BUCKET *) malloc (size + sizeof (AVL_BUCKET))))
    {
      fprintf (stderr, "Can't get memory for BUCKET\n");
      exit (1);
    }

  return AVL_USERSPACE (n);	/* Return pointer to user space */
}

/****************************************************************************
*
* Function:	avl_freenode
* Parameters:	node	- Node to free
*
* Description:	Frees a node previously allocated with avl_newnode().
*
****************************************************************************/

PUBLIC void
avl_freenode (void *node)
{
  free (AVL_HEADER (node));
}

/****************************************************************************
*
* Function:	avl_init
* Parameters:	cmp_function	- Function to compare the value of two nodes
*		prnt_function	- Function to print the value of a node
* Returns:	Pointer to the newly created AVL tree.
*
* Description:	Makes a new AVL tree with no elements and returns a pointer
*		to it.
*
****************************************************************************/

PUBLIC AVL_TREE *
avl_init (int (*cmp_function) ())
{
  AVL_TREE *tree;

  if ((tree = (AVL_TREE *) malloc (sizeof (AVL_TREE))) != NULL)
    {
      tree->count = 0;
      tree->cmp = cmp_function;
      tree->root = NULL;
    }
  else
    {
      fprintf (stderr, "Insufficient memory for AVL tree\n");
      exit (1);
    }
  return tree;
}

/****************************************************************************
*
* Function:	free_subtree
* Parameters:	t   - Subtree to free
*
* Description:	Recursive routine to free the elements of a particular
*		subtree.
*
****************************************************************************/

PRIVATE void (*_freeNode) ();

PRIVATE void
free_subtree (AVL_BUCKET * t)
{
  if (t)
    {
      free_subtree (t->left);
      free_subtree (t->right);
      _freeNode (AVL_USERSPACE (t));
    }
}

/****************************************************************************
*
* Function:	avl_empty
* Parameters:	t		- Tree to kill
*		freeNode	- Routine to free each node of the tree
*
* Description:	Deletes the entire AVL tree from memory including all the
*		nodes of the tree. We call the user supplied routine
*		freeNode() to free each node in the tree. This allows the
*		user program to perform any extra processing that is
*		required to kill each node (for example if they contain
*		pointers to other items on the heap). If no extra
*		avl_kill(myTree,avl_freenode);
*
****************************************************************************/

PUBLIC void
avl_empty (AVL_TREE * t, void (*freeNode) ())
{
  _freeNode = freeNode;
  free_subtree (t->root);
  t->root = NULL;
}

/****************************************************************************
*
* Function:	insert
* Parameters:	pp	- Pointer to pointer to root of subtree to insert into
*
* Description:	Internal recursive routine to insert a node into an
*		AVL tree. Expects the global variables Tree, and Node
*		to be set up. If we encounter a duplicate key, we return
*		the address of this key in the global Conflicting;
*
****************************************************************************/

PRIVATE void
insert (AVL_BUCKET ** pp)
{
  AVL_BUCKET *p = *pp;
  AVL_BUCKET *p1, *p2;
  int relation;			/* Relationship between root and Node           */
  static boolean h = 0;		/* Set to indicate that subtree has grown       */

  if (p == NULL)
    {

      /* We have found a valid spot for the node, so insert it and
       * let the higher level recursions know that the tree has
       * subsequently grown.
       */

      p = Node;
      p->left = p->right = NULL;
      p->bal = BAL;
      h = TRUE;
    }
  else if ((relation = Tree->cmp (AVL_USERSPACE (p), AVL_USERSPACE (Node))) == 0)
    {

      /* We have a node with a duplicate key that we cannot insert
       * into the tree. We return the address of the user space for
       * the node in the global Conflicting
       */

      Conflicting = AVL_USERSPACE (p);
      h = FALSE;
    }
  else if (relation > 0)
    {

      /* If relation is > 0, then the current node p is greater than
       * Node, so we insert the node into the left subtree.
       */

      insert (&p->left);

      if (h)
	{			/* The left subtree has grown in height */
	  switch (p->bal)
	    {
	    case LH:

	      /* The left subtree has grown taller than it is allowed,
	       * so we must perform a rebalance of the tree.
	       */

	      p1 = p->left;
	      if (p1->bal == LH)
		{

		  /* Left subtree is LEFTHIGH, so we need to do a
		   * single right rotation to rebalance the subtree p.
		   */

		  p->left = p1->right;
		  p1->right = p;
		  p->bal = BAL;
		  p = p1;
		}
	      else
		{

		  /* Left subtree is RIGHTHIGH, so we need to do a
		   * double right rotation to rebalance the subtree p.
		   */

		  p2 = p1->right;
		  p1->right = p2->left;
		  p2->left = p1;
		  p->left = p2->right;
		  p2->right = p;
		  p->bal = (p2->bal == LH) ? RH : BAL;
		  p1->bal = (p2->bal == RH) ? LH : BAL;
		  p = p2;
		}
	      p->bal = BAL;	/* Tree is now balanced         */
	      h = 0;
	      break;
	    case BAL:

	      /* The subtree was previously balanced, so now it
	       * simply becomes LEFTHIGH.
	       */

	      p->bal = LH;
	      break;
	    case RH:

	      /* The subtree was previously RIGHTHIGH, so now it
	       * has become balanced. In this case the height of this
	       * particular subtree has not grown, so h is reset to
	       * FALSE.
	       */

	      p->bal = BAL;
	      h = FALSE;
	    }
	}
    }
  else
    {

      /* Relation was < 0, indicating that the current node p is smaller
       * than Node, so we insert the new node into the right subtree.
       */

      insert (&p->right);

      if (h)
	{			/* The right subtree has grown in height        */
	  switch (p->bal)
	    {
	    case LH:

	      /* The subtree was previously LEFTHIGH, so now it
	       * has become balanced. In this case the height of this
	       * particular subtree has not grown, so h is reset to
	       * FALSE.
	       */

	      p->bal = BAL;
	      h = FALSE;
	      break;
	    case BAL:

	      /* The subtree was previously balanced, so now it
	       * simply becomes RIGHTHIGH.
	       */

	      p->bal = RH;
	      break;
	    case RH:

	      /* The right subtree has grown taller than it is allowed,
	       * so we must perform a rebalance of the tree.
	       */

	      p1 = p->right;
	      if (p1->bal == RH)
		{

		  /* Right subtree is RIGHTHIGH, so we need to do a
		   * single left rotation to rebalance the subtree p.
		   */

		  p->right = p1->left;
		  p1->left = p;
		  p->bal = BAL;
		  p = p1;
		}
	      else
		{

		  /* Right subtree is LEFTHIGH, so we need to do a
		   * double left rotation to rebalance the subtree p.
		   */

		  p2 = p1->left;
		  p1->left = p2->right;
		  p2->right = p1;
		  p->right = p2->left;
		  p2->left = p;
		  p->bal = (p2->bal == RH) ? LH : BAL;
		  p1->bal = (p2->bal == LH) ? RH : BAL;
		  p = p2;
		}
	      p->bal = BAL;	/* Tree is now balanced         */
	      h = 0;
	    }
	}
    }

  /* Reset the root of this subtree to the new root p, since it may have
   * changed during a rotate of the tree.
   */

  *pp = p;
}

/****************************************************************************
*
* Function:	avl_insert
* Parameters:	tree	- Tree to insert the node into
*		node	- New node to insert into the tree
* Returns:	NULL on success, or a pointer to the conflicting node.
*
* Description:	Inserts a new node into an AVL tree. Duplicate keys are
*		currently not supported.
*
****************************************************************************/

PUBLIC void *
avl_insert (AVL_TREE * tree, void *node)
{
  Tree = tree;
  Node = AVL_HEADER (node);
  Conflicting = NULL;

  insert (&tree->root);
  if (!Conflicting)
    tree->count++;

  return Conflicting;
}

/****************************************************************************
*
* Function:	balance_left
* Parameters:	pp	- Point to root of subtree to balance
* Returns:	True if the entire subtree shrank, FALSE if not
*
* Description:	Routine to rebalance the subtree pointed to by pp when its
*		left subtree has just shrunk. We adjust the balance factors
*		and rebalance the subtree adjusting pp to point to the
*		new root of the subtree.
*
****************************************************************************/

PRIVATE boolean
balance_left (AVL_BUCKET ** pp)
{
  AVL_BUCKET *p, *p1, *p2;
  short b1, b2;
  boolean subtree_shrank = TRUE;

  p = *pp;
  switch (p->bal)
    {
    case LH:

      /* The subtree was previously left high. Since the left subtree
       * just shrank, this tree is now balanced while the overall
       * height has been reduced.
       */

      p->bal = BAL;
      break;
    case BAL:

      /* The subtree was previously balanced, so we simply make
       * it right high, and signal that it did not shrink.
       */

      p->bal = RH;
      subtree_shrank = FALSE;
      break;
    case RH:

      /* The subtree was previously right high. In this case we must do
       * either a single or double left rotation. If the right
       * subtree was balanced or right high, we only need a single
       * left rotation. We do a double left rotate if the right subtree
       * was left high.
       */

      p1 = p->right;
      b1 = p1->bal;
      if (b1 >= BAL)
	{
	  p->right = p1->left;	/* Single left rotatation       */
	  p1->left = p;
	  if (b1 == RH)
	    p->bal = p1->bal = BAL;
	  else
	    {
	      p->bal = RH;
	      p1->bal = LH;
	      subtree_shrank = FALSE;
	    }
	  p = p1;		/* Right subtree is new root    */
	}
      else
	{
	  p2 = p1->left;	/* Double left rotation         */
	  b2 = p2->bal;
	  p1->left = p2->right;
	  p2->right = p1;
	  p->right = p2->left;
	  p2->left = p;
	  p->bal = (b2 == RH) ? LH : BAL;
	  p1->bal = (b2 == LH) ? RH : BAL;
	  p2->bal = BAL;
	  p = p2;
	}
    }
  *pp = p;
  return subtree_shrank;
}

/****************************************************************************
*
* Function:	balance_right
* Parameters:	pp	- Point to root of subtree to balance
* Returns:	True if the entire subtree shrank, FALSE if not
*
* Description:	Routine to rebalance the subtree pointed to by pp when its
*		right subtree has just shrunk. We adjust the balance factors
*		and rebalance the subtree adjusting pp to point to the
*		new root of the subtree.
*
****************************************************************************/

PRIVATE boolean
balance_right (AVL_BUCKET ** pp)
{
  AVL_BUCKET *p, *p1, *p2;
  short b1, b2;
  boolean subtree_shrank = TRUE;

  p = *pp;
  switch (p->bal)
    {
    case RH:

      /* The subtree was previously right high. Since the right subtree
       * just shrank, this tree is now balanced while the overall
       * height has been reduced.
       */

      p->bal = BAL;
      break;
    case BAL:

      /* The subtree was previously balanced, so we simply make
       * it right high, and signal that it did not shrink.
       */

      p->bal = LH;
      subtree_shrank = FALSE;
      break;
    case LH:

      /* The subtree was previously left high. In this case we must do
       * either a single or double right rotation. If the left
       * subtree was balanced or left high, we only need a single
       * right rotation. We do a double right rotate if the left
       * subtree was right high.
       */

      p1 = p->left;
      b1 = p1->bal;
      if (b1 <= BAL)
	{
	  p->left = p1->right;	/* Single right rotation        */
	  p1->right = p;
	  if (b1 == LH)
	    p->bal = p1->bal = BAL;
	  else
	    {
	      p->bal = LH;
	      p1->bal = RH;
	      subtree_shrank = FALSE;
	    }
	  p = p1;
	}
      else
	{
	  p2 = p1->right;	/* Double right rotation        */
	  b2 = p2->bal;
	  p1->right = p2->left;
	  p2->left = p1;
	  p->left = p2->right;
	  p2->right = p;
	  p->bal = (b2 == LH) ? RH : BAL;
	  p1->bal = (b2 == RH) ? LH : BAL;
	  p2->bal = BAL;
	  p = p2;
	}
    }
  *pp = p;
  return subtree_shrank;
}

/****************************************************************************
*
* Function:	descend
* Parameters:	rootp	- Pointer to pointer to tree to descend
*		pp	- Pointer to pointer to node to replace
* Returns:	True if the subtree just shrank, FALSE if not.
*
* Description:	Routine to descend to the rightmost node of the left
*		subtree. When we get there, we move it up into the position
*		occupied by pp (retaining pp's balance factor) and delete
*		this node. On the way back we rebalance the tree if
*		necessary.
*
****************************************************************************/

PRIVATE boolean
descend (AVL_BUCKET ** rootp, AVL_BUCKET ** pp)
{
  AVL_BUCKET *p = *rootp;
  boolean has_shrunk, was_left = 0;

  if (p->right)
    {

      /* Continue to descend the right subtree, until we can go no
       * further. On the way back up the recursion, if the tree has
       * shrunk, we rebalance it.
       */

      if (rootp == &(*pp)->left)
	was_left = TRUE;
      has_shrunk = descend (&p->right, pp);
      if (was_left)
	rootp = &(*pp)->left;
      return has_shrunk ? balance_right (rootp) : 0;
    }
  else
    {
      *rootp = p->left;		/* Kill the old link to p               */

      p->bal = (*pp)->bal;	/* Retain the old balance factor        */
      p->left = (*pp)->left;	/* and links                                            */
      p->right = (*pp)->right;
      Node = *pp;		/* Free the node                        */
      *pp = p;			/* Replace with p                       */
      return TRUE;		/* Subtree has shrunk - rebalance       */
    }
}

/****************************************************************************
*
* Function:	__delete
* Parameters:	pp	- Pointer to pointer to root of subtree to delete from
* Returns:	True if the subtree shrunk in size, FALSE if not.
*
* Description:	Internal recursive routine to delete a node from an
*		AVL tree. Expects the global variables Tree, and Node
*		to be set up. If the node is not found, we set the global
*		Notfound to TRUE.
*
****************************************************************************/

PRIVATE boolean
__delete (AVL_BUCKET ** pp)
{
  AVL_BUCKET *p = *pp;
  int relation;

  if (p == NULL)
    {

      /* We could not locate the node in the tree, so set Notfound to
       * TRUE.
       */

      Notfound = TRUE;
      return FALSE;
    }
  else if ((relation = Tree->cmp (AVL_USERSPACE (p), AVL_USERSPACE (Node))) == 0)
    {

      /* We have found the node, so delete it from the tree.
       */

      if (p->right == NULL)
	{

	  /* There is only a single left node in the tree, so move this
	   * node up to where p used to be, and then delete p.
	   */

	  *pp = p->left;
	  Node = p;
	  return TRUE;
	}
      else if (p->left == NULL)
	{

	  /* There is only a single right node in the tree, so move this
	   * node up to where p used to be, and then delete p.
	   */

	  *pp = p->right;
	  Node = p;
	  return TRUE;
	}
      else if (descend (&p->left, pp))
	return balance_left (pp);
    }
  else if (relation > 0)
    {

      /* If relation is > 0, then the current node p is greater than
       * Node, so we delete node from the left subtree. If the left
       * subtree has shrunk in size, we rebalance the tree and return
       * the result of this rebalance (tree shrunk, or stayed the same
       * size).
       */

      if (__delete (&p->left))
	return balance_left (pp);
    }
  else
    {

      /* If relation is < 0, then the current node p is less than
       * Node, so we delete node from the right subtree. If the right
       * subtree has shrunk in size, we rebalance the tree and return
       * the result of this rebalance (tree shrunk, or stayed the same
       * size).
       */

      if (__delete (&p->right))
	return balance_right (pp);
    }
  return FALSE;
}

/****************************************************************************
*
* Function:	avl_delete
* Parameters:	tree	- Tree to delete node from
*		node	- Node to delete from tree
* Returns:	Pointer to the node if successful, NULL of failure
*
* Description:	Deletes a node from an AVL tree. The node is removed from
*		the tree, but it's memory is not released. You must call
*		avl_freenode() to free the nodes memory.
*
****************************************************************************/

PUBLIC void *
avl_delete (AVL_TREE * tree, void *node)
{
  Tree = tree;
  Node = AVL_HEADER (node);
  Notfound = FALSE;

  __delete (&tree->root);
  if (!Notfound)
    tree->count--;

  return Notfound ? NULL : AVL_USERSPACE (Node);
}

/*-------------------------------------------------------------------------*/

PRIVATE void
pre_order (AVL_BUCKET * p, void (*visit) ())
{
  if (p)
    {
      visit (Params, AVL_USERSPACE (p));
      pre_order (p->left, visit);
      pre_order (p->right, visit);
    }
}

PRIVATE void
in_order (AVL_BUCKET * p, void (*visit) ())
{
  if (p)
    {
      in_order (p->left, visit);
      visit (Params, AVL_USERSPACE (p));
      in_order (p->right, visit);
    }
}

PRIVATE void
post_order (AVL_BUCKET * p, void (*visit) ())
{
  if (p)
    {
      post_order (p->left, visit);
      post_order (p->right, visit);
      visit (Params, AVL_USERSPACE (p));
    }
}

/****************************************************************************
*
* Function:	avl_preorder
* Parameters:	tree	- Tree to traverse
*
* Description:	Traverses the AVL tree in a pre-order fashion, visiting
*		the node first, then the left and then the right subtree.
*
*		The function visit() must accept two parameters. The second
*		is a pointer to the node to visit, while the first is the
*		params argument passed to avl_traverse.
*
****************************************************************************/

PUBLIC void
avl_traverse (AVL_TREE * tree, int order, void (*visit) (), void *params)
{
  Params = params;
  switch (order)
    {
    case PREORDER:
      pre_order (tree->root, visit);
      break;
    case INORDER:
      in_order (tree->root, visit);
      break;
    case POSTORDER:
      post_order (tree->root, visit);
    }
}

/*-------------------------------------------------------------------------*/

/* Macro to test if the depth bit for level c is set    */

#define testbit(level)	(Map[level >> 3] & (1 << (level & 0x07)))

#ifdef DEBUG
#define  PAD()    fprintf(Out,"   ");
#define  PBAL(r)  fprintf(Out,"(%c)", r->bal == BAL ? 'B' : r->bal==LH ? 'L' : 'R');
#else
#define  PAD()
#define  PBAL(r)
#endif

/****************************************************************************
*
* Function:	setbit
* Parameters:	depth	- Depth of bit to set
*		set_it	- True to set bit, FALSE to reset it
*
* Description:	Sets or resets the depth bit for a level. If this bit is
*		set, we we draw a vertical line at that depth level.
*
****************************************************************************/

PRIVATE void
setbit (int level, boolean set_it)
{
  if (set_it)
    Map[level >> 3] |= 1 << (level & 0x07);
  else
    Map[level >> 3] &= ~(1 << (level & 0x07));
}

/****************************************************************************
*
* Function:	print_tree
* Parameters:	root		- Root of subtree to display
*		left_subtree	- True if this is a left subtree
*
* Description:	Dumps the contents of subtree 'root' to the file Out.
*
****************************************************************************/

PRIVATE void
print_tree (AVL_BUCKET * root, int left_subtree)
{
  static int depth = -1;	/* Current depth within subtree */
  int i;

  if (root)
    {
      depth++;

      if (root->right)
	print_tree (root->right, FALSE);
      else
	setbit (depth + 1, TRUE);

      for (i = 1; i <= depth; i++)
	{
	  Prnt (Out, NULL);
	  PAD ();

	  if (i == depth)
	    fprintf (Out, "  %s", left_subtree ? Cset[2] : Cset[1]);
	  else if (testbit (i))
	    fprintf (Out, "  %s  ", Cset[0]);
	  else
	    fprintf (Out, "     ");
	}

      Prnt (Out, AVL_USERSPACE (root));
      PBAL (root);
      fprintf (Out, "%s\n", root->left ? (root->right ? Cset[3] : Cset[5]) :
	       (root->right ? Cset[4] : ""));

      setbit (depth, left_subtree ? FALSE : TRUE);

      if (root->left)
	print_tree (root->left, TRUE);
      else
	setbit (depth + 1, FALSE);
      depth--;
    }
}

/****************************************************************************
*
* Function:	avl_print
* Parameters:	out		- Stream to print to
*		tree		- Tree to print out
*		prnt		- Routine to print each node
*		graph_chars	- True if we print with graphics characters
*
* Description:	Dumps the contents of the AVL tree to the specified file.
*		If graph_chars is TRUE, the structure of the tree is
*		printed using IBM graphics characters, otherwise it is
*		printed using standard ASCII characters. If show_balance
*		is TRUE, the balance factors are displayed.
*
*		The routine prnt() takes a pointer to a file to print the
*		node to, and a pointer to the node to print. It must print
*		every node in a field of the same width, and when passed
*		a null pointer for the node it should print a blank field
*		the same width as the nodes. This provides correct
*		formatting for the printed tree.
*
*		The tree can have a maximum height of 64 levels. If the
*		tree is taller than this, the result is undefined. A
*		balanced AVL tree with 64 levels would have in the order
*		of 1e19 nodes!
*
****************************************************************************/

PUBLIC void
avl_print (FILE * out, AVL_TREE * tree, void (*prnt) (), boolean graph_chars)
{
  Tree = tree;
  Out = out;
  Prnt = prnt;
  Cset = graph_chars ? Graph_chars : Norm_chars;
  print_tree (tree->root, FALSE);
}

/****************************************************************************
*
* Function:	avl_findsym
* Parameters:	tree	- Tree to search for the symbol
*		node	- Node containing key to search for
* Returns:	Pointer to the node if found, NULL if not.
*
* Description:	Searches the AVL tree for the specified node and returns
*		it if it is found.
*
****************************************************************************/

PUBLIC void *
avl_findsym (AVL_TREE * tree, void *node)
{
  AVL_BUCKET *p;
  int relation;

  p = tree->root;

  while (p)
    {
      if ((relation = tree->cmp (node, AVL_USERSPACE (p))) == 0)
	return AVL_USERSPACE (p);
      else if (relation > 0)
	p = p->right;
      else
	p = p->left;
    }
  return NULL;
}

/****************************************************************************
*
* Function:	range_search
* Parameters:	root	- Root of subtree to range search
*
* Description:	Recursive routine to range search a subtree.
*
****************************************************************************/

PRIVATE void
range_search (AVL_BUCKET * root)
{
  if (root)
    {
      if (Tree->cmp (AVL_USERSPACE (root), Lower) < 0)
	{
	  /* If the root node is smaller than the lower bound, then recurse
	   * down the right subtree.
	   */

	  range_search (root->right);
	}
      else if (Tree->cmp (AVL_USERSPACE (root), Upper) > 0)
	{
	  /* If the root node is larger than the lower bound, then recurse
	   * down the left subtree.
	   */

	  range_search (root->left);
	}
      else
	{
	  /* We are within the range so recurse down the left subtree,
	   * visit the node and then recurse down the right subtree.
	   */

	  range_search (root->left);
	  Visit (Params, AVL_USERSPACE (root));
	  range_search (root->right);
	}
    }
}

/****************************************************************************
*
* Function:	avl_range
* Parameters:	tree	- Tree to range search
*		lower	- Lower bound of range search (inclusive)
*		upper	- Upper bound of range search (inclusive)
*		visit	- Function to visit each node in range
*		params	- Parameters for the visit routine.
*
* Description:	Calls visit for every node in the range [lower,upper]
*		for the AVL tree 'tree'. Visit must accept two parameters.
*		The first is the params argument passed to this function,
*		while the second is a pointer to the node to visit.
*
****************************************************************************/

PUBLIC void
avl_range (AVL_TREE * tree, void *lower, void *upper, void (*visit) (), void *params)
{
  Tree = tree;
  Visit = visit;
  Params = params;
  Lower = lower;
  Upper = upper;
  range_search (tree->root);
}

/****************************************************************************
*
* Function:	delmin
* Parameters:	root	- Root of subtree to delete from
* Returns:	True if the subtree shrank.
*
* Description:	Recursive routine to delete the smallest node from an
*		AVL tree.
*
****************************************************************************/

int
delmin (AVL_BUCKET ** pp)
{
  AVL_BUCKET *p = *pp;

  if (p->left)
    return delmin (&p->left) ? balance_left (pp) : 0;
  else
    {
      *pp = p->right;		/* Delete the node      */
      Node = p;
      return TRUE;		/* Subtree just shrank  */
    }
}

/****************************************************************************
*
* Function:	avl_delmin
* Parameters:	tree	- Tree to delete node from
* Returns:	Pointer to the node found, or NULL on error.
*
* Description:	Deletes (removes) the smallest node from an AVL tree. The
*		tree is rebalanced if need be. Note that the node is not
*		freed. You will need to call avl_freenode() to do this.
*
****************************************************************************/

PUBLIC void *
avl_delmin (AVL_TREE * tree)
{
  Node = NULL;
  delmin (&tree->root);
  if (Node)
    tree->count--;
  return Node ? AVL_USERSPACE (Node) : NULL;
}

/****************************************************************************
*
* Function:	delmax
* Parameters:	root	- Root of subtree to delete from
* Returns:	True if the subtree shrank.
*
* Description:	Recursive routine to delete the largest node from an
*		AVL tree.
*
****************************************************************************/

PRIVATE int
delmax (AVL_BUCKET ** pp)
{
  AVL_BUCKET *p = *pp;

  if (p->right)
    return delmax (&p->right) ? balance_right (pp) : 0;
  else
    {
      *pp = p->left;		/* Delete the node      */
      Node = p;
      return TRUE;		/* Subtree just shrank  */
    }
}

/****************************************************************************
*
* Function:	avl_delmax
* Parameters:	tree	- Tree to delete node from
* Returns:	Pointer to the node found, or NULL on error.
*
* Description:	Deletes (removes) the largest node from an AVL tree. The
*		tree is rebalanced if need be. Note that the node is not
*		freed. You will need to call avl_freenode() to do this.
*
****************************************************************************/

PUBLIC void *
avl_delmax (AVL_TREE * tree)
{
  Node = NULL;
  delmax (&tree->root);
  if (Node)
    tree->count--;
  return Node ? AVL_USERSPACE (Node) : NULL;
}

/*-------------------------------------------------------------------------*/
