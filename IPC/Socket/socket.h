/*  
    socket.h
    
    c 1994 Walter Fontana, Interval
 */
 
#ifndef	__SOCK_H
#define	__SOCK_H

#include <sys/types.h>

extern char * GetService (int connection, char * text);
extern char * GetData (int *connection, int port);
extern void SendData (int connection, char * text);

extern int initialize_socket (int port);
extern int CheckForClientCall (u_short port);
extern int CheckForData (int sock, void *data, unsigned int size);
extern int Send (int sock, void *data, unsigned int size);
extern int HangUp (int sock);
extern int KillReceiver (u_short port);
extern int DialServer (char *machine, u_short port);

#endif /* __SOCK_H */
