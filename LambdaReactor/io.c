/*
    iofile.c
        
    c 1994 Walter Fontana
 */

#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "avlaccess.h"
#include "randomexpr.h"
#include "reactor.h"
#include "structs.h"
#include "interact.h"
#include "conversions.h"
#include "type_client.h"
#include <stdlib.h>

#define  SEPARATOR  "="
#define  WIDTH	    70	/* line width for printout */


PUBLIC void set_parameters (char *fn, simulation ** S_,
			    parmsLambda ** Lambda_parameters_,
			    parmsRandExpr ** RandExpr_parameters_);
PUBLIC void change_parameters (char *fn, simulation ** S_,
			       parmsLambda ** Lambda_parameters_,
			       parmsRandExpr ** RandExpr_parameters_);
PUBLIC void print_parameters (FILE *fp, simulation * S,
			      parmsLambda * Lambda_parameters);
PUBLIC void get_expressions (simulation * S, parmsRandExpr * RandExpr, 
			     interpreter * Lambda);
PUBLIC void print_reactor (char *fname, simulation * S);
PUBLIC void read_basis (char *fname, simulation * S);

PRIVATE char *next_item (FILE * fp, char *token);
PRIVATE void print_nodes (void *params, REC * node);

/*===========================================================================*/

PRIVATE int COUNT = 0;
PRIVATE int FORMAT = 0;
PRIVATE FILE *OUT_FP = NULL;

/*===========================================================================*/

