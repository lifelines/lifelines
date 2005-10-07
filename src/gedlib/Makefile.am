# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines gedlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libgedcom.a

libgedcom_a_SOURCES = brwslist.c charmaps.c charprops.c \
	choose.c codesets.c \
	datei.c dateparse.c dateprint.c \
	dbcontext.c dblist.c dispfmt.c editmap.c \
	editvtab.c equaliso.c gedcom.c gengedc.c gstrings.c \
	indiseq.c init.c intrface.c keytonod.c lloptions.c \
	lldatabase.c llgettext.c locales.c \
	messages.c misc.c names.c node.c nodechk.c \
	nodeio.c nodeutls.c place.c \
	property.c record.c refns.c remove.c replace.c \
	soundex.c spltjoin.c \
	translat.c valid.c valtable.c xlat.c xreffile.c
DEFS = -DSYS_CONF_DIR=\"$(sysconfdir)\" @DEFS@

# $(top_builddir)        for config.h
# $(top_srcdir)/intl     for intl headers
# $(top_builddir)intl    for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_builddir) -I$(top_srcdir)/intl -I$(top_builddir)/intl -I$(top_srcdir)/src/hdrs

EXTRA_DIST = datei.h gedcomi.h

