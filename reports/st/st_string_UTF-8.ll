/*
 * @version        1.0
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate string functions on UTF-8
*/

char_encoding("UTF-8")

require("lifelines-reports.version:1.3")
option("explicitvars") /* Disallow use of undefined variables */
include("st_aux")

/* entry point in case not invoked via st_all.ll */
proc main()
{
	call testStrings_UTF_8()
}

/*
 test some string functions against defined & undefined strings
  */
proc testStrings_UTF_8()
{
	call initSubsection()

	call testBothWays_UTF_8("ä", "Ä", "adia")
	call testBothWays_UTF_8("ö", "Ö", "odia")
	call testBothWays_UTF_8("æ", "Æ", "ae")
	call testBothWays_UTF_8("ǽ", "Ǽ", "ae_acute")
	call testBothWays_UTF_8("œ", "Œ", "oe")


	call reportSubsection("string UTF-8 tests")
}

proc testBothWays_UTF_8(slo, shi, sname)
{
	if (ne(upper(slo), shi)) {
		call reportfail(concat("upper(", sname, ") FAILED"))
	}
	else { incr(testok) }

	if (ne(lower(shi), slo)) {
		call reportfail(concat("lower(", sname, ") FAILED"))
	}
	else { incr(testok) }
}
