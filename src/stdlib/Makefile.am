## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines stdlib area

noinst_LIBRARIES = libstdlib.a

libstdlib_a_SOURCES = appendstr.c \
	bfs.c dirs.c double.c environ.c fpattern.c icvt.c \
	listener.c lldate.c llstrcmp.c memalloc.c mystring.c \
	path.c sequence.c \
	signals.c sprintpic.c stdlibi.h stdstrng.c \
	stralloc.c strapp.c strcvt.c strutf8.c strwhite.c \
	table.c warehouse.c

# .. for config.h, hdrs for lots of stuff, ../intl for libgnuintl.h
INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs -I$(srcdir)/../intl

EXTRA_DIST = stdlibi.h

