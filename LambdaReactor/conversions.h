/* 
    conversions.h 

    c 1995 Walter Fontana 
 */

#ifndef	__CONVERSIONS_H
#define	__CONVERSIONS_H

#include "structs.h"

extern char * fast_standardize (char *expression, interpreter *Interp);
extern char * annotate (char *expression, basis *Basis);
extern char * mini_annotate (char *expression, basis * Basis);
extern char * minimize_annotation (char *expression);
extern char * strip_annotation (char *expression);
extern char * expand_annotation (char *expression);

#endif /* __CONVERSIONS_H */
