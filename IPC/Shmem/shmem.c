/* 
   IPC via shared memory - basic toolbox

   c 1995 Walter Fontana, Vienna
 */

#include <stdlib.h>
#include <sys/ipc.h>
/* #include <sys/types.h>  SUN */
#include <sys/shm.h>
#include <stdio.h>
#include "include.h"
#include <string.h>

PRIVATE key_t key = 42;    /* it's the answer to a big question... */
PRIVATE int perm_flags = 0660;
PRIVATE int create_flags = 0660 | IPC_CREAT;
PRIVATE int size = 0x3000; /* 12288 bytes */
PRIVATE char *shmaddr;

PUBLIC int create_shared_memory ();
PUBLIC char * attach_shared_memory ();
PUBLIC void detach_shared_memory ();
PUBLIC void remove_shared_memory ();
PUBLIC char * shmem_GetService (char * text);
PUBLIC char * shmem_GetData ();
PUBLIC char * shmem_SendData (char * text, char sign);

PRIVATE char * shmem_Listen ();

/*------------------------------------------------------------------------*/

PUBLIC int
create_shared_memory ()
{
  int id;

  id = shmget (key, size, create_flags);
  if (id == -1) 
    {
      perror ("shared memory allocation failure"); 
      exit (1);
    }
  return id;
}

/*------------------------------------------------------------------------*/

PUBLIC char *
attach_shared_memory ()
{
  int id;

  id = shmget (key, size, perm_flags);
  
  if (id == -1)
    {
      perror ("failed getting shared memory id");
      exit (1);
    }
     
  shmaddr = shmat (id, 0, 0);
  if (shmaddr == (char *) -1)
    {
      perror ("failed attaching shared memory");
      exit (1);
    }
  return shmaddr;
}

/*------------------------------------------------------------------------*/

PUBLIC void
detach_shared_memory ()  /* wrapper */
{
  shmdt (shmaddr); 
  return;
}

/*------------------------------------------------------------------------*/

PUBLIC void
remove_shared_memory () 
{
  int id;

  id = shmget (key, 0, 0);
  shmctl (id, IPC_RMID, NULL);
  return;
}

/*==========================================================================
  high level calls for client/server
==========================================================================*/

PUBLIC char *
shmem_GetService (char * text)
{
  shmem_SendData (text, '?');
  return (shmem_Listen ());  
}

/*--------------------------------------------------------------------------*/

PUBLIC char * 
shmem_GetData ()
{
  while (TRUE) 
    {
      while (*shmaddr != '?');

      if (*(shmaddr+1) == '@')
	{
          detach_shared_memory ();
	  remove_shared_memory ();
          exit (0);
	}
      else
	{
	  return shmaddr+1;
	}
    }
}

/*--------------------------------------------------------------------------*/

PRIVATE char *
shmem_Listen ()
{
  while (*shmaddr != '!');
  return shmaddr+1;
}

/*--------------------------------------------------------------------------*/

PUBLIC char *
shmem_SendData (char * text, char sign)
{
  strcpy (shmaddr+1, text);   /* put data text into shared memory */
  *shmaddr = sign;            /* flag: '?' for client requests    */
                              /*       '!' for server answers     */
}
