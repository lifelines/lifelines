
AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = btedit

# what goes into btedit?
btedit_SOURCES = btedit.c wprintf.c

# extra stuff for btedit
btedit_LDADD = ../btree/libbtree.a

# this is set up explicitly so btedit doesn't get $(LIBS) (doesn't need curses)
btedit: $(btedit_OBJECTS) $(btedit_DEPENDENCIES)
	@rm -f btedit
	$(LINK) $(btedit_LDFLAGS) $(btedit_OBJECTS) $(btedit_LDADD)

INCLUDES=-I$(srcdir)/../hdrs


# these dependencies are so that building parts ('make btedit' or 'make
# lines') will work

../btree/libbtree.a :
	$(MAKE) -C ../btree libbtree.a


