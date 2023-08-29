/*
    randomexpr.c    

    c 1994 Walter Fontana
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "utilities.h"
#include "structs.h"
#include "randomexpr.h"

PUBLIC char *random_expression (parmsRandExpr *Params, basis *Basis, 
				interpreter *Lambda);

PRIVATE char *lambda_expression (parmsRandExpr *Params, basis *Basis,
				 interpreter *Lambda);
PRIVATE char *make_expression (int level, float ABS, float APP, int depth, 
			       int alpha);

/*----------------------------------------------------------------------*/

PRIVATE char variables[] = "abcdefghijklmnopqrstuvwxyz";
PRIVATE float APP_incr;
PRIVATE float ABS_incr;

/*----------------------------------------------------------------------*/

PRIVATE char *
lambda_expression (parmsRandExpr *Params, basis *Basis, interpreter *Lambda)
{
  char buffer[BUFSIZE];
  char *A, *result;
  int l;

  APP_incr = (Params->appl[1] - Params->appl[0]) / (Params->max_depth - 1);
  ABS_incr = (Params->abst[1] - Params->abst[0]) / (Params->max_depth - 1);

  A = make_expression (0, Params->appl[0], Params->abst[0],
		       Params->max_depth, Params->size_alpha);
  
  l = strlen (A);
  if (l >= BUFSIZE)
    {
      printf ("random expression is too long (%d)\n", l);
      printf ("you should adjust your parameters\n");
      nrerror ("Exiting...");
    }

  /* reduce to normal form */

  strcpy (buffer, "eval ");
  strcat (buffer, A);
  strcat (buffer, ";");
  free (A);

  result = reduce_lambda (buffer, Lambda);
  
  if (!result)
    return NULL;
    
  if (Params->bind)
    {
      /* bind free variables */

      A = bind_all_free_vars (result, Lambda);
      free (result);
      result = A;
    }

  if (Basis)
    {
      /* annotate with typing basis */
  
      A = annotate (result, Basis);
      free (result);
      result = A;
    }
    
  /* standardize */

  A = standardize (result, Lambda);
  free (result);
  return A;
}

/*----------------------------------------------------------------------*/

PRIVATE char *
make_expression (int level, float ABS, float APP, int depth, int alpha)
{
  double coin;
  char var[] =
  {'\0', '\0'};
  char *A, *B, *expression;

  if (++level > depth)
    {
      /* cut off with variable */

      expression = (char *) space (sizeof (char) * 2);
      expression[0] = variables[int_urn (0, alpha - 1)];

      return expression;
    }

  coin = urn ();

  if (coin <= ABS)
    {
      /* abstraction =============================================== */

      /* 1. get a variable */

      var[0] = variables[int_urn (0, alpha - 1)];

      /* 2. get expression */

      A = make_expression (level, ABS + ABS_incr, APP + APP_incr, depth, alpha);

      expression = (char *) space (sizeof (char) * (strlen (A) + 4));

      strcpy (expression, "\\");
      strcat (expression, var);
      strcat (expression, ".");
      strcat (expression, A);

      free (A);

      return expression;
    }
  else if (coin <= (ABS + APP))
    {
      /* application ================================================ */

      A = make_expression (level, ABS + ABS_incr, APP + APP_incr, depth, alpha);
      B = make_expression (level, ABS + ABS_incr, APP + APP_incr, depth, alpha);

      expression = (char *) space (sizeof (char) * (strlen (A) + strlen (B) + 3));

      strcpy (expression, "(");
      strcat (expression, A);
      strcat (expression, ")");
      strcat (expression, B);

      free (A);
      free (B);

      return expression;
    }
  else
    {
      /* variable =================================================== */

      expression = (char *) space (sizeof (char) * 2);
      expression[0] = variables[int_urn (0, alpha - 1)];

      return expression;
    }
}

/*----------------------------------------------------------------------*/

/* returns a standardized type annotated random lambda expression */

PUBLIC char *
random_expression (parmsRandExpr *Params, basis *Basis, interpreter *Lambda)
{
  char *A;

  do
    {
      A = lambda_expression (Params, Basis, Lambda);
    }
  while (!A);

  return (A);
}

/*----------------------------------------------------------------------*/

#ifdef RANDOMEXPR_TEST

int
main (int argc, char **argv)
{
  int c = 0;
  char *A;
  interpreter * Lambda;
  parmsLambda * Parameters;
  parmsRandExpr * RandExpr;

  RandExpr = (parmsRandExpr *) space ( sizeof (parmsRandExpr));
  Parameters = (parmsLambda *) space ( sizeof (parmsLambda));

  RandExpr->max_depth = 10;
  RandExpr->size_alpha = 5;
  RandExpr->bind = 1;
  RandExpr->appl[0] = 0.5;
  RandExpr->appl[1] = 0.5;
  RandExpr->abst[0] = 0.4;
  RandExpr->abst[1] = 0.4;

  Parameters->heap_size = 4000;	/* size of heap */
  Parameters->cycle_limit = 100000; /* maximum number of cycles */
  Parameters->symbol_table_size = 500;	/* size of symbol table */
  Parameters->stack_size = 2000;  /* stack size */
  Parameters->name_length = 10;	/* max length of identifiers */
  Parameters->type_length = 300; /* max length of types */
  Parameters->standard_variable = 'x';	/* name of standard variable */
  Parameters->error_fp = stdout;  /* error report */

  Lambda = initialize_lambda (Parameters);

  while (c < 100)
    {
      A = random_expression (RandExpr, NULL, Lambda);
      printf ("%4d: %s\n", c, A); 
      c++;
      free (A);
    }
  return 0;
}

#endif /* RANDOMEXPR_TEST */
