TYPER  = TYPER
TYPER2 = TYPER2
LEXER  = LEX
PARSER = PARSE
TYPE   = TYPE
BASIS  = BASIS

C	= cc
CC	= camlc
CFLAGS	= -O fast

INCLUDE = /usr/local/lib/caml-light

LIBS	=

BASIS_OBJECTS  = lex.zi \
		 lambdatype.zi \
		 parse.zi \
		 type.zi \
		 basis.zi \
		 utils.zi \
		 printers.zi \
		 utils.zo \
		 printers.zo \
		 basis.zo
BASIS_ZO       = utils.zo \
		 printers.zo \
	 	 basis.zo

LEXER_OBJECTS = lex.zi \
		utils.zi \
		utils.zo \
		lex.zo \
		lex_int.zo
LEXER_ZO      = utils.zo \
		lex.zo \
		lex_int.zo

PARSER_OBJECTS	= lex.zi \
		  lambdatype.zi \
		  parse.zi \
		  type.zi \
		  basis.zi \
		  utils.zi \
		  printers.zi \
		  lex.zo \
		  utils.zo \
		  printers.zo \
		  basis.zo \
		  parse.zo \
		  parse_int.zo
PARSER_ZO	= utils.zo \
		  lex.zo \
		  printers.zo \
		  basis.zo \
		  parse.zo \
		  parse_int.zo

TYPE_OBJECTS  = lex.zi \
		lambdatype.zi \
		parse.zi \
		type.zi \
		basis.zi \
		utils.zi \
		printers.zi \
		lex.zo \
		utils.zo \
		basis.zo \
		printers.zo \
		parse.zo \
		type.zo \
		type_int.zo
TYPE_ZO	      = utils.zo \
		lex.zo \
		basis.zo \
		printers.zo \
		parse.zo \
		type.zo \
		type_int.zo

TYPER_OBJECTS  = lex.zi \
		 lambdatype.zi \
		 parse.zi \
		 type.zi \
		 basis.zi \
		 utils.zi \
		 printers.zi \
		 socketML.zi \
		 lex.zo \
		 utils.zo \
		 basis.zo \
		 parse.zo \
		 printers.zo \
		 type.zo \
		 socketML.o \
		 ipc.zo

TYPER_ZO       = utils.zo \
		 lex.zo \
		 basis.zo \
		 parse.zo \
		 printers.zo \
		 type.zo \
		 socketML.o \
		 ipc.zo

TYPER2_OBJECTS = lex.zi \
		 lambdatype.zi \
		 parse.zi \
		 type.zi \
		 basis.zi \
		 utils.zi \
		 printers.zi \
		 shmemML.zi \
		 lex.zo \
		 utils.zo \
		 basis.zo \
		 parse.zo \
		 printers.zo \
		 type.zo \
		 shmemML.o \
		 ipcShm.zo

TYPER2_ZO      = utils.zo \
		 lex.zo \
		 utils.zo \
		 basis.zo \
		 parse.zo \
		 printers.zo \
		 type.zo \
		 shmemML.o \
		 ipcShm.zo

all: $(BASIS) $(LEXER) $(PARSER) $(TYPE) $(TYPER) $(TYPER2)

$(LEXER): $(LEXER_OBJECTS)
	  $(CC) -o $(LEXER) $(CFLAGS) $(LEXER_ZO)

$(PARSER): $(PARSER_OBJECTS)
	  $(CC) -o $(PARSER) $(CFLAGS) $(PARSER_ZO)

$(TYPE): $(TYPE_OBJECTS)
	  $(CC) -o $(TYPE) $(CFLAGS) $(TYPE_ZO)

$(BASIS): $(BASIS_OBJECTS)
	  $(CC) -o $(BASIS) $(CFLAGS) $(BASIS_ZO)

$(TYPER): $(TYPER_OBJECTS)
	  $(CC) -custom -o $(TYPER) $(CFLAGS) $(TYPER_ZO) ../IPC/Socket/socket.o

$(TYPER2): $(TYPER2_OBJECTS)
	  $(CC) -custom -o $(TYPER2) $(CFLAGS) $(TYPER2_ZO) ../IPC/Shmem/shmem.o

clean:
	rm -f *.o *.zo *.zi

nuke: clean
	rm -f *.bak *~ core $(BASIS) $(LEXER) $(PARSER) $(TYPE) $(TYPER) $(TYPER2)

.SUFFIXES: .ml .mli .zo .zi .c .o

.mli.zi:
	$(CC) -c $(CFLAGS) $<

.ml.zo:
	$(CC) -c $(CFLAGS) $<

.c.o:
	$(C) -c -O2 -I../IPC/Socket -I../IPC/Shmem -I$(INCLUDE) $<


shmemML.o: ../IPC/Shmem/shmem.h
socketML.o: ../IPC/Socket/include.h
socketML.o: ../IPC/Socket/socket.h

.PHONY: nuke clean
