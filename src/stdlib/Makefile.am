## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines stdlib area

noinst_LIBRARIES = libstdlib.a

libstdlib_a_SOURCES = appendstr.c \
	dirs.c environ.c fpattern.c icvt.c list.c \
	listener.c lldate.c llstrcmp.c memalloc.c mystring.c \
	path.c proptbls.c sequence.c \
	signals.c sprintpic.c stdlibi.h stdstrng.c \
	stralloc.c strapp.c strcvt.c strset.c strutf8.c strwhite.c \
	table.c warehouse.c zstr.c

# $(top_srcdir)          for config.h
# $(top_srcdir)/intl     for intl headers
# ../../intl             for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/intl -I../../intl -I$(top_srcdir)/src/hdrs

EXTRA_DIST = stdlibi.h

