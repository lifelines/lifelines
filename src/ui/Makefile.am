# Makefile for the LifeLines user interface subsystem

# AUTOMAKE_OPTIONS = no-dependencies

noinst_LIBRARIES = libuicli.a libuicurses.a

libuicli_a_SOURCES = ui_cli.c

libuicurses_a_SOURCES = ui_curses.c

# $(top_srcdir)          for config.h
# $(top_srcdir)/src/hdrs for headers

AM_CPPFLAGS=-I$(top_srcdir) -I$(top_srcdir)/src/hdrs

