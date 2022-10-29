dnl
dnl Configure script for lifelines software
dnl
dnl Process this file with autoconf to produce a configure script.

dnl **************************************************************
dnl Autoconf Section
dnl **************************************************************

AC_INIT(lifelines, 3.2.0)
AC_PREREQ(2.50)
AC_REVISION([for lifelines, built with autoconf] AC_ACVERSION)
AC_CONFIG_AUX_DIR(build/autotools)
AC_CANONICAL_HOST

dnl **************************************************************
dnl Automake Section
dnl **************************************************************

dnl Use 'foreign' to avoid requiring GNU files in top-level.
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl **************************************************************
dnl Internationalization Section
dnl **************************************************************

dnl gettext
AM_GNU_GETTEXT([external],[need-ngettext])
AM_GNU_GETTEXT_VERSION([0.11.5])

dnl iconv
AM_ICONV

dnl langinfo
AM_LANGINFO_CODESET

dnl **************************************************************
dnl Compile-Chain Section
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
dnl OS/Compiler-Specific Section
dnl **************************************************************

dnl Add warning flags when using The GNU C Compiler
if test "${ac_cv_prog_gcc}" = "yes"; then
  CFLAGS="${CFLAGS} -W -Wall -Wextra"
  CFLAGS="${CFLAGS} -Wcast-align -Wmissing-declarations -Wmissing-prototypes"
  CFLAGS="${CFLAGS} -Wreturn-type -Wstrict-prototypes -pedantic"

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

dnl Specify bit width if requested
AC_ARG_WITH(32bit,
    [  --with-32bit             Build in 32-bit mode ],
    [build_32bit=$withval],
    [build_32bit=no])

AC_ARG_WITH(64bit,
    [  --with-64bit             Build in 64-bit mode ],
    [build_64bit=$withval],
    [build_64bit=no])

if test "$build_32bit" = "yes"
then
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="${CFLAGS} -m32"
    LDFLAGS="${LDFLAGS} -m32"
  else
    echo "The use of --with-32bit is only supported when using gcc!"
    exit
  fi
fi

if test "$build_64bit" = "yes"
then
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="${CFLAGS} -m64"
    LDFLAGS="${LDFLAGS} -m64"
  else
    echo "The use of --with-64bit is only supported when using gcc!"
    exit
  fi
fi

dnl Specify code coverage if requested
AC_ARG_WITH(coverage,
    [  --with-coverage             Build with code coverage ],
    [build_coverage=$withval],
    [build_coverage=no])

if test "$build_coverage" = "yes"
then
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="${CFLAGS} --coverage"
  else
    echo "The use of --with-coverage is only supported when using gcc!"
    exit
  fi
fi

dnl Enable full debugging
AC_ARG_WITH(full-debug,
    [  --with-full-debug           Build with full debug support ],
    [build_full_debug=$withval],
    [build_full_debug=no])

if test "$build_full_debug" = "yes"
then
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="${CFLAGS} -rdynamic"
  else
    echo "The use of --with-full-debug only supported when using gcc!"
    exit
  fi
fi

dnl **************************************************************
dnl Handle testing requirements
dnl **************************************************************

dnl Some tests need the 'expect' program.
echo "Looking for expect"
AC_PATH_PROG(TOOL_EXPECT, expect, FAIL)
if test "$TOOL_EXPECT" = "FAIL"; then
    AC_MSG_RESULT( ************************************************************************ )
    AC_MSG_RESULT( ******** Cannot find expect.  Some tests will be skipped. ************** )
    AC_MSG_RESULT( ************************************************************************ )
fi

AM_CONDITIONAL(HAVE_EXPECT, test "$TOOL_EXPECT" != "FAIL")

dnl **************************************************************
dnl Handle Docs Target
dnl **************************************************************

# Shall we build the docs?
AC_ARG_WITH(docs,
    [  --with-docs             Build the docs from source (requires xmlto and dblatex) ],
    [build_docs=$withval],
    [build_docs=no])

