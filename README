----------------------------------------------------------------------
AlChemy -- c 1994 - 1996 by Walter Fontana and Leo Buss
----------------------------------------------------------------------

CONDITIONS OF USE

The terms of use are spelled out in the GNU GENERAL PUBLIC LICENSE.
PLEASE READ THE FILE COPYING.

Other than that, we ask you to include an acknowledgement in your work
should you make substantial use of this software. At present,
appropriate references are

           W. Fontana and L. W. Buss 
           'The Arrival of the Fittest': Toward a Theory of 
	   Biological Organization 
           Bull. Math. Biol., 56, 1-64 (1994)
or
           W. Fontana and L. W. Buss
           The barrier of objects: From dynamical systems to 
	   bounded organizations,
           in: Boundaries and Barriers, J. Casti and A. Karlqvist (eds.), 
	   pp. 56-116, Addison-Wesley, 1996


COMPILATION

If you use the type system, you must first install caml-light 0.71.
Caml-light is a small protable dialect of ML, courtesy Xavier Leroy
and others at INRIA, France (ftp://ftp.inria.fr). PLEASE READ THE
COPYRIGHT NOTICE INCLUDED WIH THE DISTRIBUTION OF CAML-LIGHT.

Once caml-light is installed, in the directory of alchemy type

make all


INPUT

The input is now exclusively through file. The input format is rather
friendly. Look at the default input file alchemy.inp. It is
self-explanatory. Read the one basic rule of input in that file!

PEEPHOLE

The code has been extended with interprocess communication (IPC) via
sockets. What this practically means is that you can interact with your
simulation (whether it runs in the background or not) on whatever
machine it is running on the Internet.

When you type

alchemy

the program reads alchemy.inp, does its initializations and begins
looping around its main loop doing the collisions. After every
collision the process checks whether there was a call and whether data
have been sent on port 5000. If no event occured on port 5000 it
continues.

In the same directory there is a programm called "peep".
Run peep anywhere (say on the machine where you also run alchemy).

peep replies with

enter machine to connect to (return if same as caller)

you type return, or the internet address of the machine alchemy is
running on.  Then peep asks for the port. I have chosen an arbitrary
number such as 5000.  (Some ports are reserved for email and stuff
like that.)

enter port number (return if 5000)

hence, just type return.  Since alchemy is listening to the port only
after a collision, it may take a little while until something appears
on the peep-screen, because alchemy has to complete the reduction it
is in.  Alchemy notices that peep has connected on port 5000. But no
information has been sent, and alchemy continues with its collisions.
The peep screen now says sometin like this:

>> 0 << control variable
         current value: 1
         options:
                  0 --> pause simulation
                  1 --> resume simulation
                  2 --> re-read parameters from input file
                  3 --> display current parameters
                 -9 --> kill simulation
>> 1 << show reaction
         current value: 0
         options:
                  1 --> show on
                  0 --> show off :-)

==>input code and value or \ to quit this process

If you type \, then you quit peep, and alchemy continues undisturbed.
If you type, say,
1 1

It means that you want to set the control variable number 1 to the
value 1. According to the explanation on the screen that means that
you turn on a peep show :-). Alchemy will report all collisions - on
the screen of the machine it is running on! (We don't want to clog the
network...)

If you type 
1 0
you turn the peep show off.

If you type 
0 0

you pause the simulation. Alchemy enters an endless loop and watches
for something to come in on port 5000. If that which comes n changes
the control variable number 0 to 1, alchemy exits the loop and
continues with its collisions.  Try it while having the peep show on.

Turn the peep show off again. (1 0)
With 
0 3

you have alchemy display it's most important status variables.
Now comes the nice thing:
With
0 2

you tell alchemy to re-read the input file alchemy,inp.  Nobody
prevents you to have made changes to that input file.  Say, adding a
filter, turning on copies, changing the system size, etc.  (You can't
at present change the number of organizations, however) If you do so,
you may want to change the name of the simulation in the input file,
so as to know in the snapshots which ones refer to the old parameter
settings and which ones to the new parameters.  If you don't do so,
alchemy does it for you appending a ^ (hat) to your previous file
name.

In sum: you can change parameters of your simulation as it runs, no
matter where no matter how (foreground or background).  Of course,
this may be used to attach a separate graphical monitoring process.
However, it is not recommended to transmit lots of data along sockets,
since it is somwhat slow. If you need IPC with many data, then you
should use "shared memory", but this will confine the communicating
processes to the same machine.

Finally, with 0 -9 you can remotely kill the simulation.

COMMAND LINE OPTIONS

Alchemy has a few command line options that can be easily expanded
with the options.c interface to GNU's getopt.c
If you type 
alchemy -h

you will get info about the options:

This is alchemy on Nov  2 1994 at 17:14:26

Usage: alchemy

Options are:
  -f                    <arg> input filename
  -p                          pair interactions only
  -a                          report all actions in pair int
command line settings:
 -f                    = alchemy.inp
 -p                    = 0
 -a                    = 0
 
hence with 
alchemy -f leo.in

you change the default input file name from alchemy.inp to leo.in. The
-p options is now used to do all the pair interactions for later
analysis. alchemy will not do collisions, just all pair interactions
of the objects that have been read in.  The -a options tells alchemy
to produce all pair interactions omitting all the filters (this is
used to get a feeling for ehat is being filtered, and what is non
terminating within limits).

Here is a packing list:

alchemy.inp       sample input file
avl.c		  AVL data structure
avl.h
avlaccess.c	  interface to AVL data structure
avlaccess.h
filter.c	  handles filters (regex, at present)
filter.h
getopt.c	  GNU's command line option parser
getopt.h
include.h	  generic header file
interact.c	  holds all reaction schemes
interact.h
io.c		  I/O
io.h
lambda.c	  the best, the fastest, lambda herself
lambda.h
lambda.test
main.c		  main 
makefile
makefile.bak
options.c	  my interface to GNU's option parser
options.h
peep.c		  separate peep process
peephole.c	  peep hole code for alchemy
peephole.h
randomexpr.c	  random expression generator
randomexpr.h
reactor.c	  reactor manager
reactor.h
regex.c		  GNU's regular expression handler
regex.h
socket.c	  socket package
socket.h
structs.h	  most used structures
utilities.c	  general utilities
utilities.h

TYPE system

If you use the type ssytem, you must start first the typer process
before you start alchemy. The typer is in TypeSynthesis as TYPER (for
sockets). TYPER requires a port number (>= 5000), e.g. TYPER 5000, to
communicate with Alchemy.

The scheme is as follows: the lambda normalization is done (in C)
within the reactor package LambdaReactor, while the typing is done (in
Caml-light) with a separate server process, due to "multilingual"
programming constraints.

Whenever a collision happens, the interaction (i.e, the "application"
of A to B) is sent to the TYPER for type sunthesis. If the expression
is typable, then the normalization proceeds, if it is not, then the
collision is non-reactive ("elastic").


Have fun!

Walter Fontana
