
AUTOMAKE_OPTIONS = no-dependencies

# what are we going to build?
bin_PROGRAMS = llines

# what goes into lines?
llines_SOURCES = add.c advedit.c ask.c browse.c delete.c edit.c \
	export.c import.c lbrowse.c main.c merge.c miscutls.c \
	newrecs.c remove.c show.c screen.c swap.c tandem.c valgdcom.c \
	llinesi.h askprogram.c

# extra stuff for lines
llines_LDADD = ../interp/libinterp.a ../gedlib/libgedcom.a \
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


# since we're not doing dependencies automagically
add.o: ../hdrs/standard.h
add.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/translat.h
advedit.o: ../hdrs/standard.h
advedit.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
advedit.o: ../hdrs/translat.h
ask.o: ../hdrs/standard.h
ask.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/indiseq.h
browse.o: ../hdrs/standard.h
browse.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/indiseq.h
browse.o: ../hdrs/gedcom.h ../hdrs/translat.h
delete.o: ../hdrs/standard.h
delete.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
edit.o: ../hdrs/standard.h
edit.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
edit.o: ../hdrs/translat.h
export.o: ../hdrs/standard.h
export.o: ../hdrs/mystring.h ../hdrs/btree.h ../hdrs/table.h ../hdrs/gedcom.h
export.o: ../hdrs/translat.h
import.o: ../hdrs/standard.h
import.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
import.o: ../hdrs/sequence.h ../hdrs/gedcheck.h ../hdrs/translat.h
import.o: ../hdrs/screen.h
lbrowse.o: ../hdrs/standard.h
lbrowse.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
lbrowse.o: ../hdrs/indiseq.h
main.o: ../hdrs/standard.h
main.o: ../hdrs/mystring.h ../hdrs/screen.h
main.o: ../hdrs/btree.h ../hdrs/table.h
main.o: ../hdrs/gedcom.h
merge.o: ../hdrs/standard.h
merge.o: ../hdrs/mystring.h ../hdrs/btree.h ../hdrs/table.h ../hdrs/gedcom.h
merge.o: ../hdrs/translat.h
miscutls.o: ../hdrs/standard.h
miscutls.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
miscutls.o: ../hdrs/translat.h
newrecs.o: ../hdrs/standard.h
newrecs.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
newrecs.o: ../hdrs/translat.h ../hdrs/btree.h
remove.o: ../hdrs/standard.h
remove.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
screen.o: ../hdrs/standard.h
screen.o: ../hdrs/mystring.h ../hdrs/screen.h
screen.o: ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/indiseq.h
show.o: ../hdrs/standard.h
show.o: ../hdrs/mystring.h ../hdrs/screen.h
show.o: ../hdrs/table.h ../hdrs/gedcom.h ../hdrs/indiseq.h ../hdrs/translat.h
show.o: ../hdrs/cache.h
swap.o: ../hdrs/standard.h
swap.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
tandem.o: ../hdrs/standard.h
tandem.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
tandem.o: ../hdrs/indiseq.h
valgdcom.o: ../hdrs/standard.h
valgdcom.o: ../hdrs/mystring.h ../hdrs/table.h ../hdrs/gedcom.h
valgdcom.o: ../hdrs/gedcheck.h ../hdrs/translat.h

