/*
   peephole.c

   c 1994 Walter Fontana  
 */

#include <stdio.h>
#include <stdlib.h>
#include "peephole.h"
#include "socket.h"

PUBLIC int peephole (int port, access * Data);

/*----------------------------------------------------------------------*/

PUBLIC int 
peephole (int port, access * Data)
{
  static int connection = 0;
  static int flag = 0;
  
  if ((connection = CheckForClientCall ((u_short) port)) > 0)
    {
      /* send as first, then be passive! */

      if (!flag)
	{
	  Send (connection, Data, sizeof (access));
	  flag = 1;
	}
      if (CheckForData (connection, Data, sizeof (access)) > 0)
	{
	  Send (connection, Data, sizeof (access));

	  switch (Data->ctrl)
	    {
	    case -1:
	      HangUp (connection);
	      flag = 0;
	      break;
	    case -9:
	      HangUp (connection);
	      KillReceiver ((u_short) port);
	      break;
	    }
	  return (1);
	}
    }
  return (0);
}

/*----------------------------------------------------------------------*/
