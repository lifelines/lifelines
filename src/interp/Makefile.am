# Makefile for the LifeLines report interpreter subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = alloc.c builtin.c builtin_list.c eval.c \
	functab.c heapused.c \
	interp.c intrpseq.c lex.c more.c progerr.c \
	pvalalloc.c pvalmath.c pvalue.c \
	rassa.c rptsort.c rptui.c \
	symtab.c write.c yacc.y

# $(top_builddir)        for config.h
# $(top_srcdir)/src/hdrs for headers

AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir)/src/hdrs

AM_YFLAGS = -d

# pvalue.c with -O2 triggers a gcc error in gcc version 2.95.2 19991024 (release) 
# The CFLAGS below is gcc specific.  Sorry. -marc
AM_CFLAGS=-g -W -Wall -Wcast-align -Wmissing-declarations -Wmissing-prototypes -Wreturn-type -Wstrict-prototypes -pedantic -g

EXTRA_DIST = yacc.h yacc.c interpi.h rptui.h

lex.o: yacc.h parse.h

