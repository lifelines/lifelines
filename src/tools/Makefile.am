## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = btedit dbverify lltest

# what goes into btedit, dbverify and lltest?
btedit_SOURCES = btedit.c wprintf.c
dbverify_SOURCES = dbverify.c wprintf.c
lltest_SOURCES = lltest.c wprintf.c

# what libraries do btedit, dbverify and lltest need?
btedit_LDADD = ../btree/libbtree.a ../stdlib/libstdlib.a \
	@LIBINTL@ @LIBICONV@

dbverify_LDADD = ../gedlib/libgedcom.a ../btree/libbtree.a \
	../stdlib/libstdlib.a ../arch/libarch.a \
	@LIBINTL@ @LIBICONV@

lltest_LDADD = ../btree/libbtree.a ../stdlib/libstdlib.a \
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
# $(top_srcdir)/src/hdrs for headers

AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir)/src/hdrs

# Locale stuff for gettext
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
lltest.o: ../hdrs/standard.h ../hdrs/mystring.h ../hdrs/btree.h
