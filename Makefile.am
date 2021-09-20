# Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I build/autotools -I build/gettext

SUBDIRS = build po src docs reports tt tests

EXTRA_DIST = .linesrc lines.cfg lines.cfg.m4 \
             AUTHORS INSTALL LICENSE NEWS README \
             ChangeLog

CLEANFILES = .linesrc lines.cfg

docdir = $(datarootdir)/doc/lifelines
doc_DATA = $(EXTRA_DIST)

PERL=perl

lines.cfg: lines.cfg.m4
	m4 -D WINDOWS $^ | $(PERL) -p -e 's/\n/\r\n/' > $@ 

.linesrc: lines.cfg.m4
	m4 -U WINDOWS $^ > $@ 
