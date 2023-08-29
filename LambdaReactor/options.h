/*
    options.h

    c 1994 Walter Fontana
 */
      
#ifndef	__OPTIONS_H
#define	__OPTIONS_H

#include "getopt.h"

/* Option type sepecifiers */

#define	OPT_INTEGER	'd'
#define	OPT_HEX		'h'
#define	OPT_OCTAL	'o'
#define	OPT_UNSIGNED	'u'
#define	OPT_LINTEGER	'D'
#define	OPT_LHEX	'H'
#define	OPT_LOCTAL	'O'
#define	OPT_LUNSIGNED	'U'
#define	OPT_FLOAT	'f'
#define	OPT_DOUBLE	'F'
#define	OPT_LDOUBLE	'L'
#define	OPT_STRING	's'
#define	OPT_SWITCH	'!'

typedef unsigned char uchar; 

typedef struct
  {
    char *name;			/* The option identifier                */
    uchar type;			/* Type descriptor for the option       */
    void *arg;			/* Place to store the argument          */
    char *desc;			/* Description for this option          */
  }
Option;

extern void command_line_args (int argc, char *argv[], Option option[]);

#endif
