# Makefile for the LifeLines stdlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libstdlib.a

libstdlib_a_SOURCES = assert.c double.c memalloc.c path.c signals.c \
	sequence.c stdstrng.c table.c llstrcmp.c mystring.c \
	stdlibi.h

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs

# since we're not doing dependencies automagically...
assert.o: ../hdrs/standard.h
assert.o: ../hdrs/mystring.h
double.o: ../hdrs/standard.h
double.o: ../hdrs/mystring.h
llstrcmp.o: ../hdrs/mystring.h
memalloc.o: ../hdrs/standard.h
memalloc.o: ../hdrs/mystring.h
mystring.o: ../hdrs/mystring.h
path.o: ../hdrs/standard.h
path.o: ../hdrs/mystring.h
sequence.o: ../hdrs/standard.h
sequence.o: ../hdrs/mystring.h ../hdrs/sequence.h
signals.o: ../hdrs/standard.h
signals.o: ../hdrs/mystring.h ../hdrs/table.h
signals.o: ../hdrs/gedcom.h ../hdrs/interp.h
stdstrng.o: ../hdrs/standard.h
stdstrng.o: ../hdrs/mystring.h
table.o: ../hdrs/standard.h
table.o: ../hdrs/mystring.h ../hdrs/table.h