PUBLIC void
set_parameters (char *fn, simulation ** S_,
		parmsLambda ** Lambda_parameters_,
		parmsRandExpr ** RandExpr_parameters_)
{
  int i, j, k, l, num;
  char *s, sign[2];
  const char *r;
  char token[300], buffer[BUFSIZE];
  FILE *fp, *fopen ();
  
  simulation *S;
  parmsLambda *Lambda_parameters;
  parmsRandExpr *RandExpr_parameters;
  organization ** Org;
  
  fp = fopen (fn, "r");
  
  system ("clear");
  printf ("reading parameters from %s\n", fn);

  /* SIMULATION PARAMETERS *********************************************/

  *S_ = (simulation *) space (sizeof (simulation));
  S = *S_ ;
  
  sscanf (next_item (fp, token), "%s", S->name);
  strcpy (token, S->name);
  strcat (token, ".log");
  S->logfp = fopen (token, "w");
  fprintf(S->logfp, "simulation started: %s\n", time_stamp());

  sscanf (next_item (fp, token), "%s", buffer);
  if (strcmp (buffer, "ORIGINAL") == 0)
    S->Reaction = ORIGINAL;
  if (strcmp (buffer, "MAVELLI/MAESTRO") == 0)
    S->Reaction = MAVELLI_MAESTRO;
  else
    S->Reaction = ORIGINAL;
    
  sscanf (next_item (fp, token), "%s", buffer);
  if (strcmp (buffer, "NULL") != 0)
    {
      S->type_check = 1;
      read_basis (buffer, S);
      fprintf (S->logfp, "\ntyping basis\n\n");
      l = -1;
      while (S->TypeBasis[++l].name)
	{
	  fprintf (S->logfp, "name: %s\n", S->TypeBasis[l].name);
	  fprintf (S->logfp, "type: %s\n\n", S->TypeBasis[l].type);
	}
      if (l == 0) 
	fprintf (S->logfp, "none\n\n");
    }

  sscanf (next_item (fp, token), "%d", &S->type_save);
  sscanf (next_item (fp, token), "%d", &S->systems);
  sscanf (next_item (fp, token), "%d", &S->max_objects);
  sscanf (next_item (fp, token), "%d", &S->collisions);
  sscanf (next_item (fp, token), "%d", &S->snapshot);
  sscanf (next_item (fp, token), "%ld", &S->seed);

  randomize (S->seed);

  /* LAMBDA REDUCTION PARAMETERS ***************************************/

  *Lambda_parameters_ = (parmsLambda *) space (sizeof (parmsLambda));
  Lambda_parameters = *Lambda_parameters_;
  
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->heap_size);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->cycle_limit);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->symbol_table_size);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->stack_size);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->name_length);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->type_length);
  sscanf (next_item (fp, token), "%s", buffer);
  Lambda_parameters->standard_variable = buffer[0];
  sscanf (next_item (fp, token), "%s", buffer);
  if (strcmp (buffer, "NULL") == 0)
    Lambda_parameters->error_fp = NULL;
  else if (strcmp (buffer, "stdout") == 0)
    Lambda_parameters->error_fp = stdout;
  else
    Lambda_parameters->error_fp = fopen (buffer, "w");

  /* RANDOM EXPRESSION GENERATOR PARAMETERS ****************************/

  *RandExpr_parameters_ = (parmsRandExpr *) space (sizeof (parmsRandExpr));
  RandExpr_parameters = *RandExpr_parameters_;
  
  sscanf (next_item (fp, token), "%d", &RandExpr_parameters->max_depth);
  sscanf (next_item (fp, token), "%d", &RandExpr_parameters->size_alpha);
  sscanf (next_item (fp, token), "%d", &RandExpr_parameters->bind);
  sscanf (next_item (fp, token), "%f %f", &RandExpr_parameters->appl[0],
	  &RandExpr_parameters->appl[1]);
  sscanf (next_item (fp, token), "%f %f", &RandExpr_parameters->abst[0],
	  &RandExpr_parameters->abst[1]);

  /* sanity check on probabilities */

  if ((RandExpr_parameters->appl[0]
       + RandExpr_parameters->abst[0]) >= 1.
      || (RandExpr_parameters->appl[1]
	  + RandExpr_parameters->abst[1]) >= 1.)
    nrerror ("random expression generator probabilities are phony");

  /* ORGANIZATION PARAMETERS *******************************************/

  Org = (organization **) space (sizeof (organization *) * S->systems);
  S->Org = Org;
  
  for (i = 0; i < S->systems; i++)
    {
      Org[i] = (organization *) space (sizeof (organization));
      
      /* get AVL data structure for objects */

      Org[i]->Objects = initialize_tree ();
      
      /* initial objects file */
      
      sscanf (next_item (fp, token), "%s", Org[i]->Params.input_objects);

      /*  
	  regular expression filters:
          initial (j=0), operator (j=1), argument (j=2), result (j=3)
       */

      for (j = 0; j < 4; j++)
	{
	  sscanf (next_item (fp, token), "%d", &num);
	  
	  Org[i]->Filter.regexp[j].text = (char **) space (sizeof (char *) * (num + 1));
	  Org[i]->Filter.regexp[j].compiled = (regex_t **) space (sizeof (regex_t *) * (num + 1));
	  Org[i]->Filter.regexp[j].match = (int *) space (sizeof (int) * (num + 1));

	  for (k = 0; k < num; k++)
	    {
	      /* READ REGULAR EXPRESSION */

	      sscanf (next_item (fp, token), "%s %s", sign, buffer);
	      
	      if (sign[0] == '-')
		Org[i]->Filter.regexp[j].match[k] = 0;
	      else if (sign[0] == '+')
		Org[i]->Filter.regexp[j].match[k] = 1;
	      else
		nrerror ("read_parameters: unrecognized matching option");
		  
	      s = (char *) space (sizeof (char) * ((l = strlen (buffer)) + 1));
	      strcpy (s, buffer);
	      Org[i]->Filter.regexp[j].text[k] = s;

	      /* compile it; syntax option is GNU by default */

	      Org[i]->Filter.regexp[j].compiled[k] = (regex_t *) space (sizeof (regex_t));
	      Org[i]->Filter.regexp[j].compiled[k]->translate = NULL;
	      Org[i]->Filter.regexp[j].compiled[k]->fastmap = (char *) space (sizeof (char) * 256);;
	      Org[i]->Filter.regexp[j].compiled[k]->buffer = NULL;
	      Org[i]->Filter.regexp[j].compiled[k]->allocated = 0;

	      r = re_compile_pattern (Org[i]->Filter.regexp[j].text[k], l,
				      Org[i]->Filter.regexp[j].compiled[k]);
	      if (r)
		nrerror ((char *) r);
	    }
	}

      /* FUNCTIONAL FILTERS */
      
      sscanf (next_item (fp, token), "%f", &Org[i]->Filter.fun.copy_prob);

      /* INTERACTION LAWS */

      sscanf (next_item (fp, token), "%d", &Org[i]->Params.num_laws);

      Org[i]->Law = (law *) space (sizeof (law) * Org[i]->Params.num_laws);
      for (j = 0; j < Org[i]->Params.num_laws; j++)
	{
	  sscanf (next_item (fp, token), "%s", buffer);
	  s = (char *) space (sizeof (char) * (strlen (buffer) + 1));
	  strcpy (s, buffer);
	  Org[i]->Law[j].expr = s;
	  sscanf (next_item (fp, token), "%f", &Org[i]->Law[j].prob);
	}
    }

  fclose (fp);

  /* LOG EVERYTHING */

  fp = fopen (fn, "r");
  file_copy (fp, S->logfp);
  fclose (fp);
  fflush (S->logfp);
}

