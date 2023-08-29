/*
   main.c

   c 1994,1995 Walter Fontana  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "avlaccess.h"
#include "randomexpr.h"
#include "reactor.h"
#include "interact.h"
#include "options.h"
#include "peephole.h"
#include "io.h"
#include "structs.h"
#include "type_client.h"

PRIVATE void clean_up (simulation *S, interpreter *L);
PRIVATE void update (simulation *S);
PRIVATE void peep(access * Peep, simulation ** S, parmsLambda ** L_par, 
		    interpreter **L, parmsRandExpr ** RandEx_par);
PRIVATE void collide (simulation * S, interpreter * L);
PRIVATE void L1collide (simulation * S, interpreter * L);

/*=======================================================================*/

/* command line options */

PRIVATE char *IN_FILE  = "alchemy.inp";
PRIVATE char *TYPER    = "gadget";
PRIVATE int TYPER_PORT = 5001;
PRIVATE int PEEP_PORT  = 5000;
PRIVATE int WINDOW     = 100;
PRIVATE int ALL        = 0;
PRIVATE int PAIRINT    = 0;
PUBLIC  int SHRDMEM    = 0;

PRIVATE Option option[] =
{
/*  option name   type descriptor   variable       description  */
/* (uchar name)   (uchar type)     (void *arg)     (char *desc) */

  {"f", OPT_STRING, &IN_FILE, "input filename"},
  {"typer", OPT_STRING, &TYPER, "type server machine address"},
  {"port", OPT_INTEGER, &TYPER_PORT, "port for type service"},
  {"peep", OPT_INTEGER, &PEEP_PORT, "port for peep service"},
  {"w", OPT_INTEGER, &WINDOW, "window for rate stats"},
  {"shared", OPT_SWITCH, &SHRDMEM, "use shared memory for ipc"},
  {"p", OPT_SWITCH, &PAIRINT, "pair interactions only"},
  {"a", OPT_SWITCH, &ALL, "report all actions in pair int"},
  {0, 0, 0, 0}
};

/*=======================================================================*/

PRIVATE access * Peep;

/*=======================================================================*/

PUBLIC int
main (int argc, char **argv)
{
  simulation *S;
  interpreter *L;
  parmsLambda *L_par;
  parmsRandExpr *RandEx_par;
  char num[10], name[100];
  int dump = 0;
  
  command_line_args (argc, argv, option);

  /*- INITIALIZE STUFF --------------------------------------*/

  set_parameters (IN_FILE, &S, &L_par, &RandEx_par);
  L = initialize_lambda (L_par);
  initialize_socket (PEEP_PORT);                   /* we are server */
  if (S->type_check)
    initialize_type_synthesis (TYPER, TYPER_PORT); /* we are client */
  get_expressions (S, RandEx_par, L);

  Peep = (access *) space (sizeof (access));
  Peep->ctrl = 1;

  /*- if requested, do the pair interactions only -----------*/

  if (PAIRINT)
    {
      pair_interactions (S, L, ALL);
      clean_up (S, L);
      return 0;
    }

  /*- MAIN LOOP ---------------------------------------------*/
  /*---------------------------------------------------------*/

  for (S->bangs = 0; S->bangs <= S->collisions;)
    {
      while (!Peep->ctrl)    /* waiting loop */
	while (!peephole (PEEP_PORT, Peep));

      /*- SNAPSHOT ------------------------------------------*/

      if (S->bangs % S->snapshot == 0)
	{
	  strcpy (name, S->name);
	  strcat (name, int_to_char (dump, num));
	  print_reactor (name, S);
	  dump++;
	}

      L1collide (S, L);

      update (S);
      
      /* IPC PEEPHOLE ----------------------------------------*/

      if (peephole (PEEP_PORT, Peep))
	peep(Peep, &S, &L_par, &L, &RandEx_par);
    }

  /*---------------------------------------------------------*/
  /*- CLEAN UP -----------------------------------------------*/

  clean_up (S, L);
}

/*=======================================================================*/

PRIVATE void
update (simulation *S)
{
  static int previous_new = 0;
  static int previous_clashes = 0;
  static int c = 0;

  S->bangs++;
      
  if (++c == WINDOW)
    {
      S->new_rate = (S->new - previous_new) / (float) c;
      S->clashes_rate = (S->typeclashes - previous_clashes) / (float) c;
      previous_new = S->new;
      previous_clashes = S->typeclashes;
      c = 0;
    }
}

