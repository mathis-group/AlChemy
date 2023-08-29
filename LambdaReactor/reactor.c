/*
    reactor.c    

    c 1994 Walter Fontana
 */

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <malloc.h>
#include "utilities.h"
#include "structs.h"
#include "avlaccess.h"
#include "interact.h"
#include "type_client.h"

PUBLIC REC *add_to_system (int id, char *object, simulation * S, int type);
PUBLIC void pair_interactions (simulation * S, interpreter * L, int pure);
PUBLIC int which_systems (char *item, int *systems, simulation * S);
PUBLIC REC *choose_from_system (int sys, simulation * S);
PUBLIC int choose_system (simulation * S);

PRIVATE void flow (simulation * S);
PRIVATE void choose_object (void *params, REC * node);
PRIVATE void remove_from_system (int id, char *object, simulation * S);
PRIVATE int identifier (char *item, AVL_TREE *database);
PRIVATE void index_of_object (void *params, REC * node);
PRIVATE void traverse1 (void *params, REC * node);
PRIVATE void traverse2 (void *params, REC * node);

/*===========================================================================*/

PRIVATE double PROB = 0.0;
PRIVATE double SUM = 0.0;
PRIVATE int PURE;
PRIVATE int OP;
PRIVATE int ARG;
PRIVATE int INDEX;
PRIVATE int OBJ[2];
PRIVATE REC * NODE[2];
PRIVATE char *OBJECT = NULL;
PRIVATE FILE *pairFP = NULL;
PRIVATE simulation * SIMUL;
PRIVATE interpreter * LAMBDA;

PRIVATE jmp_buf BACK_FROM_TRAVERSE; /* save context for long jump */

/*=======================================================================*/

PUBLIC REC *
add_to_system (int id, char *object, simulation * S, int type)
{
  int found;
  char *sigma;
  REC *node;

  if (S->num_objects < S->max_objects)
    {
      node = AVL (INSERT, S->Org[id]->Objects, object, &found);
      
      /* (if _object has been found, allocated space has been freed) */
      
      node->instances++;
      S->Org[id]->State.num_objects++;
      S->num_objects++;
      
      if (!found)
	{
	  if (type)
	    {
	      /* 
		 type synthesize the normal form; I've put this 
		 here to minimize communication with server.
	       */
	      sigma = type_synthesis (object);
	      node->type  = (char *) space (strlen (sigma) + 1);
	      strcpy (node->type, sigma);
	    }

	  node->sysid = id;
	  S->diversity++;
	  S->new++;
	}
    }
  else
    {
      flow (S);
      node = add_to_system (id, object, S, type);
    }
  return (node);
}

/*----------------------------------------------------------------------*/

PRIVATE void
flow (simulation * S)
{
  int o;
  REC *n;

  o = choose_system (S);
  n = choose_from_system (o, S);
  remove_from_system (o, n->object, S);
  return;
}

/*----------------------------------------------------------------------*/

PUBLIC REC *
choose_from_system (int sys, simulation * S)
{
  int jm;

  jm = setjmp (BACK_FROM_TRAVERSE);
  if (jm)
    {
      return NODE[0];
    }
  else
    {
      SUM = 0.0;
      PROB = (urn () * S->Org[sys]->State.num_objects);
      traverse_AVL (S->Org[sys]->Objects, choose_object);
    }
}

/*----------------------------------------------------------------------*/

PUBLIC int
choose_system (simulation * S)
{
  double select, s;
  int i;

  select = (urn () * S->num_objects);
  s = 0.0;
  for (i = 0; i < S->systems; i++)
    {
      s += (double) S->Org[i]->State.num_objects;
      if (s > select)
	return i;
    }

  nrerror ("choose_system: a mess");
}

/*----------------------------------------------------------------------*/

/* traversal routine */

PRIVATE void
choose_object (void *params, REC * node)
{
  SUM += (double) node->instances;
  if (SUM >= PROB)
    {
      NODE[0] = node;
      OBJECT = node->object;
      longjmp (BACK_FROM_TRAVERSE, 1);
    }
}

/*----------------------------------------------------------------------*/

PRIVATE void
remove_from_system (int id, char *object, simulation * S)
{
  int found;
  REC *nodeA, *nodeB;

  nodeA = AVL (FIND, S->Org[id]->Objects, object, &found);

  if (nodeA)
    {
      nodeA->instances--;
      S->Org[id]->State.num_objects--;
      S->num_objects--;
      if (nodeA->instances == 0)
	{
	  S->diversity--;
	  nodeB = AVL (DELETE, S->Org[id]->Objects, object, &found);
	  free_node (nodeB);
	}
    }
  else
    nrerror ("remove_from_system: object does not exist");
}

