# Makefile for the LifeLines report interpreter subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = alloc.c builtin.c eval.c functab.c heapused.c \
	interp.c intrpseq.c lex.c more.c pvalue.c \
	pvalmath.c rassa.c symtab.c write.c yacc.y

# $(top_srcdir)          for config.h
# $(top_srcdir)/intl     for libintl
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/intl -I$(top_srcdir)/src/hdrs

AM_YFLAGS = -d

# pvalue.c with -O2 triggers a gcc error in gcc version 2.95.2 19991024 (release) 
# The CFLAGS below is gcc specific.  Sorry. -marc
CFLAGS = -g -W -Wall -Wcast-align -Wmissing-declarations -Wmissing-prototypes -Wreturn-type -Wstrict-prototypes -pedantic -g

EXTRA_DIST = yacc.h interpi.h parse.h

lex.o: yacc.h parse.h


