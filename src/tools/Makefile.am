## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = btedit dbverify

# what goes into btedit and dbverify?
btedit_SOURCES = btedit.c wprintf.c
dbverify_SOURCES = dbverify.c wprintf.c

# what libraries to btedit and dbverify need?
btedit_LDADD = ../btree/libbtree.a ../stdlib/libstdlib.a \
	@LIBINTL@ @LIBICONV@
dbverify_LDADD = ../gedlib/libgedcom.a ../btree/libbtree.a \
	../stdlib/libstdlib.a ../arch/libarch.a \
	@LIBINTL@ @LIBICONV@

# 2002.06.29 Perry commented these out -- we don't need them I think
# how do btedit and dbverify get built?
# (this is set up explicitly so they don't get $(LIBS) (no curses!)
# btedit$(EXEEXT): $(btedit_OBJECTS) $(btedit_DEPENDENCIES)
#	@rm -f btedit$(EXEEXT)
#	$(LINK) $(btedit_LDFLAGS) $(btedit_OBJECTS) $(btedit_LDADD)
#
#dbverify$(EXEEXT): $(dbverify_OBJECTS) $(dbverify_DEPENDENCIES)
#	@rm -f dbverify$(EXEEXT)
#	$(LINK) $(dbverify_LDFLAGS) $(dbverify_OBJECTS) $(dbverify_LDADD)

# $(top_builddir)        for config.h
# $(top_srcdir)/intl     for intl headers
# $(top_builddir)/intl             for generated libintl.h
# $(top_srcdir)/src/hdrs for headers
INCLUDES=-I$(top_builddir) -I$(top_srcdir)/intl -I$(top_builddir)/intl -I$(top_srcdir)/src/hdrs

# Stuff for gettext (copied from gnu hello-1.3.35)
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
# End stuff for gettext

# these dependencies are so that building parts ('make btedit' or 'make
# lines') will work

../btree/libbtree.a :
	$(MAKE) -C ../btree libbtree.a

../stdlib/libstdlib.a :
	$(MAKE) -C ../stdlib libstdlib.a

../gedlib/libgedcom.a :
	$(MAKE) -C ../gedlib libgedcom.a

# since we're not doing dependencies automagically...
btedit.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
dbverify.o: ../hdrs/standard.h ../hdrs/btree.h ../hdrs/gedcom.h
