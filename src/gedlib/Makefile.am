# Makefile for the LifeLines gedlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libgedcom.a

libgedcom_a_SOURCES = brwslist.c charmaps.c choose.c editmap.c \
	editvtab.c equaliso.c gedcom.c gstrings.c indiseq.c \
	init.c intrface.c keytonod.c messages.c misc.c names.c \
	node.c nodeutls.c place.c refns.c replace.c spltjoin.c \
	translat.c valid.c valtable.c xreffile.c

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs


# since we're not doing dependencies automagically...
brwslist.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/indiseq.h
charmaps.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/table.h
charmaps.o: ../hdrs/gedcom.h ../hdrs/translat.h
choose.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/indiseq.h
choose.o: ../hdrs/standard.h ../hdrs/gedcom.h
editmap.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/table.h
editmap.o: ../hdrs/gedcom.h ../hdrs/translat.h
editvtab.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/table.h
editvtab.o: ../hdrs/gedcom.h
equaliso.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
equaliso.o: ../hdrs/standard.h
gedcom.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
gedcom.o: ../hdrs/sequence.h ../hdrs/gedcheck.h ../hdrs/standard.h
gstrings.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
gstrings.o: ../hdrs/standard.h ../hdrs/translat.h
indiseq.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
indiseq.o: ../hdrs/indiseq.h ../hdrs/interp.h ../hdrs/standard.h
init.o: ../hdrs/mystring.h ../hdrs/btree.h ../hdrs/table.h ../hdrs/gedcom.h
init.o: ../hdrs/standard.h
intrface.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
keytonod.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
keytonod.o: ../hdrs/standard.h ../hdrs/cache.h
messages.o: ../hdrs/standard.h ../hdrs/mystring.h
misc.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
misc.o: ../hdrs/standard.h
names.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/btree.h ../hdrs/gedcom.h
names.o: ../hdrs/standard.h ../hdrs/translat.h
node.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
node.o: ../hdrs/standard.h ../hdrs/translat.h
nodeutls.o: ../hdrs/mystring.h ../hdrs/btree.h ../hdrs/table.h
nodeutls.o: ../hdrs/standard.h ../hdrs/gedcom.h
place.o: ../hdrs/standard.h ../hdrs/mystring.h
refns.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/btree.h ../hdrs/gedcom.h
refns.o: ../hdrs/standard.h
replace.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
replace.o: ../hdrs/standard.h
spltjoin.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
spltjoin.o: ../hdrs/standard.h
translat.o: ../hdrs/mystring.h ../hdrs/translat.h
translat.o: ../hdrs/standard.h
valid.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
valid.o: ../hdrs/standard.h
valtable.o: ../hdrs/mystring.h ../hdrs/table.h
valtable.o: ../hdrs/standard.h ../hdrs/gedcom.h
xreffile.o: ../hdrs/mystring.h ../hdrs/btree.h ../hdrs/table.h
xreffile.o: ../hdrs/standard.h ../hdrs/gedcom.h

