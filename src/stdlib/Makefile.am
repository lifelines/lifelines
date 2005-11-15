## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines stdlib area

noinst_LIBRARIES = libstdlib.a

libstdlib_a_SOURCES = appendstr.c array.c \
	dirs.c environ.c errlog.c \
	fileops.c fpattern.c generic.c \
	icvt.c hashtab.c list.c \
	listener.c llabort.c lldate.c llstrcmp.c log.c memalloc.c \
	mychar_funcs.c mychar_tables.c mystring.c \
	norm_charmap.c object.c \
	path.c proptbls.c rbtree.c sequence.c \
	signals.c sprintpic.c stack.c \
	stdlib.c stdlibi.h stdstrng.c \
	stralloc.c strapp.c strcvt.c strset.c strutf8.c strwhite.c \
	table.c vtable.c zstr.c

# $(top_builddir)        for config.h
# $(top_srcdir)/intl     for intl headers
# $(top_builddir)/intl   for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_builddir) -I$(top_srcdir)/intl -I$(top_builddir)/intl -I$(top_srcdir)/src/hdrs

EXTRA_DIST = rbtree.h stack.h stdlibi.h

