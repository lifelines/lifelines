# Makefile for the LifeLines report interpreter subsystem

CC=cc
CFLAGS=-g
AR=ar
ARFLAGS=-cr
RANLIB=echo
HDRS=../hdrs
OFILES= alloc.o builtin.o date.o eval.o functab.o interp.o \
	intrpseq.o lex.o more.o rassa.o write.o yacc.o
DFILES= alloc.c builtin.c date.c eval.c functab.c interp.c \
	intrpseq.c lex.c more.c rassa.c write.c yacc.c yacc.y makefile

interp.a: $(OFILES)
	$(AR) $(ARFLAGS) interp.a $(OFILES)
	$(RANLIB) interp.a

distribute:
	mkdir ../dist/interp
	cp $(DFILES) ../dist/interp

.c.o:
	$(CC) $(CFLAGS) -c -I$(HDRS) $<
