## Makefile.am -- Run tests in the tests subdirectory

EXTRA_DIST              = defs $(TESTS)
MAINTAINERCLEANFILES    = Makefile.in
SHELL                   = /bin/bash

testsubdir              = date gengedcomstrong interp math pedigree-longname \
                          string view-history Royal92

TESTS_ENVIRONMENT       = top_builddir=$(top_builddir)

# Tests that have no external dependencies
TESTS1  =               date/checkjd2date.llscr         \
			gengedcomstrong/test1.llscr     \
			interp/eqv_pvalue.llscr         \
			interp/fullname.llscr           \
			interp/ifelse.llscr             \
			math/test1.llscr                \
			math/test2.llscr                \
			pedigree-longname/test1.llscr   \
			string/string-unicode.llscr     \
			string/mc_llexec.llscr          \
			string/mc_llexec1.llscr         \
			string/mc_llines.llscr          \
			string/mc_llines1.llscr         \
			view-history/view-history.llscr \
			Royal92/Royal92.llscr           \
			Royal92/Analyze.llscr           \
			Royal92/check_lltest.llscr 

# Tests that depend on 'expect'
if HAVE_EXPECT
TESTS2	=		string/mc_llines2.llscr
endif

TESTS	=		$(TESTS1) $(TESTS2)

# make runs run_a_test file.llscr to generate file.log
TEST_EXTENSIONS = .llscr
LLSCR_LOG_COMPILER = $(srcdir)/run_a_test

clean-local:
	-rm -f */*.llout */*.out */*.stdout */*.diff */*.fix
	-rm -rf */testdb */*.filter */*.valgrind */errs.log
