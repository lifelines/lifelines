# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# Makefile for the LifeLines gedlib area

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libgedcom.a

libgedcom_a_SOURCES = brwslist.c charmaps.c choose.c date.c editmap.c \
	editvtab.c equaliso.c gedcom.c gengedc.c gstrings.c\
	indiseq.c init.c intrface.c keytonod.c locales.c lloptions.c\
	messages.c misc.c names.c node.c nodeutls.c place.c\
	property.c refns.c remove.c replace.c spltjoin.c translat.c\
	valid.c valtable.c xreffile.c

# .. for config.h, hdrs for lots of stuff, ../intl for libgnuintl.h
INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs -I$(srcdir)/../intl
EXTRA_DIST = gedcomi.h


