# This makefile creates the B-Tree database subsystem

CC=cc
CFLAGS=-g
AR=ar
ARFLAGS=-cr
RANLIB=echo
HDRS=../hdrs
OFILES= addkey.o block.o file.o index.o opnbtree.o record.o \
	traverse.o utils.o
DFILES= addkey.c block.c file.c index.c opnbtree.c record.c \
	traverse.c utils.c makefile

btree.a: $(OFILES)
	$(AR) $(ARFLAGS) btree.a $(OFILES)
	$(RANLIB) btree.a

distribute:
	mkdir ../dist/btree
	cp $(DFILES) ../dist/btree

.c.o:
	$(CC) $(CFLAGS) -c -I$(HDRS) $<
