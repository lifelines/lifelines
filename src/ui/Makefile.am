# Makefile for the LifeLines report interpreter subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libuicli.a

libuicli_a_SOURCES = ui_cli.c

# $(top_srcdir)          for config.h
# $(top_srcdir)/intl     for intl headers
# ../../intl             for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/intl -I../../intl -I$(top_srcdir)/src/hdrs

