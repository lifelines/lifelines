# Makefile for the LifeLines report interpreter subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = alloc.c builtin.c date.c eval.c functab.c interp.c \
	intrpseq.c lex.c more.c pvalue.c rassa.c write.c heapused.c yacc.y

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs
AM_YFLAGS = -d

# pvalue.c with -O2 triggers a gcc error in gcc version 2.95.2 19991024 (release) 
# The CFLAGS below is gcc specific.  Sorry. -marc
CFLAGS = -g -W -Wall -Wcast-align -Wmissing-declarations -Wmissing-prototypes -Wreturn-type -Wstrict-prototypes -pedantic -g

EXTRA_DIST = yacc.h

lex.o: yacc.h

# since we're not doing dependencies automagically...
alloc.o: ../hdrs/standard.h
alloc.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/interp.h
builtin.o: ../hdrs/standard.h
builtin.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
builtin.o: ../hdrs/cache.h ../hdrs/interp.h ../hdrs/indiseq.h
builtin.o: ../hdrs/translat.h
date.o: ../hdrs/standard.h
date.o: ../hdrs/mystring.h ../hdrs/table.h
eval.o: ../hdrs/standard.h
eval.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/interp.h
functab.o: ../hdrs/standard.h
functab.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
functab.o: ../hdrs/interp.h
interp.o: ../hdrs/standard.h
interp.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
interp.o: ../hdrs/interp.h ../hdrs/cache.h ../hdrs/indiseq.h
intrpseq.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/table.h
intrpseq.o: ../hdrs/gedcom.h ../hdrs/interp.h ../hdrs/indiseq.h
lex.o: ../hdrs/standard.h
lex.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/interp.h
more.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/table.h
more.o: ../hdrs/gedcom.h ../hdrs/cache.h ../hdrs/interp.h ../hdrs/indiseq.h
pvalue.o: ../hdrs/standard.h
pvalue.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/cache.h
pvalue.o: ../hdrs/interp.h
rassa.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/table.h
rassa.o: ../hdrs/gedcom.h ../hdrs/interp.h
write.o: ../hdrs/standard.h
write.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/interp.h
yacc.o: ../hdrs/standard.h
yacc.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/interp.h

