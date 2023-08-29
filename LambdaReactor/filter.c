/*
    filter.c    

    c 1994 Walter Fontana  
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "include.h"
#include "regex.h"
#include "conversions.h"
#include "filter.h"

PUBLIC int syntax_sieve (char *object, Regex Filter, int strip);

/*---------------------------------------------------------------------*/

/* apply lexical filters; return 1 if object is ok, 0 otherwise */

PUBLIC int
syntax_sieve (char *object, Regex Filter, int strip)
{
  int l, match;
  char *naked;
  register int i, rc;

  if (!object)
    return 0;  /* nothing from nothing */
  
  if (!Filter.compiled[0])  /* nothing to do */
    return 1;
  
  if (strip)
    naked = strip_annotation (object);
  else
    naked = object;

  l = strlen (naked);
  i = -1;
  while (Filter.compiled[++i])
    {
      rc = re_search (Filter.compiled[i], naked, l, 0, l, NULL);
      if (rc < 0) 
	match = 0;
      else 
	match = 1;
      if (match != Filter.match[i])
	{
	  if (strip) free (naked);
	  return 0;
	}
    }
  
  /* put here additional lexical filters */
  
  if (strip) free (naked);
    
  return 1;
}

/*---------------------------------------------------------------------*/
