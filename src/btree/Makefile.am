# This makefile creates the B-Tree database subsystem

AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libbtree.a

libbtree_a_SOURCES = \
	addkey.c \
	addkey.h \
	block.c \
	block.h \
	file.c \
	index.c \
	index.h \
	opnbtree.c \
	opnbtree.h \
	record.c \
	traverse.c \
	utils.c \
	utils.h

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs

# since we're not doing dependencies automagically...
addkey.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
block.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
file.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
index.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
opnbtree.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
record.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
traverse.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
utils.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h

