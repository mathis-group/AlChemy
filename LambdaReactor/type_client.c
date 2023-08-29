/*
   type_client.c

   c 1995 Walter Fontana
 */

#include <stdio.h>
#include "include.h"
#include "utilities.h"
#include "socket.h"
#include "type_client.h"
#include "shmem.h"
#include <stdlib.h>

PUBLIC void initialize_type_synthesis (char *machine, int port);
PUBLIC void ctl_type_synthesis (char *ctl);
PUBLIC char *type_synthesis (char *object);

extern  int SHRDMEM;

PRIVATE int connection = 0;

/*--------------------------------------------------------------------------*/

PUBLIC void
initialize_type_synthesis (char *machine, int port)
{
  if (SHRDMEM)      /* shared memory ipc */
    {
      attach_shared_memory ();
    }
  else              /* socket ipc */
    {
      connection = DialServer (machine, (unsigned short) port); 
  
      if (!connection)
	{
	  printf ("no type server connection on %s at port %d\n", machine, port);
	  exit (0);
	}
      printf ("type server on %s at port %d ready.\n", machine, port);
      fflush (stdout);
    }
}

/*--------------------------------------------------------------------------*/

PUBLIC void
ctl_type_synthesis (char *ctl) 

/* ctl = "*" to hang up and ctl = "@" to shutdown server */

{
  if (SHRDMEM)          /* shared memory ipc */
    {
      shmem_SendData (ctl, '?');
      detach_shared_memory ();
    }
  else                  /* socket ipc */
    {
      SendData (connection, ctl);
      HangUp (connection);
    }
}
    
/*--------------------------------------------------------------------------*/

PUBLIC char *
type_synthesis (char *object)
{
  char *type;
  
  /* send object to type server and wait for result */
  
  if (SHRDMEM)
    type = shmem_GetService (object);         /* shared memory ipc */
  else
    type = GetService (connection, object);   /* socket ipc */

  /* 
      possible results: 
      type | IllegalChar... | TypingBug... | ParseErr | Clash... 
  */
  
  switch (*type)
    {
    case 'C':   /* type clash */
    
      return NULL;
      break;
      
    case 'I':   /* illegal character */
    case 'T':   /* problem in type synthesis */
    case 'P':   /* problem in parser */
    
      printf ("bug in type synthesis of object\n%s\n", object);
      printf ("call programmer\n");
      nrerror (type);
      break;
      
    default:    /* object has a type */
  
      return type;
      break;
    }
}

/*--------------------------------------------------------------------------*/