/*----------------------------------------------------------------------*/

/* re-reads an input file */

PUBLIC void
change_parameters (char *fn, simulation ** S_,
		  parmsLambda ** Lambda_parameters_,
		  parmsRandExpr ** RandExpr_parameters_)
{
  int i, j, k, l, num, dummy;
  long ldummy;
  char *s, sign[2];
  const char *r;
  char token[300], buffer[BUFSIZE];
  FILE *fp, *fopen ();
  
  simulation *S;
  parmsLambda *Lambda_parameters;
  parmsRandExpr *RandExpr_parameters;
  organization ** Org;
  
  S = *S_ ;
  
  fprintf(S->logfp, "parameters changed: %s\n", time_stamp());
  fclose(S->logfp);
  
  fp = fopen (fn, "r");
  
  system ("clear");
  printf ("re-reading parameters from %s\n", fn);
  
  /* SIMULATION PARAMETERS *********************************************/

  sscanf (next_item (fp, token), "%s", buffer);
  
  if (str_index (S->name, buffer) != -1)
    strcat (S->name, "^");
  else 
    strcpy (S->name, buffer);

  strcpy (buffer, S->name);
  strcat (buffer, ".log");
  S->logfp = fopen (buffer, "w");
  
  sscanf (next_item (fp, token), "%s", buffer);
  sscanf (next_item (fp, token), "%s", buffer);
  sscanf (next_item (fp, token), "%d", &S->type_save);
  sscanf (next_item (fp, token), "%d", &dummy); /* no change permitted */
  sscanf (next_item (fp, token), "%d", &S->max_objects);
  sscanf (next_item (fp, token), "%d", &S->collisions);
  sscanf (next_item (fp, token), "%d", &S->snapshot);
  sscanf (next_item (fp, token), "%ld", &ldummy);

  /* LAMBDA REDUCTION PARAMETERS ***************************************/

  Lambda_parameters = *Lambda_parameters_;
  
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->heap_size);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->cycle_limit);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->symbol_table_size);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->stack_size);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->name_length);
  sscanf (next_item (fp, token), "%d", &Lambda_parameters->type_length);
  sscanf (next_item (fp, token), "%s", buffer);
  Lambda_parameters->standard_variable = buffer[0];
  sscanf (next_item (fp, token), "%s", buffer);
  
  if (Lambda_parameters->error_fp && Lambda_parameters->error_fp != stdout) 
    fclose (Lambda_parameters->error_fp);
      
  if (strcmp (buffer, "NULL") == 0)
    Lambda_parameters->error_fp = NULL;
  else if (strcmp (buffer, "stdout") == 0)
    Lambda_parameters->error_fp = stdout;
  else
    Lambda_parameters->error_fp = fopen (buffer, "w");

  /* RANDOM EXPRESSION GENERATOR PARAMETERS ****************************/

  RandExpr_parameters = *RandExpr_parameters_;
  
  sscanf (next_item (fp, token), "%d", &RandExpr_parameters->max_depth);
  sscanf (next_item (fp, token), "%d", &RandExpr_parameters->size_alpha);
  sscanf (next_item (fp, token), "%d", &RandExpr_parameters->bind);
  sscanf (next_item (fp, token), "%f %f", &RandExpr_parameters->appl[0],
	  &RandExpr_parameters->appl[1]);
  sscanf (next_item (fp, token), "%f %f", &RandExpr_parameters->abst[0],
	  &RandExpr_parameters->abst[1]);

  /* sanity check on probabilities */

  if ((RandExpr_parameters->appl[0]
       + RandExpr_parameters->abst[0]) >= 1.
      || (RandExpr_parameters->appl[1]
	  + RandExpr_parameters->abst[1]) >= 1.)
    nrerror ("random expression generator probabilities are phony");

  /* ORGANIZATION PARAMETERS *******************************************/
  
  Org = S->Org;
  
  for (i = 0; i < S->systems; i++)
    {
      /* initial objects file */
      
      sscanf (next_item (fp, token), "%s", buffer); /* not used */

      /*  
	  REGULAR EXPRESSION FILTERS:
          initial (j=0), operator (j=1), argument (j=2), result (j=3)
       */

      for (j = 0; j < 4; j++)
	{
	  /* get rid of old stuff */
	  
	  l = -1;
	  while (Org[i]->Filter.regexp[j].text[++l])
	    {
	      free (Org[i]->Filter.regexp[j].text[l]);
	      free (Org[i]->Filter.regexp[j].compiled[l]);
	    }
	  free (Org[i]->Filter.regexp[j].text);
	  free (Org[i]->Filter.regexp[j].compiled);
	  free (Org[i]->Filter.regexp[j].match);
	  
	  sscanf (next_item (fp, token), "%d", &num);
	  
	  Org[i]->Filter.regexp[j].text = (char **) space (sizeof (char *) * (num + 1));
	  Org[i]->Filter.regexp[j].compiled = (regex_t **) space (sizeof (regex_t *) * (num + 1));
	  Org[i]->Filter.regexp[j].match = (int *) space (sizeof (int) * (num + 1));

	  for (k = 0; k < num; k++)
	    {
	      /* read regular expression */

	      sscanf (next_item (fp, token), "%s %s", sign, buffer);
	      
	      if (sign[0] == '-')
		Org[i]->Filter.regexp[j].match[k] = 0;
	      else if (sign[0] == '+')
		Org[i]->Filter.regexp[j].match[k] = 1;
	      else
		nrerror ("read_parameters: unrecognized matching option");
		  
	      s = (char *) space (sizeof (char) * ((l = strlen (buffer)) + 1));
	      strcpy (s, buffer);
	      Org[i]->Filter.regexp[j].text[k] = s;

	      /* compile it; syntax option is GNU by default */

	      Org[i]->Filter.regexp[j].compiled[k] = (regex_t *) space (sizeof (regex_t));
	      Org[i]->Filter.regexp[j].compiled[k]->translate = NULL;
	      Org[i]->Filter.regexp[j].compiled[k]->fastmap = (char *) space (sizeof (char) * 256);;
	      Org[i]->Filter.regexp[j].compiled[k]->buffer = NULL;
	      Org[i]->Filter.regexp[j].compiled[k]->allocated = 0;

	      r = re_compile_pattern (Org[i]->Filter.regexp[j].text[k], l,
				      Org[i]->Filter.regexp[j].compiled[k]);
	      if (r)
		nrerror ((char *) r);
	    }
	}

      /* FUNCTIONAL FILTERS */
      
      sscanf (next_item (fp, token), "%f", &Org[i]->Filter.fun.copy_prob);

      /* INTERACTION LAWS */
      
      /* get rid of old stuff */
      
      for (l = 0; l < Org[i]->Params.num_laws; l++)
	free (Org[i]->Law[l].expr);
      free (Org[i]->Law);
	
      sscanf (next_item (fp, token), "%d", &Org[i]->Params.num_laws);

      Org[i]->Law = (law *) space (sizeof (law) * Org[i]->Params.num_laws);
      for (j = 0; j < Org[i]->Params.num_laws; j++)
	{
	  sscanf (next_item (fp, token), "%s", buffer);
	  s = (char *) space (sizeof (char) * (strlen (buffer) + 1));
	  strcpy (s, buffer);
	  Org[i]->Law[j].expr = s;
	  sscanf (next_item (fp, token), "%f", &Org[i]->Law[j].prob);
	}
    }

  fclose (fp);

  /* LOG EVERYTHING */

  fp = fopen (fn, "r");
  file_copy (fp, S->logfp);
  fclose (fp);
  fflush (S->logfp);
}

