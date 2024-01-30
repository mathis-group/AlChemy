/*
   server example (shared memory)

   c 1995 Walter Fontana
 */

#include <stdio.h>
#include "include.h"
#include "shmem.h"

/*--------------------------------------------------------------------------*/

main ()
{
  char *text;

  create_shared_memory ();
  attach_shared_memory ();

  while (TRUE)
    {
      text = shmem_GetData ();
      
      /* do something with the data */

      printf ("received: %s\n", text);
      
      shmem_SendData (text, '!');
    }
}

/*--------------------------------------------------------------------------*/
