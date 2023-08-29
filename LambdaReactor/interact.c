/*
    interact.c    

    c 1994,1995 Walter Fontana
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "peephole.h"
#include "interact.h"
#include "conversions.h"
#include "type_client.h"

PUBLIC react *Reaction (REC * A, REC * B, simulation * S, interpreter * L, int show);
PUBLIC char *pure_lambda_action (char *A, char *B, char *Law, interpreter * L);

PRIVATE react *Original_reaction (simulation * S, interpreter * L);
PRIVATE void lambda_action (int both, simulation * S, interpreter * L);
PRIVATE char *choose_law (organization * Org);
PRIVATE char *_lambda_action (char *A, organization * OA, char *B, organization * OB,
			      simulation * S, interpreter * L);

PRIVATE Action lambda;

PUBLIC char I_CODE;    /* 
			  O = failed operator filter
			  A = failed argument filter
			  T = failed typing
			  R = failed result filter
			  + = over limits
		       */

PUBLIC int REDUCTIONS; /* number of reductions in the last normalization */

/*======================================================================*/

PUBLIC react *
Reaction (REC * A, REC * B, simulation * S, interpreter * L, int show)
{
  react * reaction;
  register int i;
  
  lambda.f[0] = A;
  lambda.f[1] = B;

  switch (S->Reaction)
    {
    case ORIGINAL:
      reaction = Original_reaction (S, L);

      if (show)
	{
	  fprintf (stdout, "coll %d between\n", S->bangs);
	  fprintf (stdout, "LAMBDA: %s\n", lambda.f[0]->object);
	  if (S->type_check && S-> type_save)
	    fprintf (stdout, "  TYPE: %s\n", lambda.f[0]->type);
	  fprintf (stdout, "and\n");
	  fprintf (stdout, "LAMBDA: %s\n", lambda.f[1]->object);
	  if (S->type_check && S-> type_save)
	    fprintf (stdout, "  TYPE: %s\n", lambda.f[1]->type);
	  fprintf (stdout, "yields\n");
	  if (reaction)
	    {
	      i = -1;
	      while (reaction->product[++i])
		fprintf (stdout, "LAMBDA: %s\n", reaction->product[i]);
	    }
	  else
	    fprintf (stdout, "ELASTIC\n");
	  
	  fflush (stdout);
	}
      
      return reaction;
      break;
      
    default:
      nrerror ("reaction not implemented");
      break;
    }
}

/*----------------------------------------------------------------------*/

PRIVATE react *
Original_reaction (simulation * S, interpreter * L)
{
  float P;
  react *reaction;

  /* 
     this looks asymmetric, but is not; for observe that
     we could also call the symmetric lambda_action(lambda, 1, S, L)
     and choose randomly between the two products. This, however,
     is statistically the same as calling lambda_action(lambda, 0, S, L)
     and relying on the randomness in the choice sequence of the reactants.
   */

  lambda_action (0, S, L);

  if (lambda.action[0] == NULL)
    return NULL;

  reaction = (react *) calloc(1, sizeof (react));

  /* one product (terminate product-array with NULL) */

  reaction->product[0] = lambda.action[0];

  /* apply copy filter? */

  P = S->Org[lambda.f[0]->sysid]->Filter.fun.copy_prob;

  if (P == 1.)			/* no */
    return reaction;

  /* do we have a copy action? */

  if (strcmp (reaction->product[0], lambda.f[0]->object) == 0
      || strcmp (reaction->product[0], lambda.f[1]->object) == 0)
    {
      /* yes, we do */

      if (P != 0.)
	{
	  if (urn () < P)
	    return reaction;
	}
      free (lambda.action[0]);
      free (reaction);
      return NULL;
    }
   else
    return reaction;
}

/*----------------------------------------------------------------------*/

/* lexically filtered lambda action of two objects A and B */

PRIVATE void
lambda_action (int both, simulation * S, interpreter * L)
{
  char *A, *B;
  int a, b;

  A = lambda.f[0]->object;
  B = lambda.f[1]->object;
  a = lambda.f[0]->sysid;
  b = lambda.f[1]->sysid;

  /* (A)B -> action[0] */

  lambda.action[0] = _lambda_action (A, S->Org[a], B, S->Org[b], S, L);
  
  if (both)
    {
      /* (B)A -> action[1] */

      lambda.action[1] = _lambda_action (B, S->Org[b], A, S->Org[a], S, L);
    }
}

/*----------------------------------------------------------------------*/

PRIVATE char *
_lambda_action (char *A, organization * OA, char *B, organization * OB,
		simulation * S, interpreter * L)
{
  char *r, *result, *sigma;
  char buffer[BUFSIZE];

  I_CODE = '+';
  
  /* (A)B -> result */
  
  if ((strlen (A) + strlen (B)) >= (BUFSIZE-500))
    return NULL;

  if (syntax_sieve (A, OA->Filter.regexp[OPERATOR], S->type_check))
    {
      if (syntax_sieve (B, OB->Filter.regexp[ARGUMENT], S->type_check))
	{
	  strcpy (buffer, "eval ((");
	  strcat (buffer, choose_law (OA));
	  strcat (buffer, ")");
	  strcat (buffer, A);
	  strcat (buffer, ")");
	  strcat (buffer, B);

	  /* type checking filter */
	  
	  if (S->type_check)
	    sigma = type_synthesis (buffer + 5);
	  else
	    sigma = buffer; /* something non-NULL */
	    
	  if (sigma)
	    {
	      /* passed filters; now reduce to normal form */

	      strcat (buffer, ";");  /* terminate phrase */
	      
	      r = reduce_lambda (buffer, L);  REDUCTIONS = L->reductions;
	      result = standardize (r, L);
	      free (r);
	      
	      if (syntax_sieve (result, OA->Filter.regexp[RESULT], S->type_check))
		{
		  return result;
		}
	      else
		{
		  I_CODE = 'R';
		  free (result);
		}
	    }
	  else
	    {
	      I_CODE = 'T';
	      S->typeclashes++;
	    }
	}
      else
	{
	  I_CODE = 'A';
	}
    }
  else
    {
      I_CODE = 'O';
    }
  return NULL;
}

/*----------------------------------------------------------------------*/

PUBLIC char *
pure_lambda_action (char *A, char *B, char *Law, interpreter * L)
{
  char *r, *result;
  char buffer[BUFSIZE];

  /* (A)B -> result */

  if ((strlen (A) + strlen (B)) >= (BUFSIZE-200))
    return NULL;

  strcpy (buffer, "eval ((");
  strcat (buffer, Law);
  strcat (buffer, ")");
  strcat (buffer, A);
  strcat (buffer, ")");
  strcat (buffer, B);
  strcat (buffer, ";");

  /* reduce to normal form */

  r = reduce_lambda (buffer, L);
  result = standardize (r, L);
  free (r);

  return result;
}

/*----------------------------------------------------------------------*/

PRIVATE char *
choose_law (organization * Org)
{
  double coin, s;
  int i;

  if (Org->Params.num_laws == 1)
    return Org->Law[0].expr;

  coin = urn ();
  s = 0.0;
  for (i = 0; i < Org->Params.num_laws; i++)
    {
      s += Org->Law[i].prob;
      if (s > coin)
	return Org->Law[i].expr;
    }
  nrerror ("chose_law: no law chosen ");
}

/*----------------------------------------------------------------------*/
