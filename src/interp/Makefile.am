# Makefile for the LifeLines report interpreter subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libinterp.a

libinterp_a_SOURCES = alloc.c builtin.c builtin_list.c eval.c \
	functab.c heapused.c \
	interp.c intrpseq.c lex.c more.c progerr.c \
	pvalalloc.c pvalmath.c pvalue.c \
	rassa.c rptsort.c rptui.c \
	symtab.c write.c yacc.y

BUILT_SOURCES = yacc.h

# $(top_builddir)        for config.h
# $(top_srcdir)/src/hdrs for headers

AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir)/src/hdrs

# We want a header file (-d) and debugging support (-t)
AM_YFLAGS = -d -t

# NOTE: Automake will automatically include the yacc.[ch] intermediate files
# in the distribution so we do not need to add them to EXTRA_DIST.
# If you want to rebuild the yacc.[ch], intermediate files, either:
# a) remove them and run make in this directory, or
# b) run "make clean-maintainer" here and then run ./configure and make again.

EXTRA_DIST = interpi.h rptui.h
MAINTAINERCLEANFILES = yacc.output

lex.o: yacc.h
interp.o: yacc.h
