## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines stdlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libstdlib.a

libstdlib_a_SOURCES = bfs.c dirs.c double.c environ.c fpattern.c\
	lldate.c llstrcmp.c memalloc.c mystring.c path.c sequence.c\
	signals.c stdstrng.c stdlibi.h table.c warehouse.c lloptions.c

# .. for config.h, hdrs for lots of stuff, ../intl for libgnuintl.h
INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs -I$(srcdir)/../intl

EXTRA_DIST = stdlibi.h

