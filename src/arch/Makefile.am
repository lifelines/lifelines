## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES= libarch.a

libarch_a_SOURCES = platform.c
libarch_a_DEPENDENCIES = @LIBOBJS@
libarch_a_LIBADD = @LIBOBJS@

# $(top_srcdir)          for config.h
# $(top_srcdir)/intl     for intl headers
# ../../intl             for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/intl -I../../intl -I$(top_srcdir)/src/hdrs

