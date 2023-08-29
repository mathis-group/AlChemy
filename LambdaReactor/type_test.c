/*
   client.c

   c 1994 Walter Fontana
 */

#include <stdio.h>
#include "include.h"
#include "utilities.h"
#include "socket.h"
#include <stdlib.h>

PUBLIC char * AskForPlace (unsigned short *port);

/*--------------------------------------------------------------------------*/

main ()
{
  char *machine, *text, *result;
  int connection;
  unsigned short port;

  machine = AskForPlace (&port);
  connection = DialServer (machine, port); 
  if (machine) free (machine);
  
  if (!connection)
    {
      printf ("no connection\n");
      exit (0);
    }
    
  while (TRUE)
    {
      printf("input text (* to hang up and @ to kill server)\n");
      text = get_line (stdin);
      if (*text == '*' || *text == '@')
	{
	  SendData (connection, text);
	  HangUp (connection);
	  free (text);
	  exit (0);
	}
      result = GetService (connection, text);
      printf("type: %s\n", result);
      free (text);
    }
}

/*--------------------------------------------------------------------------*/

PUBLIC char *
AskForPlace (unsigned short *port)
{
  char *text, *machine;

  printf ("enter machine where server is running (return if same as client)\n");
  machine = get_line (stdin);
  if (*machine == '\0')
    {
      free (machine);
      machine = NULL;
    }

  printf ("enter port number (return if 5001)\n");
  text = get_line (stdin);
  if (*text == '\0')
      *port = 5001;   /* default port */
  else
      sscanf (text, "%u", port);
  free (text);

  return machine;
}