/*----------------------------------------------------------------------*/

PUBLIC void
print_parameters (FILE *fp, simulation * S,
		  parmsLambda * Lambda_parameters)
{
  int i, j, k, l;
  
  system ("clear");
  fprintf (fp, "\nstatus after %d collisions\n\n", S->bangs);
  
  /* SIMULATION PARAMETERS *********************************************/

  fprintf(fp, "name: %s\n", S->name);

  if (S->Reaction == ORIGINAL)
    fprintf(fp, "reaction: ORIGINAL\n");
  if (S->Reaction == MAVELLI_MAESTRO)
    fprintf(fp, "reaction: MAVELLI/MAESTRO\n");
   
  fprintf(fp, "type synthesis: %d\n", S->type_check);
  fprintf(fp, "save types: %d\n", S->type_save);
  fprintf(fp, "systems: %d\n", S->systems);
  fprintf(fp, "maximum number of objects: %d\n", S->max_objects);
  fprintf(fp, "collisions wanted: %d\n", S->collisions);
  fprintf(fp, "snapshot interval: %d\n", S->snapshot);
  fprintf(fp, "seed (initial): %ld\n", S->seed);
  
  if (S->TypeBasis)
    {
      fprintf (fp, "\ntyping basis\n\n");
      l = -1;
      while (S->TypeBasis[++l].name)
	{
	  fprintf (fp, "name: %s\n", S->TypeBasis[l].name);
	  fprintf (fp, "type: %s\n", S->TypeBasis[l].type);
	}
      if (l == 0) 
	fprintf (fp, "none\n");
    }
  fprintf(fp, "\n");
    
  fprintf(fp, "current collision: %d\n", S->bangs);
  fprintf(fp, "current number of objects: %d\n", S->num_objects);
  fprintf(fp, "cumulative innovations: %d\n", S->new);
  fprintf(fp, "rate of innovation: %f\n", S->new_rate);
  fprintf(fp, "cumulative type clashes: %d\n", S->typeclashes);
  fprintf(fp, "rate of type clashes: %f\n", S->clashes_rate);
  
  /* LAMBDA REDUCTION PARAMETERS ***************************************/

  fprintf(fp, "\nLAMBDA\n\n");

  fprintf(fp, "heap size: %d\n", Lambda_parameters->heap_size);
  fprintf(fp, "reduction limit: %d\n", Lambda_parameters->cycle_limit);
  fprintf(fp, "symbol table size: %d\n", Lambda_parameters->symbol_table_size);
  fprintf(fp, "stack size: %d\n", Lambda_parameters->stack_size);

  /* ORGANIZATION PARAMETERS *******************************************/

  for (i = 0; i < S->systems; i++)
    {
      fprintf(fp, "\nSYSTEM %d\n\n", i);
	
      /* REGULAR EXPRESSION FILTERS */

      for (j = 0; j < 4; j++)
	{
	  fprintf(fp, "Regex, context %d\n", j);
	  l = -1;
	  while (S->Org[i]->Filter.regexp[j].text[++l])
	    {
	      fprintf(fp, " regex: (%d) %s\n", 
		      S->Org[i]->Filter.regexp[j].match[l],
		      S->Org[i]->Filter.regexp[j].text[l]);
	    }
	  if (l == 0) 
	    fprintf (fp, "none\n\n");
	
	}

      /* FUNCTIONAL FILTERS */
      
      fprintf(fp, "\n");
      fprintf(fp, "copy acceptance: %f\n", S->Org[i]->Filter.fun.copy_prob);

      /* INTERACTION LAWS */

      fprintf(fp, "\n");
      for (j = 0; j < S->Org[i]->Params.num_laws; j++)
	{
	  fprintf(fp, "law %d: %s\n", j, S->Org[i]->Law[j].expr);
	}
      fprintf(fp, "\n");
    }
    fflush (fp);
}

