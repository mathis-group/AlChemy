/*
   peep.c

   c 1994 Walter Fontana
 */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "include.h"
#include "utilities.h"
#include "peephole.h"
#include <string.h>

PRIVATE void talk ();
PRIVATE void show_options ();

/*--------------------------------------------------------------------------*/

PRIVATE void 
talk ()
{
  char *line;
  char *machine;
  int connection;
  int code;
  int value;
  access *Data;
  unsigned short port;

  Data = (access *) space (sizeof (access));

  printf ("enter machine to connect to (return if same as caller)\n");
  line = get_line (stdin);
  if (*line == '\0')
    {
      machine = NULL;
      free (line);
    }
  else
      machine = line;

  printf ("enter port number (return if 5000)\n");
  line = get_line (stdin);
  if (*line == '\0')
      port = 5000;
  else
      sscanf (line, "%u", port);
  free (line);

  while (TRUE)
    {
      if ((connection = DialServer (machine, port)) > 0)
	{
	  system ("clear");
	  while (CheckForData (connection, Data, sizeof (access)) <= 0);
	  show_options (Data);

	  line = get_line (stdin);
	  if (*line == '\0')
	    {
	      Send (connection, Data, sizeof (access));
	      free (line);
	    }
	  else if (*line == '\\')
	    {
	      Data->ctrl = -1;
	      if ((connection = DialServer (machine, port)) > 0)
		Send (connection, Data, sizeof (access));
	      HangUp (connection);
	      free (line);
	      free (machine);
	      exit (0);
	    }
	  else
	    {
	      sscanf (line, "%d %d", &code, &value);

	      switch (code)
		{
		case 0:
		  if (value == 2)
		    {
		      printf ("re-read parameters? (y/n) [y]\n");
		      line = get_line (stdin);
		      if (!(*line == 'y' || *line == '\0'))
			{
			  free (line);
			  Data->ctrl = 1;
			  break;
			}
		      printf ("input filename [alchemy.inp]\n");
		      line = get_line (stdin);
		      if (*line == '\0')
			strcpy (Data->message, "alchemy.inp");
		      else
			strcpy (Data->message, line);
		      free (line);
		    }
		  Data->ctrl = value;
		  break;
		  
		case 1:
		  Data->show = value;
		  break;
		  
		default:
		  printf ("no such parameter\n");
		  break;
		}
	      free (line);

	      Send (connection, Data, sizeof (access));
	    }
	}
    }
}

/*--------------------------------------------------------------------------*/

PRIVATE void
show_options(access * Data)
{
  int i = 0;
  
  printf (">> %d << control variable\n", i++);
  printf ("         current value: %d\n", Data->ctrl);
  printf ("         options:\n");
  printf ("                  0 --> pause simulation\n");
  printf ("                  1 --> resume simulation\n");
  printf ("                  2 --> re-read parameters from input file\n");
  printf ("                  3 --> display current parameters\n");
  printf ("                  4 --> display lambda reducer status\n");
  printf ("                  5 --> dump reactor contents to file\n");
  printf ("                 -9 --> kill simulation\n");
  printf (">> %d << show reaction\n", i++);
  printf ("         current value: %d\n", Data->show);
  printf ("         options:\n");
  printf ("                  1 --> show on\n");
  printf ("                  0 --> show off :-)\n");
  printf ("\n==>input code and value or \\ to quit this process\n");
}  

/*--------------------------------------------------------------------------*/

int
main (int argc, char **argv)
{
  talk ();
}

