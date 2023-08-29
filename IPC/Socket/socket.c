/* 
   IPC via socket - basic toolbox: socket part

   c 1994 Walter Fontana, Interval
   c 1995 Walter Fontana, Vienna
 */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include "include.h"
#include "socket.h"

#define MAKE_NONBLOCKING(fd) fcntl(fd, F_SETFL, O_NDELAY)

#define BASE	 5000  /* in applications choose port numbers >= BASE ! */
#define PORTS      10  /* max number of ports */
#define	MAXNAME   100

/* transmission part */

PUBLIC int initialize_socket (int port);
PUBLIC int CheckForClientCall (u_short port);
PUBLIC int CheckForData (int sock, void *data, unsigned int size);
PUBLIC int Send (int sock, void *data, unsigned int size);
PUBLIC int HangUp (int sock);
PUBLIC int KillReceiver (u_short port);
PUBLIC int DialServer (char *machine, u_short port);

PRIVATE int SetUpSocket (u_short port);

/* socket part */

PRIVATE int SetUpReceiver (u_short portnum);
PRIVATE int CheckForCall (int s);
PRIVATE int CallSocket (char *hostname, u_short portnum);
PRIVATE int ReadData (int sock, void *data, int BytesRequested);
PRIVATE int WriteData (int sock, void *data, unsigned BytesOut);

/* high level client/server */

PUBLIC char * GetService (int connection, char * text);
PUBLIC char * GetData (int *connection, int port);
PUBLIC void SendData (int connection, char * text);

PRIVATE char * Listen (int connection);

PUBLIC char Data[BUFSIZE];

/*==========================================================================*/

PRIVATE int 
SetUpReceiver (u_short portnum)	/* establish a socket */
{
  char myname[MAXNAME + 1];
  int s, length;
  struct sockaddr_in sa;
  struct hostent *hp;

  bzero (&sa, sizeof (struct sockaddr_in));	/* clear our address */
  gethostname (myname, MAXNAME);	/* who are we? */
  hp = gethostbyname (myname);	/* get our address info */

  if (hp == NULL)
    {				/* we don't exist !? */
      fprintf (stderr, "%s: unknown host\n", myname);
      exit (1);
    }

  sa.sin_family = hp->h_addrtype;	/* this is our host address */
  sa.sin_port = htons (portnum);	/* this is our port number */

  /* create socket */

  if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("opening stream socket");
      exit (1);
    }

  /* bind address to socket */

  if (bind (s, &sa, sizeof (sa)) < 0)
    {
      perror ("binding stream socket");
      close (s);
      return (-1);
    }

  length = sizeof (sa);
  if (getsockname (s, &sa, &length))
    {
      perror ("getting socket name");
      exit (1);
    }
  fprintf (stderr, "socket has port #%d\n", ntohs (sa.sin_port));

  listen (s, 3);		/* max # of queued connects */
  return (s);
}

/*--------------------------------------------------------------------------*/

PRIVATE int 
CheckForCall (int s)
{
  struct sockaddr_in isa;	/* address of socket */
  int i;			/* size of address */
  int t;			/* socket of connection */

  i = sizeof (struct sockaddr_in);

  /* accept connection if there is one */

  if ((t = accept (s, &isa, &i)) < 0)
    {
      return (-1);
    }
  return (t);
}

/*--------------------------------------------------------------------------*/

