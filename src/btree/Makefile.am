# This makefile creates the B-Tree database subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libbtree.a

libbtree_a_SOURCES = \
	addkey.c \
	block.c \
	btrec.c \
	file.c \
	index.c \
	opnbtree.c \
	traverse.c \
	utils.c \
	btreei.h

# $(top_builddir)        for config.h
# $(top_srcdir)/intl     for intl headers
# $(top_builddir)intl    for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_builddir) -I$(top_srcdir)/intl -I$(top_builddir)/intl -I$(top_srcdir)/src/hdrs

EXTRA_DIST = btreei.h

# since we're not doing dependencies automagically...
addkey.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
block.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
file.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
index.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
opnbtree.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
btrec.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
traverse.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
utils.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h

