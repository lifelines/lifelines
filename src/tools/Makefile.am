## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = btedit dbverify

# what goes into btedit and dbverify?
btedit_SOURCES = btedit.c wprintf.c
dbverify_SOURCES = dbverify.c wprintf.c

# what libraries to btedit and dbverify need?
btedit_LDADD = ../btree/libbtree.a ../stdlib/libstdlib.a
dbverify_LDADD = ../gedlib/libgedcom.a ../btree/libbtree.a ../stdlib/libstdlib.a ../arch/libarch.a \
	@INTLLIBS@

# how do btedit and dbverify get built?
# (this is set up explicitly so they don't get $(LIBS) (no curses!)
btedit$(EXEEXT): $(btedit_OBJECTS) $(btedit_DEPENDENCIES)
	@rm -f btedit$(EXEEXT)
	$(LINK) $(btedit_LDFLAGS) $(btedit_OBJECTS) $(btedit_LDADD)

dbverify$(EXEEXT): $(dbverify_OBJECTS) $(dbverify_DEPENDENCIES)
	@rm -f dbverify$(EXEEXT)
	$(LINK) $(dbverify_LDFLAGS) $(dbverify_OBJECTS) $(dbverify_LDADD)

# .. for config.h, hdrs for lots of stuff, ../intl for libgnuintl.h
INCLUDES=-I$(srcdir)/../hdrs -I$(srcdir)/../intl

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
