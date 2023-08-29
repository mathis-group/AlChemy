/*
    randomexpr.h
    
    c 1994 Walter Fontana
 */

#ifndef	__RANDOMEXPR_H
#define	__RANDOMEXPR_H

#include "lambda.h"
#include "conversions.h"

typedef struct parmsRandExpr
  {
    int max_depth;		/* random expression depth */
    int size_alpha;		/* size of var alphabet */
    boolean bind;		/* bind free vars */
    float appl[2];		/* probability for application */
    float abst[2];		/* probability for abstraction */
  }
parmsRandExpr;

extern char *random_expression(parmsRandExpr *Params, basis *Basis, 
                               interpreter *Lambda);

#endif  /* __RANDOMEXPR_H */
