# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines gedlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libgedcom.a

libgedcom_a_SOURCES = brwslist.c charmaps.c choose.c date.c dispfmt.c editmap.c \
	editvtab.c equaliso.c gedcom.c gengedc.c gstrings.c\
	indiseq.c init.c intrface.c keytonod.c locales.c lloptions.c\
	messages.c misc.c names.c node.c nodeutls.c place.c\
	property.c refns.c remove.c replace.c spltjoin.c translat.c\
	valid.c valtable.c xreffile.c

# $(top_srcdir)          for config.h
# $(top_srcdir)/intl     for libintl
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/intl -I$(top_srcdir)/src/hdrs

EXTRA_DIST = gedcomi.h

