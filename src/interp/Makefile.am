# Makefile for the LifeLines report interpreter subsystem

AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = alloc.c builtin.c date.c eval.c functab.c interp.c \
	intrpseq.c lex.c more.c pvalue.c rassa.c write.c yacc.y

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs

AM_YFLAGS = -d

EXTRA_DIST = yacc.h

