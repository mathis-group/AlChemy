/* 
    interact.h
    
    c 1994 Walter Fontana
 */

#ifndef	__INTERACT_H
#define	__INTERACT_H

#define ORIGINAL	1
#define MAVELLI_MAESTRO 2

#define PRODUCTS        5  /* enough for any model ... */

#include "structs.h"

typedef struct _Action
  {
    REC *f[2];		/* the two objects that act upon each other */
    char *action[2];	/* their action according to the lambda way */
  }
Action;

typedef struct react
  {
    char *product[PRODUCTS];  /* the products of the reaction scheme */
  }
react;


extern react *Reaction (REC * A, REC * B, simulation * S, interpreter * L, int show);
extern char *pure_lambda_action (char *A, char *B, char *Law, interpreter * L);

#endif /* __INTERACT_H */
