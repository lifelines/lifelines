# Makefile for the LifeLines stdlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libstdlib.a

libstdlib_a_SOURCES = bfs.c dirs.c double.c environ.c fpattern.c\
	lldate.c llstrcmp.c memalloc.c mystring.c path.c sequence.c\
	signals.c stdstrng.c stdlibi.h table.c warehouse.c lloptions.c

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs

EXTRA_DIST = stdlibi.h


# since we're not doing dependencies automagically...
bfs.o: ../hdrs/llstdlib.h
bfs.o: ../hdrs/bfs.h
double.o: ../hdrs/standard.h
double.o: ../hdrs/mystring.h
dirs.o: ../hdrs/standard.h ../hdrs/llstdlib.h
environ.o: ../hdrs/sys_inc.h ../hdrs/llstdlib.h
fpattern.o: ../hdrs/fpattern.h
lldate.o: ../hdrs/llstdlib.h
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
table.o: ../hdrs/llstdlib.h ../hdrs/table.h
warehouse.o: ../hdrs/llstdlib.h ../hdrs/warehouse.h

