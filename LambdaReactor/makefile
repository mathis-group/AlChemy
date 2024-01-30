IPC_SOCK = ../IPC/Socket
IPC_SHM  = ../IPC/Shmem

PROG	   = ALCHEMY
PEEP	   = PEEP
TYPETEST   = TYPE_TEST
LAMBDA     = LAMBDA

CC	= cc
INCLUDE = -I$(IPC_SOCK) -I$(IPC_SHM)
#INCLUDE = -I$(IPC_SHM)
CFLAGS  = -ansi -D_DEFAULT_SOURCE -O3 $(INCLUDE)
#CFLAGS  = -g
LIBS    =

FILES   = avl.c \
          avlaccess.c \
	  getopt.c \
	  io.c \
          lambda.c \
          conversions.c \
	  options.c \
	  regex.c \
	  utilities.c \
	  randomexpr.c \
	  filter.c \
	  reactor.c \
	  interact.c \
	  $(IPC_SOCK)/socket.c \
	  $(IPC_SHM)/shmem.c \
	  peephole.c \
	  type_client.c \
	  main.c

OBJS    = avl.o \
          avlaccess.o \
	  getopt.o \
	  io.o \
          lambda.o \
          conversions.o \
	  options.o \
	  regex.o \
	  utilities.o \
	  randomexpr.o \
	  filter.o \
	  reactor.o \
	  interact.o \
	  $(IPC_SOCK)/socket.o \
	  $(IPC_SHM)/shmem.o \
	  peephole.o \
	  type_client.o \
	  main.o

PEEPFILES = peep.c
PEEPOBJS  = peep.o \
	    $(IPC_SOCK)/socket.o \
	    utilities.o

TYPEFILES = type_test.c
TYPEOBJS  = type_test.o \
	    $(IPC_SOCK)/socket.o \
	    utilities.o

all: $(PROG) $(PEEP) $(TYPETEST)

type_client.o:
		$(CC) -c $(CFLAGS) $(INCLUDE) type_client.c
peephole.o:
		$(CC) -c $(CFLAGS) -I$(IPC_SOCK) peephole.c
type_test.o:
		$(CC) -c $(CFLAGS) -I$(IPC_SOCK) type_test.c

$(PROG):    $(OBJS)
	    $(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LIBS)

$(PEEP):    $(PEEPOBJS)
	    $(CC) -o $(PEEP) $(CFLAGS) $(PEEPOBJS) $(LIBS)

$(TYPETEST):	$(TYPEOBJS)
		$(CC) -o $(TYPETEST) $(CFLAGS) $(TYPEOBJS) $(LIBS)

$(LAMBDA):  lambda.c lambda.o utilities.o conversions.o
	    $(CC) -g -o $(LAMBDA) -DLAMBDA_TEST $(CFLAGS) lambda.c utilities.o \
	    conversions.o

nuke: clean
	rm -f *.bak *~ $(PROG) $(LAMBDA) $(PEEP) $(TYPETEST)

clean:
	rm -f *.o core

avl.o: avl.h include.h
avlaccess.o: avlaccess.h include.h utilities.h structs.h avl.h lambda.h
avlaccess.o: regex.h filter.h
conversions.o: utilities.h include.h lambda.h structs.h
conversions.o: avl.h regex.h filter.h
filter.o: include.h regex.h filter.h conversions.h
getopt.o: getopt.h
interact.o: peephole.h structs.h include.h avl.h lambda.h regex.h filter.h
interact.o: interact.h conversions.h type_client.h
io.o: utilities.h include.h conversions.h
io.o: avlaccess.h structs.h avl.h lambda.h regex.h filter.h randomexpr.h
io.o: reactor.h interact.h type_client.h
lambda.o: utilities.h
lambda.o: include.h lambda.h
main.o: utilities.h include.h avlaccess.h structs.h avl.h lambda.h regex.h
main.o: filter.h randomexpr.h reactor.h interact.h options.h getopt.h
main.o: peephole.h io.h type_client.h
options.o: include.h options.h
options.o: getopt.h
peep.o: include.h utilities.h peephole.h structs.h avl.h lambda.h regex.h
peep.o: filter.h
peephole.o: peephole.h structs.h
peephole.o: include.h avl.h lambda.h regex.h filter.h ../IPC/Socket/socket.h
randomexpr.o: utilities.h include.h structs.h avl.h
randomexpr.o: lambda.h regex.h filter.h randomexpr.h conversions.h
reactor.o: utilities.h include.h structs.h avl.h
reactor.o: lambda.h regex.h filter.h avlaccess.h interact.h type_client.h
regex.o: regex.h
type_client.o: include.h utilities.h
type_client.o: ../IPC/Socket/socket.h type_client.h
type_client.o: ../IPC/Shmem/shmem.h
type_test.o: include.h utilities.h
type_test.o: ../IPC/Socket/socket.h
utilities.o: include.h

.PHONY: nuke clean
