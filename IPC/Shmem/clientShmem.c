/*
   client example (shared memory)

   c 1995 Walter Fontana
 */

#include <stdlib.h>
#include <stdio.h>
#include "include.h"
#include "utilities.h"
#include "shmem.h"

/*--------------------------------------------------------------------------*/

main ()
{
  char *text, *result;

  attach_shared_memory ();

  while (TRUE)
    {
      printf ("input text (* to hang up and @ to kill server)\n");
      text = get_line (stdin);
      if (*text == '*' || *text == '@')
	{
	  shmem_SendData (text, '?');
          detach_shared_memory ();
	  free (text);
	  exit (0);
	}
      result = shmem_GetService (text);
      printf ("result: %s\n", result);
      free (text);
    }
}

/*--------------------------------------------------------------------------*/
