# Makefile for the LifeLines gedlib area

CC=cc
CFLAGS=-g
AR=ar
ARFLAGS=-cr
RANLIB=echo
HDRS=../hdrs
OFILES= brwslist.o charmaps.o choose.o editmap.o editvtab.o gedcom.o \
	gstrings.o indiseq.o init.o intrface.o keytonod.o messages.o \
	misc.o names.o node.o place.o refns.o spltjoin.o translat.o valid.o \
	valtable.o xreffile.o
DFILES= brwslist.c charmaps.c choose.c editmap.c editvtab.c gedcom.c \
	gstrings.c indiseq.c init.c intrface.c keytonod.c messages.c \
	misc.c names.c node.c place.c refns.c spltjoin.c translat.c valid.c \
	valtable.c xreffile.c makefile

gedlib.a: $(OFILES)
	$(AR) $(ARFLAGS) gedlib.a $(OFILES)
	$(RANLIB) gedlib.a

distribute:
	mkdir ../dist/gedlib
	cp $(DFILES) ../dist/gedlib

.c.o:
	$(CC) $(CFLAGS) -c -I$(HDRS) $<