/*======================================================================*/

/* 
    pair_interactions does not handle multiple interaction laws,
    but it would be easy to change it; may happen when need is pressing...
*/

PUBLIC void
pair_interactions (simulation * S, interpreter * L, int pure)
{
  char buffer[BUFSIZE];
  FILE * fopen();

  fprintf (stdout, "\nname: %s\n", S->Org[S->systems-1]->Params.input_objects);
  fflush(stdout);
  
  strcpy (buffer, S->Org[S->systems-1]->Params.input_objects);
  strcat (buffer, ".pairs");
  pairFP = fopen (buffer, "w");
  
  SIMUL = S;
  LAMBDA = L;
  PURE = pure;

  for (OP = 0; OP < SIMUL->systems; OP++)
    {
      for (ARG = 0; ARG < SIMUL->systems; ARG++)
	{
	  OBJ[0] = 0;
	  traverse_AVL (SIMUL->Org[OP]->Objects, traverse1);
	}
    }
  fclose (pairFP);
  SIMUL = NULL;
  LAMBDA = NULL;
}

/*----------------------------------------------------------------------*/

PRIVATE void
traverse1 (void *params, REC * node)
{
  OBJ[0]++;
  NODE[0] = node;
  OBJ[1] = 0;
  traverse_AVL (SIMUL->Org[ARG]->Objects, traverse2);
}

/*----------------------------------------------------------------------*/

PRIVATE void
traverse2 (void *params, REC * node)
{
  extern char I_CODE;   /*
			   O = failed operator filter
			   A = failed argument filter
			   T = failed typing
			   R = failed result filter
			   + = over limits
		         */
  extern int REDUCTIONS;
  
  react * reaction;
  register int i, j, n, idx;
  int sys[10];
  char *p;
  
  OBJ[1]++;
  NODE[1] = node;
  
  reaction = Reaction (NODE[0], NODE[1], SIMUL, LAMBDA, 0);

  /* log the interaction */
  
  fprintf (pairFP, "%3d [%d] : %3d [%d] => ", OBJ[0], NODE[0]->sysid,
		    OBJ[1], NODE[1]->sysid);

  if (reaction)
    {
      i = -1;
      while (p = reaction->product[++i])
	{
	  if (i)
	    fprintf (pairFP, "%21c", ' ');
	  n = which_systems (p, sys, SIMUL);
	  if (n == 0)
	    fprintf (pairFP, " *   %s\n", p);
	  for (j = 0; j < n; j++)
	    {
	      idx = identifier (p, SIMUL->Org[sys[j]]->Objects);
	      fprintf (pairFP, "%3d [%d] %d\n", idx, sys[j], REDUCTIONS);
	    }
	}
    }
  else if (PURE)
    {
      p = pure_lambda_action (NODE[0]->object, NODE[1]->object, 
			  SIMUL->Org[NODE[0]->sysid]->Law[0].expr, 
			  LAMBDA);
      fprintf (pairFP, "-|- [%s]\n", p);
      if (p) 
	free (p);
    }
  else
    {
      fprintf (pairFP, " %c\n", I_CODE);
    }
  fflush (pairFP);

  /* clean up */

  if (reaction)
    {
      i = -1;
      while (p = reaction->product[++i]) free (p);
      free (reaction);
    }
}

/*======================================================================*/

/* which systems contain "item"? */

PUBLIC int
which_systems (char *item, int *systems, simulation * S)
{
  register int i, found;
  int dummy;
  
  found = 0;
  for (i = 0; i < S->systems; i++)
    {
      if (AVL (FIND, S->Org[i]->Objects, item, &dummy))
	systems[found++] = i;
    }
  return found;
}

/*----------------------------------------------------------------------*/

/* get index of "item" in database */

PRIVATE int
identifier (char *item, AVL_TREE *database)
{
  int jm;
  
  INDEX = 0;
  OBJECT = item;
  jm = setjmp (BACK_FROM_TRAVERSE);
  if (jm)
      return INDEX;
  else
      traverse_AVL (database, index_of_object);
}  

/*----------------------------------------------------------------------*/

/* traversal routine */

PRIVATE void
index_of_object (void *params, REC * node)
{
  INDEX++;
  if (strcmp (OBJECT, node->object) == 0)
      longjmp (BACK_FROM_TRAVERSE, 1);
}

/*----------------------------------------------------------------------*/
