CLIENT	   = client
SERVER	   = server

CC	= cc
CFLAGS  = -ansi -D_DEFAULT_SOURCE -O3
LIBS    =

CLIENTFILES = clientSock.c \
              utilities.c \
              socket.c
CLIENTOBJS  = clientSock.o \
              utilities.o \
              socket.o

SERVERFILES = serverSock.c \
              utilities.c \
              socket.c
SERVEROBJS  = serverSock.o \
              utilities.o \
              socket.o

all: $(CLIENT) $(SERVER)

$(CLIENT):  $(CLIENTOBJS)
	    $(CC) -o $(CLIENT) $(CFLAGS) $(CLIENTOBJS) $(LIBS)

$(SERVER):  $(SERVEROBJS)
	    $(CC) -o $(SERVER) $(CFLAGS) $(SERVEROBJS) $(LIBS)

nuke: clean
	rm -f *.bak *~ $(CLIENT) $(SERVER)

clean:
	rm -f *.o core

clientSock.o: include.h utilities.h socket.h
serverSock.o: include.h utilities.h socket.h
socket.o: include.h socket.h
utilities.o: include.h

.PHONY: nuke clean
