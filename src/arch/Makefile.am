## Process this file with automake to produce Makefile.in
## (configure will process the resulting Makefile.in to produce Makefile)

# AUTOMAKE_OPTIONS = no-dependencies

lib_LIBRARIES= libarch.a

libarch_a_SOURCES=
libarch_a_DEPENDENCIES = @LIBOBJS@
libarch_a_LIBADD = @LIBOBJS@

# .. for config.h, hdrs for lots of stuff, ../intl for libgnuintl.h
INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs -I$(srcdir)/../intl

#need explicit dependencies
alphasort.o: ../hdrs/arch.h
getopt.o: ../hdrs/arch.h
scandir.o: ../hdrs/arch.h
sleep.o: ../hdrs/arch.h
snprintf.o: ../hdrs/arch.h
vsnprintf.o: ../hdrs/arch.h
