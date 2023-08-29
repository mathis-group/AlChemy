/*  
    peephole.h
    
    c 1994 Walter Fontana
 */
 
#ifndef	__PEEP_H
#define	__PEEP_H

#include "structs.h"

typedef struct access
  {
    int ctrl;
    int show;
    char message[100];
  }
access;

extern int peephole (int port, access * Data);

#endif  /* __PEEP_H */
