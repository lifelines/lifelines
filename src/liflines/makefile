# Makefile that creates the LifeLines executable.

CC=cc
#CFLAGS=-g -DBETA
CFLAGS=-g
DFLAGS=
EXEC=lines302
HDRS=../hdrs
LLIBS=../interp/interp.a ../gedlib/gedlib.a ../stdlib/stdlib.a \
	 ../btree/btree.a
#MALLOC=-lmalloc
MALLOC=
TLIB=
SLIBS=$(MALLOC) -lcurses $(TLIB)
BIN=../bin

OFILES= add.o advedit.o ask.o browse.o delete.o edit.o export.o import.o \
	lbrowse.o main.o merge.o miscutls.o newrecs.o remove.o show.o \
	screen.o swap.o tandem.o valgdcom.o
DFILES= add.c advedit.c ask.c browse.c delete.c edit.c export.c import.c \
	lbrowse.c main.c merge.c miscutls.c newrecs.c remove.c show.c \
	screen.c swap.c tandem.c valgdcom.c makefile

build: $(OFILES)
	rm -f $(EXEC)
	rm -f $(BIN)/$(EXEC)
	$(CC) -o $(EXEC) $(OFILES) $(LLIBS) $(SLIBS)
	ln $(EXEC) $(BIN)

distribute:
	mkdir ../dist/liflines
	cp $(DFILES) ../dist/liflines

.c.o:
	$(CC) $(CFLAGS) -c $(DFLAGS) -I$(HDRS) $<