/*----------------------------------------------------------------------*/

PUBLIC void
read_basis (char *fname, simulation * S)
{
  int n, t, j;
  char *line, *temp;
  char name[50];
  FILE *fopen (), *in;

  n = lines_in_file (fname);
  
  /* read types from file */
      
  in = fopen (fname, "r");
  printf("reading basis file %s ...\n", fname);
  
  S->TypeBasis = (basis *) space (sizeof (basis) * (n + 1));
  
  t = 0;
  while ( (temp = get_line (in)) )
    {
      line = temp;
      j = 0;
      while (*line != ' ') name[j++] = *line++;
      name[j] = '\0';
      
      while (*line == ' ' || *line == ':') line++;
      
      S->TypeBasis[t].name = (char *) space (strlen (name) + 1);
      strcpy (S->TypeBasis[t].name, name);
      S->TypeBasis[t].type = (char *) space (strlen (line) + 1);
      strcpy (S->TypeBasis[t].type, line);
      
      free (temp);
      t++;
    }
  fclose (in);
}

/*----------------------------------------------------------------------*/

PUBLIC void
get_expressions (simulation * S, parmsRandExpr * RandExpr,
		 interpreter * Lambda)
{
  FILE *fopen (), *in;
  char *A, *B, *sigma, *line, buffer[BUFSIZE];
  int i, j, num, copies, found, got_types;
  int scanned, accepted;
  REC *node;

  for (i = 0; i < S->systems; i++)
    {
      if (str_index (S->Org[i]->Params.input_objects, "NULL") == -1)
	{
	  /* read objects from file */
	  
	  in = fopen (S->Org[i]->Params.input_objects, "r");
	  if (!in)
	    {
	      printf ("get_expressions: file %s does not exist\n", 
		      S->Org[i]->Params.input_objects);
	      exit (-1);
	    }
	  printf("reading file %s ...\n", S->Org[i]->Params.input_objects);
	  scanned = accepted = 0;
	  
	  if (str_index (S->Org[i]->Params.input_objects, ".typ") != -1)
	    got_types = 1;
	  else 
	    got_types = 0;

	  while ( (line = get_line (in)) ) 
	    {
	      scanned++;
	      sscanf (line, "%s", buffer);
	      B = (char *) space (sizeof (char) * (strlen (buffer) + 1));
	      strcpy (B, buffer);
	      
              A = expand_annotation (B);
	      free (B);
	      
	      if (got_types)		  /* developer: sigma contains blanks */
		  sigma = get_line (in);  /* caution when making a sscanf */
	      
	      if (syntax_sieve (A, S->Org[i]->Filter.regexp[INITIAL], S->type_check))
		{
		  if ( (j = str_index (line, "{")) >= 0)
		    sscanf (line + j + 1, "%d %d", &num, &copies);
		  else
		    copies = 1;

		  if (!got_types)
		    {
		      if (S->type_check) 
			sigma = type_synthesis (A);
		      else
			sigma = buffer;  /* set to non-NULL */
		    }

		  if (sigma)
		    {
		      node = add_to_system (i, A, S, 0);
		      if (S->type_check && S->type_save)
			{
			  node->type = (char *) space (strlen(sigma) + 1);
			  strcpy (node->type, sigma);
			}
		      node->instances += (copies-1);
		      S->Org[i]->State.num_objects += (copies-1);
		      S->num_objects += (copies-1);
		  
		      accepted++;
		    }
		  else
		    free (A);
		}
	      else
		free (A);
	      
	      if (got_types)
		free (sigma);
	      free (line);
	    }
	  printf("%d objects scanned, %d accepted\n", scanned, accepted);
	  fclose (in);
	}
      else 
	{
	  /* invoke random object generator */
	  
	  sscanf (S->Org[i]->Params.input_objects, "NULL-%d", &num);
	  printf ("generating %d random expressions...\n", num);
	  for (j = 0; j < num; j++)
	    {
	      A = random_expression (RandExpr, S->TypeBasis, Lambda);
	      
	      if (syntax_sieve (A, S->Org[i]->Filter.regexp[INITIAL], S->type_check))
		{
		  if (!AVL (FIND, S->Org[i]->Objects, A, &found))
		    {
		      if (S->type_check)
			sigma = type_synthesis (A);
		      else
			sigma = buffer;
		      
		      if (sigma)
			{
			  node = add_to_system (i, A, S, 0);
			  if (S->type_check && S->type_save)
			    {
			      node->type = (char *) space (strlen(sigma) + 1);
			      strcpy (node->type, sigma);
			    }
			  if (!((j+1) % 100))
			    printf("%d...\n", j+1);
			  continue;
			}
		    }
		}
	      free (A);
	      j--;
	    }
	}
    }
    printf ("Done.\n");
}