PRIVATE int 
CallSocket (char *name, u_short portnum)
{
  struct sockaddr_in sa;
  struct hostent *hp;
  char hostname[MAXNAME + 1];
  int a, s;

  if (!name)
    gethostname (hostname, MAXNAME);
  else
    strcpy (hostname, name);

  if ((hp = gethostbyname (hostname)) == NULL)
    {				/* do we know the host's */
      fprintf (stderr, "%s: unknown host\n", hostname);
      exit (1);
    }

  bzero (&sa, sizeof (sa));
  bcopy (hp->h_addr, (char *) &sa.sin_addr, hp->h_length);  /* set address */
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons ((u_short) portnum);

  if ((s = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)
    {				/* get socket */
      perror ("opening stream socket");
      exit (1);
    }
  if (connect (s, &sa, sizeof (sa)) < 0)
    {				/* connect */
      perror ("connecting stream socket");
      close (s);
      exit (1);
    }
  return (s);
}

/*--------------------------------------------------------------------------*/

PRIVATE int 
ReadData (int sock, void *data, int BytesRequested)
{
  int bytecount;		/* counts bytes read */
  int bytesread;		/* bytes read this pass */
  char *ptr;

  ptr = data;
  bytecount = 0;
  bytesread = 0;

  /* fill buffer */

  while (bytecount < BytesRequested)
    {
      if ((bytesread = read (sock, ptr, BytesRequested - bytecount)) > 0)
	{
	  bytecount += bytesread;
	  ptr += bytesread;	/* move buffer ptr for next read */
	}
      else if (bytesread < 0)
	{
	  if (errno == EAGAIN)
	    return (0);		/* no data waiting */
	  if (errno == EWOULDBLOCK)
	    return (0);		/* hm. */
	  else
	    return (-errno);	/* get out of here */
	}
    }
  return (bytecount);
}

/*--------------------------------------------------------------------------*/

/* send <BytesOut> bytes organized in <data> down the socket */

PRIVATE int 
WriteData (int sock, void *data, unsigned BytesOut)
{
  int byteswritten;		/* bytes written this pass */

  byteswritten = 0;

  if ((byteswritten = write (sock, data, BytesOut)) > 0);
  else if (byteswritten < 0)
    {
      if (errno == EAGAIN)
	return (0);		/* can't write */
      if (errno == EWOULDBLOCK)
	return (0);		/* hm. */
      else
	return (-errno);	/* get out of here */
    }
  return (byteswritten);
}

/*==========================================================================*/
/* 
   IPC via socket - basic toolbox: transmission part

   c 1994 Walter Fontana, Interval
 */

PRIVATE int receiver[PORTS];
PRIVATE int InSocket[PORTS];
PRIVATE int OutSocket[PORTS];

/*--------------------------------------------------------------------------*/

PUBLIC int
initialize_socket (int port)  /* just a wrapper */
{
  return (SetUpSocket ((u_short) port));
}

/*--------------------------------------------------------------------------*/

PRIVATE int
SetUpSocket (u_short port)
{
  if ((receiver[port - BASE] = SetUpReceiver (port)) < 0)
    {
      perror ("setting up receiver");
      exit (1);
    }
  MAKE_NONBLOCKING (receiver[port - BASE]);
  return (receiver[port-BASE]);
}

/*--------------------------------------------------------------------------*/

PUBLIC int 
CheckForClientCall (u_short port)
{
  if (InSocket[port - BASE] <= 0)
    {
      /* we are disconnected, see whether we got called */

      if ((InSocket[port - BASE] = CheckForCall (receiver[port - BASE])) > 0)
	MAKE_NONBLOCKING (InSocket[port - BASE]);
    }
  return InSocket[port - BASE];
}

/*--------------------------------------------------------------------------*/

PUBLIC int 
CheckForData (int sock, void *data, unsigned int size)
{
  int b;

  /* we have a connection. Did it send something? */

  if ((b = ReadData (sock, data, size)) > 0)
    return (b);
  else if (b < 0)
    {
      perror (NULL);
      return -1;
    }
}

/*--------------------------------------------------------------------------*/

PUBLIC int 
DialServer (char *machine, u_short port)
{
  if (OutSocket[port - BASE] <= 0)
    {
      if ((OutSocket[port - BASE] = CallSocket (machine, port)) < 0)
	{
	  perror ("troubles in making connection");
	  exit (1);
	}
      MAKE_NONBLOCKING (OutSocket[port - BASE]);
    }
  return OutSocket[port - BASE];
}

/*--------------------------------------------------------------------------*/

PUBLIC int 
Send (int sock, void *data, unsigned int size)
{
  int w;

  if ((w = WriteData (sock, data, size)) > 0)
    return (1);

  printf ("uncapable of sending data\n");
  HangUp (sock);
  return -1;
}

/*--------------------------------------------------------------------------*/

PUBLIC int 
HangUp (int sock)
{
  register int i;

  for (i = 0; i < PORTS; i++)
    {
      if (sock == InSocket[i])
	{
	  close (InSocket[i]);
	  break;
	}
      if (sock == OutSocket[i])
	{
	  close (OutSocket[i]);
	  break;
	}
    }
  InSocket[i] = OutSocket[i] = 0;
  return 1;
}

/*---------------------------------------------------------------------------*/

PUBLIC int 
KillReceiver (u_short port)
{
  close (receiver[port - BASE]);
  InSocket[port - BASE] = OutSocket[port - BASE] = 0;
  return 1;
}

/*==========================================================================
  high level calls for text transmission
==========================================================================*/

PUBLIC char *
GetService (int connection, char * text)
{
  SendData (connection, text);
  return (Listen (connection));
}


/*--------------------------------------------------------------------------*/

PUBLIC char * 
GetData (int *connection, int port)
{
  int nbytes;
  
  while (TRUE) 
    {
      if ((*connection = CheckForClientCall ((u_short) port)) > 0)
	{
	  if (CheckForData (*connection, &nbytes, sizeof (int)) > 0)
	    {
	      while (!CheckForData (*connection, Data, nbytes));
	      if (Data[0] == '*')
		{
		  HangUp (*connection);
		}
	      else if (Data[0] == '@')
		{
		  HangUp (*connection);
		  KillReceiver ((u_short) port);
		  exit (0);
		}
	      else
		{
		  return Data;
		}
	    }
	}
    }
}

/*--------------------------------------------------------------------------*/

PUBLIC void 
SendData (int connection, char * text)
{
  int nbytes;
  
  nbytes = strlen (text) + 1;
  Send (connection, &nbytes, sizeof (int));
  Send (connection, text, nbytes);
  return;
}

/*--------------------------------------------------------------------------*/

PRIVATE char *
Listen (int connection)
{
  int nbytes;
  
  while (!CheckForData (connection, &nbytes, sizeof (int)));
  while (!CheckForData (connection, Data, nbytes));
  return Data;
}

