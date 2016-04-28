## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

SUBDIRS = mswin

noinst_LIBRARIES= libarch.a

libarch_a_SOURCES = platform.c
libarch_a_DEPENDENCIES = @LIBOBJS@
libarch_a_LIBADD = @LIBOBJS@

# $(top_builddir)        for config.h
# $(top_srcdir)/src/hdrs for headers

AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir)/src/hdrs

