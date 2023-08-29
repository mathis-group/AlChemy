
CLIENT	   = client
SERVER	   = server

CC	= cc
CFLAGS  = -ansi -D_DEFAULT_SOURCE -O3
LIBS    =

CLIENTFILES = clientShmem.c \
              utilities.c \
              shmem.c
CLIENTOBJS  = clientShmem.o \
              utilities.o \
              shmem.o

SERVERFILES = serverShmem.c \
              utilities.c \
              shmem.c
SERVEROBJS  = serverShmem.o \
              utilities.o \
              shmem.o

all: $(CLIENT) $(SERVER)

$(CLIENT):  $(CLIENTOBJS)
	    $(CC) -o $(CLIENT) $(CFLAGS) $(CLIENTOBJS) $(LIBS)

$(SERVER):  $(SERVEROBJS)
	    $(CC) -o $(SERVER) $(CFLAGS) $(SERVEROBJS) $(LIBS)


nuke: clean
	rm -f *.bak *~ $(CLIENT) $(SERVER)

clean:
	rm -f *.o core

clientShmem.o: utilities.h shmem.h
serverShmem.o: include.h shmem.h
shmem.o: include.h
utilities.o: include.h

.PHONY: nuke clean