/*----------------------------------------------------------------------*/

PRIVATE char *
next_item (FILE * fp, char *token)
{
  int s;
  char *line;

  if ((line = get_line (fp)) == NULL)
    return NULL;

  /* skip lines until we get one containing a SEPARATOR */

  while ((s = str_index (line, SEPARATOR)) == -1)
    {
      free (line);
      if ((line = get_line (fp)) == NULL)
	return NULL;
    }
  strcpy (token, line + s + 1);
  free (line);

  return token;
}

/*--------------------------------------------------------------------------*/

PUBLIC void
print_reactor (char *fname, simulation * S)
{
  int i;
  FILE *fopen ();
  
  OUT_FP = fopen (fname, "w");
  for (i = 0; i < S->systems; i++) 
    {
      COUNT = 0;
      traverse_AVL (S->Org[i]->Objects, print_nodes);
    }
  fclose (OUT_FP);
}

/*--------------------------------------------------------------------------*/

/* traversal routine */

PRIVATE void
print_nodes (void *params, REC * node)
{
  int i, j, l;
  char *A;

  COUNT++;

  switch (FORMAT)
    {
    case 1:  /* visual display */

      fprintf (OUT_FP, "(#%4d) %4d %4d\n", COUNT, node->instances, node->sysid);
      l = strlen (node->object);
      fprintf (OUT_FP, "         ");
      for (i = 0; i < l / WIDTH; i++)
	{
	  for (j = i * WIDTH; j < (i + 1) * WIDTH; j++)
	    fprintf (OUT_FP, "%c", node->object[j]);
	  fprintf (OUT_FP, "\n");
	  fprintf (OUT_FP, "         ");
	}
      fprintf (OUT_FP, "%s\n", (node->object) + ((l / WIDTH) * WIDTH));
      break;

    default:  /* standard format */

      A = minimize_annotation (node->object);
      fprintf (OUT_FP, "%s {%d %d %d}\n", A, COUNT, node->instances,
					  node->sysid);
      if (node->type) fprintf (OUT_FP, "%s\n", node->type);
      free (A);
      break;
    }
}

/*======================================================================*/

#ifdef IO_TEST

int
main (int argc, char **argv)
{
  simulation *S;
  interpreter *L;
  parmsLambda *L_parameters;
  parmsRandExpr *RandEx_parameters;

  set_parameters ("alchemy.inp", &S, &L_parameters, &RandEx_parameters);
  L = initialize_lambda (L_parameters);
  get_expressions (S, RandEx_parameters, L);

  strcpy (name, S->name);
  strcat (name, ".out");
  print_reactor (name, S);

  return (0);
}

#endif /* IO_TEST */
