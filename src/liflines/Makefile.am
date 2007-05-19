# Process this file with automake to produce Makefile.in
# (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = llines llexec

# what goes into lines?
llines_SOURCES = add.c advedit.c ask.c askgedc.c askprogram.c \
	browse.c brwsmenu.c \
	cscurses.c delete.c dynmenu.c \
	edit.c error.c export.c import.c interact.c \
	lbrowse.c lines_usage.c listui.c loadsave.c \
	main.c menuset.c merge.c miscutls.c \
	newrecs.c pedigree.c \
	scan.c screen.c searchui.c selectdb.c show.c swap.c \
	tandem.c valgdcom.c \
	listui.h llinesi.h 

# what goes into llexec ?
llexec_SOURCES = ask.c error.c lines_usage.c llexec.c selectdb.c llexec.h

# extra stuff for lines
llines_LDADD = ../interp/libinterp.a ../gedlib/libgedcom.a \
	../btree/libbtree.a ../stdlib/libstdlib.a ../arch/libarch.a \
	@LIBINTL@ @LIBICONV@

# extra stuff for llexec
llexec_LDADD = ../interp/libinterp.a ../gedlib/libgedcom.a \
	../btree/libbtree.a ../stdlib/libstdlib.a ../arch/libarch.a \
	../ui/libuicli.a \
	@LIBINTL@ @LIBICONV@

# $(top_builddir)        for config.h
# $(top_srcdir)/intl     for intl headers
# $(top_builddir)/intl   for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_builddir) -I$(top_srcdir)/intl -I$(top_builddir)/intl -I$(top_srcdir)/src/hdrs

# Stuff for gettext (copied from gnu hello-1.3.35)
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
# End stuff for gettext

EXTRA_DIST = llinesi.h screeni.h


# these dependencies are so that building parts ('make btedit' or 'make
# lines') will work

../btree/libbtree.a :
	$(MAKE) -C ../btree libbtree.a

../interp/libinterp.a :
	$(MAKE) -C ../interp libinterp.a

../gedlib/libgedcom.a :
	$(MAKE) -C ../gedlib libgedcom.a

../stdlib/libstdlib.a :
	$(MAKE) -C ../stdlib libstdlib.a

../ui/libuicli.a :
	$(MAKE) -C ../ui libuicli.a

# since we're not doing dependencies automagically
add.o: ../hdrs/standard.h
add.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/translat.h
advedit.o: ../hdrs/standard.h
advedit.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
advedit.o: ../hdrs/translat.h
ask.o: ../hdrs/standard.h
ask.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/indiseq.h
askprogram.o: ../hdrs/arch.h ../hdrs/gedcom.h ../hdrs/indiseq.h
askprogram.o: ../hdrs/liflines.h ../hdrs/lloptions.h ../hdrs/standard.h
askprogram.o: ../hdrs/screen.h ../hdrs/table.h
browse.o: ../hdrs/standard.h
browse.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/indiseq.h
browse.o: ../hdrs/gedcom.h ../hdrs/translat.h
delete.o: ../hdrs/standard.h
delete.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
edit.o: ../hdrs/standard.h
edit.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
edit.o: ../hdrs/translat.h
error.o: ../hdrs/standard.h ../hdrs/mystring.h
export.o: ../hdrs/btree.h ../hdrs/gedcom.h ../hdrs/lloptions.h
export.o: ../hdrs/mystring.h ../hdrs/standard.h ../hdrs/table.h
export.o: ../hdrs/translat.h
import.o: ../hdrs/gedcheck.h ../hdrs/gedcom.h ../hdrs/lloptions.h
import.o: ../hdrs/mystring.h ../hdrs/screen.h ../hdrs/sequence.h
import.o: ../hdrs/standard.h ../hdrs/table.h ../hdrs/translat.h
lbrowse.o: ../hdrs/standard.h
lbrowse.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
lbrowse.o: ../hdrs/indiseq.h
main.o: ../hdrs/btree.h ../hdrs/gedcom.h ../hdrs/lloptions.h
main.o: ../hdrs/mystring.h ../hdrs/screen.h ../hdrs/standard.h
main.o: ../hdrs/table.h
menuitem.o: ../hdrs/standard.h ../hdrs/menuitem.h
merge.o: ../hdrs/standard.h
merge.o: ../hdrs/mystring.h ../hdrs/btree.h ../hdrs/table.h ../hdrs/gedcom.h
merge.o: ../hdrs/translat.h
miscutls.o: ../hdrs/standard.h
miscutls.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
miscutls.o: ../hdrs/translat.h
newrecs.o: ../hdrs/standard.h
newrecs.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
newrecs.o: ../hdrs/translat.h ../hdrs/btree.h
pedigree.o: ../hdrs/llstdlib.h ../hdrs/screen.h ../hdrs/mystring.h
scan.o: ../hdrs/sys_inc.h ../hdrs/llstdlib.h ../hdrs/table.h ../hdrs/indiseq.h
scan.o: ../hdrs/interp.h ../hdrs/screen.h ../hdrs/liflines.h ../hdrs/fpattern.h
scan.o: llinesi.h
screen.o: ../hdrs/standard.h
screen.o: ../hdrs/mystring.h ../hdrs/screen.h
screen.o: ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/indiseq.h
screen.o: ../hdrs/menuitem.h
show.o: ../hdrs/standard.h
show.o: ../hdrs/mystring.h ../hdrs/screen.h
show.o: ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/indiseq.h ../hdrs/translat.h
show.o: ../hdrs/date.h ../hdrs/cache.h
swap.o: ../hdrs/standard.h
swap.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
tandem.o: ../hdrs/standard.h
tandem.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
tandem.o: ../hdrs/indiseq.h
valgdcom.o: ../hdrs/standard.h
valgdcom.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
valgdcom.o: ../hdrs/gedcheck.h ../hdrs/translat.h

