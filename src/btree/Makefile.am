# This makefile creates the B-Tree database subsystem

AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libbtree.a

libbtree_a_SOURCES = addkey.c block.c file.c index.c opnbtree.c record.c traverse.c utils.c

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs

