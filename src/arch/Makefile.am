## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES= libarch.a

libarch_a_SOURCES = platform.c
libarch_a_DEPENDENCIES = @LIBOBJS@
libarch_a_LIBADD = @LIBOBJS@

# $(top_builddir)        for config.h
# $(top_srcdir)/intl     for intl headers
# $(top_builddir)/intl   for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_builddir) -I$(top_srcdir)/intl -I$(top_builddir)/intl -I$(top_srcdir)/src/hdrs

