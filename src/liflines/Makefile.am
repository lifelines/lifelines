

# what are we going to build?
bin_PROGRAMS = lines

# what goes into lines?
lines_SOURCES = add.c advedit.c ask.c browse.c delete.c edit.c export.c import.c \
	lbrowse.c main.c merge.c miscutls.c newrecs.c remove.c show.c \
	screen.c swap.c tandem.c valgdcom.c

# extra stuff for lines
lines_LDADD = ../interp/libinterp.a ../gedlib/libgedcom.a \
	../stdlib/libstdlib.a ../btree/libbtree.a

INCLUDES=-I$(srcdir)/../hdrs


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


