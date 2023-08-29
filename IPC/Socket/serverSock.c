/*
   server example (sockets)

   c 1995 Walter Fontana
 */

#include <stdlib.h>
#include <stdio.h>
#include "include.h"
#include "utilities.h"
#include "socket.h"

PUBLIC void AskForPort (unsigned short *port);

/*--------------------------------------------------------------------------*/

main ()
{
  unsigned short port;
  int connection;
  char *text;

  AskForPort (&port);
  initialize_socket (port);

  while (TRUE)
    {
      text = GetData (&connection, port);
      
      /* do something with the data */

      printf ("received: %s\n", text);
      
      SendData (connection, text);
    }
}

/*--------------------------------------------------------------------------*/

PUBLIC void 
AskForPort (unsigned short *port)
{
  char *text;

  printf ("enter port number (return if 5000)\n");
  text = get_line (stdin);
  if (*text == '\0')
      *port = 5000;   /* default port */
  else
      sscanf (text, "%u", port);
  free (text);
}
