# Makefile for the LifeLines stdlib area

CC=cc
CFLAGS=-g
AR=ar
ARFLAGS=-cr
RANLIB=echo
HDRS=../hdrs
OFILES= assert.o double.o memalloc.o path.o signals.o sequence.o \
	stdstrng.o table.o
DFILES= assert.c double.c memalloc.c path.c signals.c sequence.c \
	stdstrng.c table.c makefile

stdlib.a: $(OFILES)
	$(AR) $(ARFLAGS) stdlib.a $(OFILES)
	$(RANLIB) stdlib.a

distribute:
	mkdir ../dist/stdlib
	cp $(DFILES) ../dist/stdlib

.c.o:
	$(CC) $(CFLAGS) -c -I$(HDRS) $<
