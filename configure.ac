dnl
dnl Configure script for lifelines software
dnl
dnl Process this file with autoconf to produce a configure script.

dnl **************************************************************
dnl Autoconf Preamble
dnl **************************************************************

AC_INIT(src/liflines/main.c)
AC_PREREQ(2.50)
AC_REVISION([for lifelines, built with autoconf] AC_ACVERSION)
AC_CONFIG_AUX_DIR(build/autotools)
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(lifelines, 3.0.62)

AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl **************************************************************
dnl Check for Compile-Chain Programs
dnl **************************************************************

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL

dnl MTE: 01/11/04: Force the use of GNU Bison only.
dnl MTE: AC_PROG_YACC will find any YACC, but only
dnl MTE: Bison works for us.
AC_PROG_YACC
if test "$YACC" != "bison -y";
then
  echo "LifeLines requires GNU Bison to compile src/interp/yacc.y."
  exit
fi

dnl **************************************************************
dnl Check for Internationalization Support (gettext and iconv)
dnl **************************************************************

dnl Internationalization (gettext)
dnl ALL_LINGUAS is not needed with latest autotools
dnl but debian maintainer still needs it
dnl These languages should be a copy of po/LINGUAS
ALL_LINGUAS="da de el eo es fr it nl pl rw sv"
AM_GNU_GETTEXT(,need-ngettext)

dnl codeset conversion library
AM_ICONV

dnl **************************************************************
dnl Apply OS/Compiler-Specific Settings
dnl **************************************************************

dnl Add warning flags when using The GNU C Compiler
if test "${ac_cv_prog_gcc}" = "yes"; then
  # -ansi breaks compile of liflines/askprogram.c [pere 2000-12-31]
  for flag in \
	-W \
	-Wall \
	-Wcast-align \
	-Wmissing-declarations \
	-Wmissing-prototypes \
	-Wreturn-type \
	-Wstrict-prototypes \
	-pedantic
  do
    JAPHAR_GREP_CFLAGS($flag,    [ CFLAGS="$CFLAGS $flag"    ])
  done

  dnl Add debugging as well
  CFLAGS="${CFLAGS} -g"
fi

# OS-Specific Fixups
# Cygwin - fixes linker problems
# Darwin - will pick up Fink-installed headers and libraries
case $host in
  *pc-cygwin*)
    CFLAGS="${CFLAGS} -DBROKEN_LINKER"
    ;;  
  *apple-darwin*)
    CPPFLAGS="${CPPFLAGS} -I/sw/include";
    LDFLAGS="${LDFLAGS} -L/sw/lib"
    ;;
esac

dnl **************************************************************
dnl Handle Docs Target
dnl **************************************************************

# Shall we build the docs?
AC_ARG_WITH(docs,
    [  --with-docs             Build the docs (requires sgmltools) ],
    [with_docs=$withval],
    [with_docs=no]
)

DOCS_TARGET=""
if test "$with_docs" = "yes"
then
   	echo Looking for sgmltools
	AC_PATH_PROG(SGMLTOOLS, sgmltools, FAIL)
	if test "$SGMLTOOLS" = "FAIL"; then
		AC_MSG_RESULT( ******************************************************************* )
		AC_MSG_RESULT( *** Cannot find sgmltools to build docs. skipping building docs *** )
		AC_MSG_RESULT( *** Visit http://sgmltools-lite.sourceforge.net and install it. *** )
		AC_MSG_RESULT( ******************************************************************* )
	else
		DOCS_TARGET="simple"

	fi
fi

AC_SUBST(DOCS_TARGET)

dnl **************************************************************
dnl Handle Profiling Target
dnl **************************************************************

# Compile with profiling, to find bottlenecks
AC_ARG_WITH(profiling,
  [  --with-profiling        Compile with profiling support],
  [# Does it work for other compilers then GCC? [pere 2000-12-30]
   if test "${ac_cv_prog_gcc}" = "yes"; then
      CFLAGS="${CFLAGS} -pg -a"
      LDFLAGS="${LDFLAGS} -pg -a"

      # Make sure 'config.h' is changed if profiling is turned on, to
      # trigger recompile for every source file.
      AC_DEFINE(PROFILING, 1, [Profiling enabled?])
   else
      echo "Do not know how to perform profiling using this compiler!"
   fi
  ]
)

dnl **************************************************************
dnl Check for Standard Headers/Structures/Libraries
dnl **************************************************************

echo Looking for header files
AC_CHECK_HEADERS( getopt.h dirent.h pwd.h locale.h windows.h )
AC_CHECK_HEADERS( wchar.h wctype.h )
AC_CHECK_HEADERS( math.h )
AC_CHECK_HEADERS( ncurses/curses.h ncursesw/curses.h )

echo Looking for library functions
AC_CHECK_FUNCS( _vsnprintf heapwalk _heapwalk getpwuid setlocale )
AC_CHECK_FUNCS( wcscoll towlower towupper iswspace iswalpha)
AC_CHECK_LIB( m, sin )
AC_CHECK_LIB( m, cos )
AC_CHECK_LIB( m, tan )
AC_CHECK_LIB( m, asin )
AC_CHECK_LIB( m, acos )
AC_CHECK_LIB( m, atan )

echo Looking for curses-compatible screen output library
AC_CHECK_LIB(ncursesw, tparm)
if test "$ac_cv_lib_ncursesw_tparm" = "yes"; then
  echo Using ncursesw
else
AC_CHECK_LIB(ncurses, tparm)
if test "$ac_cv_lib_ncurses_tparm" = "yes"; then
  echo Using ncurses
else
  echo Looking for curses
  AC_CHECK_LIB(curses, main)
  if test "$ac_cv_lib_curses_main" = "yes"; then
    echo Using curses
  else
    case $host in
      *mingw32*)
        echo "Didn't find an up-to-date ncurses or curses library!"
        echo "Using LifeLines version of curses for Windows."
      ;;
      *)
        echo "Didn't find an up-to-date ncurses or curses library!"
        echo "Link problems may result!"
      ;;
    esac
  fi
