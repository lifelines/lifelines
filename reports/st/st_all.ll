/*
 * @version        1.0
 * @author         Perry Rapp
 * @category       self-test
 * @output         mixed
 * @description    calls all self-test modules

Validates report language functions,
and optionally dumps various data to a file
(to exercise db functions).

Perry is using this for a a regression test.

Covered: strings, numbers, dates, collation locales
TODO: more conversion tests
TODO: logic
TODO: non-ASCII dates
TODO: Flag date tests for gedcom legal vs illegal

*/


char_encoding("ASCII")

require("lifelines-reports.version:1.3")
option("explicitvars") /* Disallow use of undefined variables */
include("st_string")
include("st_collate")
include("st_date")
include("st_number")
include("st_convert")
include("st_db")

global(true)
global(dbuse)

proc main()
{
	set(true,1)

	getint(alltests, "Run all tests ? (1=yes, 0=prompt)")
	getint(dbuse, "Exercise db functions ? (0=no)")
	getint(logout, "Output errors to file (0=no)")

	if (dostep(alltests, "Test collation ? (0=no)")) {
		call testCollate()
	}
	if (dostep(alltests, "Test strings ? (0=no)")) {
		call testStrings()
	}
	if (dostep(alltests, "Test numbers ? (0=no)")) {
		call testNums()
	}
	if (dostep(alltests, "Test dates ? (0=no)")) {
		call testDates()
	}
	if (dostep(alltests, "Test codeset conversion ? (0=no)")) {
		call testConvert()
	}

	if (dbuse) 
	{
	  call exerciseDb()
	}
}

/* should we perform this step ? prompt if not doing all */
func dostep(alltests, prompt)
{
	if (alltests) { return(true) }
	getint(doit, prompt)
	return(doit)
}