/*-----------------------------------------------------------------------*/

PRIVATE void
clean_up (simulation *S, interpreter *L)
{
  fprintf (S->logfp, "\nsimulation terminated: %s\n\n", time_stamp ());
  print_parameters(S->logfp, S, L->parms);
  fprintf (S->logfp, "\nlambda reducer status\n\n");
  lambda_status (S->logfp);
  fclose (S->logfp);
  
  if (S->type_check)
    ctl_type_synthesis ("@");  /* shut down typer */
}  

/*-----------------------------------------------------------------------*/

PRIVATE void
peep(access * Peep, simulation ** S, parmsLambda ** L_par, interpreter **L,
       parmsRandExpr ** RandEx_par)
{
  static int snap = 1;
  char name[100], num[10];
  
  switch (Peep->ctrl)
    {
    
    case 2:		/* re-read parameters */
    
      change_parameters (Peep->message, S, L_par, RandEx_par);
      free_interpreter (*L);
      *L = initialize_lambda (*L_par);

      Peep->ctrl = 1;
      break;

    case 3:		/* show parameters */
    
      print_parameters (stdout, *S, *L_par);
      Peep->ctrl = 1;
      break;
      
    case 4:		/* show lambda status */
    
      lambda_status (stdout);
      Peep->ctrl = 1;
      break;
      
    case 5:		/* snapshot */
    
      strcpy (name, (*S)->name);
      strcat (name, ".snap");
      strcat (name, int_to_char (snap, num));
      print_reactor (name, *S);
      snap++;
      Peep->ctrl = 1;
      break;
      
    case -9:		/* kill */

      clean_up (*S, *L);
      exit (0);
      break;

    default:
    
      break;
    }
  return;
}

/*-----------------------------------------------------------------------*/

/* this one simply defines the mass action kinetics */
/* version for multiple L1s */

PRIVATE void
collide (simulation * S, interpreter * L)
{
  int sys[10];
  int i, n, r, glue;
  int a, b;
  char *p;
  REC *A;
  REC *B;
  react *reaction;

/*- choose two objects at random --------------------------*/

  a = choose_system (S);
  A = choose_from_system (a, S);

  do				/* avoid physical self-collision */
    {
      b = choose_system (S);
      B = choose_from_system (b, S);
    }
  while (strcmp (A->object, B->object) == 0 && A->instances == 1);

  /* - make them react --------------------------------------- */

  reaction = Reaction (A, B, S, L, Peep->show);

  /* - add the products to the reactor ----------------------- */

  if (reaction)
    {
      i = -1;
      while (p = reaction->product[++i])
	{
	  n = which_systems (p, sys, S);
	  if (n == 0)
	    {
	      glue = S->systems - 1;	/* add to the "glue" */
	      add_to_system (glue, p, S, (S->type_check && S->type_save));
	    }
	  else
	    {
	      r = int_urn (0, n - 1);
	      add_to_system (sys[r], p, S, (S->type_check && S->type_save));
	    }
	}
      /* 
	 If products were already in the system, "product" has been
	 freed in add_to_system. All that remains is to free the "reaction"-box.
       */

      free (reaction);
    }
}

/*----------------------------------------------------------------------*/

/* this one simply defines the mass action kinetics */
/* simplified version for one L1 only */

PRIVATE void
L1collide (simulation * S, interpreter * L)
{
  int i;
  char *p;
  REC *A;
  REC *B;
  react *reaction;

/*- choose two objects at random --------------------------*/

  A = choose_from_system (0, S);

  do				/* avoid physical self-collision */
    {
      B = choose_from_system (0, S);
    }
  while (strcmp (A->object, B->object) == 0 && A->instances == 1);

  /* - make them react --------------------------------------- */

  reaction = Reaction (A, B, S, L, Peep->show);

  /* - add the products to the reactor ----------------------- */
  
  if (reaction)
    {
      i = -1;
      while (p = reaction->product[++i]) 
	add_to_system (0, p, S, (S->type_check && S->type_save));
      free (reaction);
    }
}

/*----------------------------------------------------------------------*/