fi
fi

dnl Check for replacement functions
AC_REPLACE_FUNCS( sleep scandir alphasort getopt snprintf vsnprintf )
AC_REPLACE_FUNCS( nl_langinfo wcslen has_key )
AC_REPLACE_FUNCS( _llnull )

dnl **************************************************************
dnl Platform/Version-Specific Checks
dnl **************************************************************

echo Looking for header files and libraries to support heap walking
dnl We need to cheat on OSX/Darwin.  The standard configure
dnl check brings in a C++ header which bombs and causes
dnl configure to display a particularly nasty warning, which
dnl we don't want to expose to end-users.
case $host in
  *apple-darwin*)
    HAVE_ALLOC_H=no
    HAVE_MALLOC_H=no
    HAVE_HEAPWALK=no
    HAVE__HEAPWALK=no
    ;;
  *)
    AC_CHECK_HEADERS( alloc.h malloc.h )
    AC_CHECK_FUNCS( heapwalk _heapwalk )
    ;;
esac

dnl ngettext function missing from 0.10.35
echo Looking for ngettext
saved_libs="$LIBS"
LIBS="$LIBS $LIBINTL"
AC_TRY_LINK([#include <libintl.h>], [ngettext("","",0);],
    ac_cv_func_ngettext=yes, ac_cv_func_ngettext=no)
  if test "$ac_cv_func_ngettext" = yes; then
    echo "found ngettext"
    AC_DEFINE(HAVE_NGETTEXT, 1, [Define if you have the ngettext() function.])
  else
    echo "couldn't find ngettext"
    LIBS="$saved_libs"
  fi

case $host_os in
  *windows*)
    AC_DEFINE(LINES_CONFIG_FILE,"lines.cfg",[lifelines configuration file])
    ;;
  *)
    AC_DEFINE(LINES_CONFIG_FILE,".linesrc",[lifelines configuration file])
    ;;
esac

dnl bind_textdomain_codeset function missing from 0.10.35
echo Looking for bind_textdomain_codeset
saved_libs="$LIBS"
LIBS="$LIBS $LIBINTL"
AC_TRY_LINK([#include <libintl.h>], [bind_textdomain_codeset("","");],
    ac_cv_func_bind_textdomain_codeset=yes, ac_cv_func_bind_textdomain_codeset=no)
  if test "$ac_cv_func_bind_textdomain_codeset" = yes; then
    echo "found bind_textdomain_codeset"
    AC_DEFINE(HAVE_BIND_TEXTDOMAIN_CODESET, 1, [Define if you have the bind_textdomain_codeset() function.])
  else
    echo "couldn't find bind_textdomain_codeset"
    LIBS="$saved_libs"
  fi

dnl **************************************************************
dnl Build Makefiles
dnl **************************************************************

AC_OUTPUT(Makefile \
        src/Makefile \
        src/arch/Makefile \
        src/btree/Makefile \
        src/interp/Makefile \
        src/gedlib/Makefile \
        src/stdlib/Makefile \
        src/liflines/Makefile \
        src/tools/Makefile \
        src/ui/Makefile \
        src/hdrs/Makefile \
        src/hdrs/win32/Makefile \
        docs/Makefile \
        reports/Makefile \
        reports/st/Makefile \
        tt/Makefile \
        win32/Makefile \
        build/Makefile \
        intl/Makefile \
        po/Makefile.in)
