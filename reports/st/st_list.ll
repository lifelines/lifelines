/*
 * @version        1.01 (2002-12-17-0600)
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate list functions
*/

char_encoding("ASCII")

require("lifelines-reports.version:1.3")
option("explicitvars") /* Disallow use of undefined variables */
include("st_aux")

/* entry point in case not invoked via st_all.ll */
proc main()
{
	call testLists()
}

/*
 test some list functions
  */
proc testLists()
{
	call initSubsection()

	list(li)
	if (not(empty(li))) {
		call reportfail("empty FAILED")
	}
	else { incr(testok) }
	enqueue(li, 1)
	if (empty(li)) {
		call reportfail("not empty FAILED")
	}
	else { incr(testok) }
	set(te, dequeue(li))
	if (ne(te, 1)) {
		call reportfail("dequeue(1) FAILED")
	}
	else { incr(testok) }
/* enqueue & dequeue */
	enqueue(li, 100)
	enqueue(li, 200)
	set(te, dequeue(li))
	if (ne(te, 100)) {
		call reportfail("dequeue(100) FAILED")
	}
	else { incr(testok) }
	set(te, dequeue(li))
	if (ne(te, 200)) {
		call reportfail("dequeue(200) FAILED")
	}
	else { incr(testok) }
	if (not(empty(li))) {
		call reportfail("empty (enqueue & dequeue) FAILED")
	}
	else { incr(testok) }
/* requeue */
	enqueue(li, 10)
	enqueue(li, 20)
	set(te, dequeue(li))
	if (ne(te, 10)) {
		call reportfail("dequeue(10) FAILED")
	}
	else { incr(testok) }
	requeue(li, 8)
	set(te, dequeue(li))
	if (ne(te, 8)) {
		call reportfail("dequeue(8) FAILED")
	}
	else { incr(testok) }
	dequeue(li)
	if (not(empty(li))) {
		call reportfail("empty (requeue) FAILED")
	}
	else { incr(testok) }
/* push & pop */
	push(li, 1)
	push(li, 2)
	set(te, pop(li))
	if (ne(te, 2)) {
		call reportfail("pop(2) FAILED")
	}
	else { incr(testok) }
	set(te, pop(li))
	if (ne(te, 1)) {
		call reportfail("pop(1) FAILED")
	}
	else { incr(testok) }
	if (not(empty(li))) {
		call reportfail("empty (push&pop) FAILED")
	}
	else { incr(testok) }
/* getel & setel */
	enqueue(li, 1)
	enqueue(li, 2)
	set(te, getel(li, 2))
	if (ne(te, 2)) {
		call reportfail("getel(,2)==2 FAILED")
	}
	else { incr(testok) }
	setel(li, 4, 4)
	set(te, dequeue(li))
	if (ne(te, 1)) {
		call reportfail("dequeue(1) from setel FAILED")
	}
	else { incr(testok) }
	set(te, dequeue(li))
	if (ne(te, 2)) {
		call reportfail("dequeue(2) from setel FAILED")
	}
	else { incr(testok) }
	set(te, dequeue(li))
	if (ne(te, 0)) {
		/* the 3rd was uninitialized created by setel */
		call reportfail("dequeue(3) from setel FAILED")
	}
	else { incr(testok) }
	set(te, dequeue(li))
	if (ne(te, 4)) {
		call reportfail("dequeue(4) from setel FAILED")
	}
	else { incr(testok) }
	if (not(empty(li))) {
		call reportfail("empty (getel & setel) FAILED")
	}
	else { incr(testok) }

    call reportSubsection("list tests")
}

