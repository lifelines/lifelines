
# AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = btedit dbverify

# what goes into btedit and dbverify?
btedit_SOURCES = btedit.c wprintf.c
dbverify_SOURCES = dbverify.c wprintf.c

# what libraries to btedit and dbverify need?
btedit_LDADD = ../btree/libbtree.a ../stdlib/libstdlib.a
dbverify_LDADD = ../gedlib/libgedcom.a ../btree/libbtree.a ../stdlib/libstdlib.a

# how do btedit and dbverify get built?
# (this is set up explicitly so they don't get $(LIBS) (no curses!)
btedit: $(btedit_OBJECTS) $(btedit_DEPENDENCIES)
	@rm -f btedit
	$(LINK) $(btedit_LDFLAGS) $(btedit_OBJECTS) $(btedit_LDADD)

dbverify: $(dbverify_OBJECTS) $(dbverify_DEPENDENCIES)
	@rm -f btedit
	$(LINK) $(dbverify_LDFLAGS) $(dbverify_OBJECTS) $(dbverify_LDADD)

INCLUDES=-I$(srcdir)/../hdrs

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