if test "$build_docs" = "yes"
then
    echo "Looking for xmlto"
    AC_PATH_PROG(TOOL_XMLTO, xmlto, FAIL)
    if test "$TOOL_XMLTO" = "FAIL"; then
        AC_MSG_RESULT( ************************************************************************ )
        AC_MSG_RESULT( ******** Cannot find xmlto to build docs.  Will not build docs. ******** )
        AC_MSG_RESULT( ************************************************************************ )
        build_docs=no
    fi
    AC_PATH_PROG(TOOL_DBLATEX, dblatex, FAIL)
    if test "$TOOL_DBLATEX" = "FAIL"; then
        AC_MSG_RESULT( ************************************************************************ )
        AC_MSG_RESULT( ******* Cannot find dblatex to build docs.  Will not build docs. ******* )
        AC_MSG_RESULT( ************************************************************************ )
        build_docs=no
    fi
fi

AM_CONDITIONAL(BUILD_DOCS, test "$build_docs" = "yes")

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

echo Looking for library functions
AC_CHECK_FUNCS( _vsnprintf getpwuid setlocale )
AC_CHECK_FUNCS( wcscoll towlower towupper iswspace iswalpha )
AC_CHECK_FUNCS( backtrace backtrace_symbols )
AC_SEARCH_LIBS( sin, m )
AC_SEARCH_LIBS( cos, m )
AC_SEARCH_LIBS( tan, m )
AC_SEARCH_LIBS( asin, m )
AC_SEARCH_LIBS( acos, m )
AC_SEARCH_LIBS( atan, m )

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

dnl **************************************************************
dnl Check for curses
dnl **************************************************************

dnl Since some distributions package wide and narrow functions
dnl in separate libraries, we need to check for them separately.
dnl We also need to search for a wide character routine, so that
dnl we can warn if only a narrow version of curses is found.
dnl
dnl We search for libraries first, since its easier to tell if
dnl the right libraries are installed than the right headers.

echo Looking for curses libraries
AC_CHECK_LIB(ncursesw, in_wch)
if test "$ac_cv_lib_ncursesw_in_wch" = "yes"; then
  echo Using ncursesw
else
  AC_CHECK_LIB(ncurses, in_wch)
  if test "$ac_cv_lib_ncurses_in_wch" = "yes"; then
    echo Using ncurses
  else
    AC_CHECK_LIB(cursesw, in_wch)
    if test "$ac_cv_lib_cursesw_in_wch" = "yes"; then
      echo Using cursesw
    else
      AC_CHECK_LIB(curses, in_wch)
      if test "$ac_cv_lib_curses_in_wch" = "yes"; then
        echo Using curses
      else
        case $host in
          *mingw32*)
            echo "Using LifeLines version of curses for Windows."
          ;;
          *)
            echo "LifeLines requires a curses implementation with wide character support!"
            exit
          ;;
        esac
      fi
    fi
  fi
fi

dnl Check for headers
echo Looking for curses headers
AC_CHECK_HEADERS( ncursesw/curses.h ncurses/curses.h curses.h )

dnl **************************************************************
dnl Check for Replacement Functions for Standard Libraries/Curses
dnl **************************************************************

echo Checking for replacement functions
dnl Standard libraries
AC_REPLACE_FUNCS( sleep scandir alphasort getopt snprintf vsnprintf )
dnl Standard libraries - internationalization / wchar
AC_REPLACE_FUNCS( nl_langinfo wcslen )
dnl Standard libraries - curses
AC_REPLACE_FUNCS( has_key )

dnl **************************************************************
dnl Platform-Specific Configuration
dnl **************************************************************

case $host_os in
  *windows*)
    AC_DEFINE(LINES_CONFIG_FILE,"lines.cfg",[lifelines configuration file])
    ;;
  *)
    AC_DEFINE(LINES_CONFIG_FILE,".linesrc",[lifelines configuration file])
    ;;
esac

dnl **************************************************************
dnl Generate Makefiles
dnl **************************************************************

AC_OUTPUT(Makefile \
        src/Makefile \
        src/arch/Makefile \
        src/arch/mswin/Makefile \
        src/btree/Makefile \
        src/interp/Makefile \
        src/gedlib/Makefile \
        src/stdlib/Makefile \
        src/liflines/Makefile \
        src/tools/Makefile \
        src/ui/Makefile \
        src/hdrs/Makefile \
        src/hdrs/mswin/Makefile \
        docs/man/Makefile \
        docs/manual/Makefile \
        docs/editor/Makefile \
        docs/Makefile \
        reports/Makefile \
        reports/desc-tex2/Makefile \
        reports/novel/Makefile \
        reports/pedtex/Makefile \
        reports/ps-fan/Makefile \
        reports/st/Makefile \
        tt/Makefile \
        build/Makefile \
        tests/Makefile \
        po/Makefile.in)
