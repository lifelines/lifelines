## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

lib_LIBRARIES= libarch.a

libarch_a_SOURCES=
libarch_a_DEPENDENCIES = @LIBOBJS@
libarch_a_LIBADD = @LIBOBJS@

INCLUDES=-I$(srcdir)/.. -I$(srcdir)/../hdrs

#need explicit dependencies
scandir.o: ../hdrs/arch.h
