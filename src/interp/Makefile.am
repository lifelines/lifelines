# Makefile for the LifeLines report interpreter subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = alloc.c builtin.c eval.c functab.c heapused.c \
	interp.c intrpseq.c lex.c more.c pvalue.c \
	pvalmath.c rassa.c symtab.c write.c yacc.y

# .. for config.h, hdrs for lots of stuff, ../intl for libgnuintl.h
INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs -I$(srcdir)/../intl
AM_YFLAGS = -d

# pvalue.c with -O2 triggers a gcc error in gcc version 2.95.2 19991024 (release) 
# The CFLAGS below is gcc specific.  Sorry. -marc
CFLAGS = -g -W -Wall -Wcast-align -Wmissing-declarations -Wmissing-prototypes -Wreturn-type -Wstrict-prototypes -pedantic -g

EXTRA_DIST = yacc.h interpi.h parse.h

lex.o: yacc.h parse.h


