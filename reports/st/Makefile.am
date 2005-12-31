# This makefile is for the lifelines reports

AUTOMAKE_OPTIONS = no-dependencies

# SELFTEST_REPORTS is to hold all parts of the self-test scripts
SELFTEST_REPORTS = st_all.ll  \
	st_aux.li             \
	st_collate.li         \
	st_collate_8859-1.li  \
	st_collate_UTF-8.li   \
	st_convert.li         \
	st_date.li            \
	st_db.li              \
	st_list.li            \
	st_name.li            \
	st_number.li          \
	st_string.li          \
	st_string_UTF-8.li    \
	st_table.li           \
	trigtest.ll
SELFTEST_REFERENCE = st_all.ref st_all_stdout.ref
TEST_ITER_REPORTS = test_forindi.ll test_forfam.ll test_indi_it.ll \
                    test_fam_it.ll test_othr_it.ll trigtest.ll
TEST_ITER_REFERENCE = test_forindi.ref test_forfam.ref test_indi_it.ref \
                    test_fam_it.ref test_othr_it.ref trigtest.ref
TEST_ITER_DB = ti.ged
TEST_OUTPUTS =test_forindi.out test_forfam.out test_indi_it.out \
                    test_fam_it.out test_othr_it.out st_all.out \
		    st_all.stdout

TESTS = selftest
pkg_REPORTS = $(SELFTEST_REPORTS) \
              $(TEST_ITER_REPORTS)
EXTRA_DIST =  $(SELFTEST_REPORTS) $(SELFTEST_REFERENCE) \
              $(TEST_ITER_REPORTS) $(TEST_ITER_REFERENCE) $(TEST_ITER_DB)
CLEANFILES =  $(TEST_OUTPUTS) errs.log llines.leak_log selftest

LLEXEC = ../../src/liflines/llexec
LLINES = ../../src/liflines/llines

.PHONY: local test_iter st_all selftest
selftest: ti test_iter st_all

local: $(TEST_ITER_DB) $(TEST_ITER_REPORTS) $(SELFTEST_REPORTS)
	ln -fs /bin/true selftest 
	for i in $? ; do \
	    dest=`basename $$i` ;\
	    if [ $$dest != $$i ] ; then \
		cp $$i $$dest ; \
	    fi ;\
	done

ti: local ti.ged $(LLINES)
	rm -rf ti
	(echo yurti ; echo yyq) | $(LLINES) ./ti  > /dev/null

test_iter: $(TEST_ITER_REPORTS) $(TEST_ITER_REFERENCE) $(TEST_ITER_DB) $(LLEXEC)
	@for i in $(TEST_ITER_REPORTS) ; do \
	    this=`basename $$i .ll` ;\
	    echo "$(LLEXEC) ./ti -x  ./$$this.ll > $$this.out" ;\
	    $(LLEXEC) ./ti -x  ./$$this.ll > $$this.out;\
	    if diff $$this.out $(srcdir)/$$this.ref >/dev/null ; then\
	        : echo "ok" ; \
	    else \
	        echo "test $$i failed - to see failure execute" ; \
		echo "diff $$this.out $(srcdir)/$$this.ref" ; \
		ln -fs /bin/false selftest ;\
	    fi \
	done

st_all: $(SELFTEST_REPORTS) $(LLEXEC)
	(echo 1; echo 1 ;echo 0 ; echo st_all.out) | \
	      $(LLEXEC) ./ti -x ./st_all.ll > st_all.stdout
	@if diff st_all.out $(srcdir)/st_all.ref >/dev/null ; then\
	        : echo "test st_all output ok" ; \
	    else \
	        echo "test st_all output failed - to see failure execute" ; \
	        echo "diff st_all.out $(srcdir)/st_all.ref" ; \
		ln -fs /bin/false selftest ;\
	    fi
	@if diff st_all.stdout $(srcdir)/st_all_stdout.ref >/dev/null ; then\
	        : echo "test st_all stdout ok" ; \
	    else \
	        echo "test st_all failed - to see failure execute" ; \
	        echo "diff st_all.stdout $(srcdir)/st_all_stdout.ref" ; \
		ln -fs /bin/false selftest ;\
	    fi 
