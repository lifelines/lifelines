/*=============================================================
 * lines_usage.c -- Usage display for Lifelines
 *  for main executable and for llexec
 *  generates --help output, so also used by help2man
 *===========================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "llinesi.h"


/*===============================================
 * print_lines_usage -- display program help/usage
 *  displays to stdout
 *  exename: [IN]  "Lines" or "llines" or "llexec"
 *=============================================*/
void
print_lines_usage (CNSTRING exename)
{
	if (0 == strcmp(exename, "llexec")) {
		printf(_("%s invokes the Lifelines report execution program without GUI."), exename);
	} else {
		printf(_("%s invokes the Lifelines GUI."), exename);
	}
	printf("\n\n");
	printf(_("Lifelines is a program to manipulate genealogical information\n"
		"in lineage-linked GEDCOM format. It has a curses interface and a\n"
		"built-in interpreter for its own genealogical report language.\n\n"
		"It has its own databases, and records are edited directly in GEDCOM\n"
		"(with an editor of the user's choice). Records store\n"
		"information about people, families, sources, events, and\n"
		"other types of data (such as notes). Lifelines includes a\n"
		"custom report language and comes with a collection of reports.")
		);
	printf("\n\n");
	printf(_("Usage: %s [OPTIONS] [database]"), exename);
	printf("\n\n");
	printf(_("Options:"));
	printf("\n\t");
	printf(_("-C[FILE]"));
	printf("\n\t\t");
	printf(_("Specify configuration file location"));
	printf("\n\t-F\n\t\t");
	printf(_("Finnish option (if compiled to allow Finnish option)"));
	printf("\n\t");
	printf(_("-I[KEY=VALUE]"));
	printf("\n\t\t");
	printf(_("Specify a user property (e.g. -ILLEDITOR=gvim)"));
	printf("\n\t-a\n\t\t");
	printf(_("log dynamic memory operation (for debugging)"));
	printf("\n\t");
	printf(_("-c[TYPE][DIRECT,INDIRECT]"));
	printf("\n\t\t");
	printf(_("supply cache values (eg, -ci400,4000f400,4000 sets direct indi\n"
		"\t\t& fam caches to 400, and indirect indi & fam caches to 4000)"));
	printf("\n\t-d\n\t\t");
	printf(_("debug mode (disable signal trapping)"));
	printf("\n\t-f\n\t\t");
	printf(_("force open (for recovery from errors or power failure)"));
	printf("\n\t-i\n\t\t");
	printf(_("open database with immutable access (for use on read-only media)"));
	printf("\n\t-k\n\t\t");
	printf(_("always show keys (even if REFN available)"));
	printf("\n\t-ln\n\t\t");
	printf(_("unlock a database (for correct access on read-write media)"));
	printf("\n\t-ly\n\t\t");
	printf(_("lock a database (for use on read-only media)"));
	printf("\n\t-n\n\t\t");
	printf(_("do not use traditional family rules"));
	printf("\n\t-O[FILE]\n\t\t");
	printf(_("Specify program output filename (eg, -o/tmp/mytests)"));
	printf("\n\t-r\n\t\t");
	printf(_("open database with read-only access (prohibiting other\n"
		"\t\twrite access)"));
	printf("\n\t-t\n\t\t");
	printf(_("trace function calls in report programs (for debugging)"));
	printf("\n\t");
	printf(_("-u[HEIGHT,WIDTH]"));
	printf("\n\t\t");
	printf(_("specify window size (eg, -u120,34 specifies 120 columns\n"
		"\t\tby 34 rows)"));
	printf("\n\t--help\n\t\t");
	printf(_("display this help and exit"));
	printf("\n\t-w\n\t\t");
	printf(_("open database with writeable access (this is the default)"));
	printf("\n\t");
	printf("-x[REPORT]");
	printf("\n\t\t");
	printf(_("execute a single lifelines report program directly"));
	printf("\n\t-z\n\t\t");
	printf(_("Use normal ASCII characters for drawing lines, instead of\n"
		"\t\tspecial VT100 terminal characters"));
	printf("\n\t--version\n\t\t");
	printf(_("output version information and exit"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n\t");
	if (0 == strcmp(exename, "llexec")) {
	    printf(_("%s myfamily -x eol"), exename);
	    printf("\n\t\t");
	    printf(_("Open the database 'myfamily' with Lifelines"));
	    printf("\n\t\t");
	    printf(_("and run the eol.ll report"));
	    printf("\n\t");
	    printf(_("%s -f myfamily -x eol"), exename);
	    printf("\n\t\t");
	    printf(_("Unlock database 'myfamily', after a power failure left the"));
	    printf("\n\t\t");
	    printf(_("database locked and run 'eol.ll' report"));
	    printf("\n\n");
	} else {
	    printf(_("%s myfamily"), exename);
	    printf("\n\t\t");
	    printf(_("Open database 'myfamily' with Lifelines"));
	    printf("\n\t");
	    printf(_("%s -f myfamily"), exename);
	    printf("\n\t\t");
	    printf(_("Unlock database 'myfamily', after a power failure left\n"
		    "\t\tthe database locked"));
	    printf("\n\n");
	}
	printf(_("REPORTING BUGS"));
	printf("\n\t");
	printf(_("Report bugs to lifelines.sourceforge.net"));
	printf("\n\n");
	printf(_("COPYRIGHT"));
	printf("\n\t");
	printf("Lifelines is distributed under an X/MIT Open Source license.\n"
		"\tSee file COPYING in the program directory.");
	printf("\n\n");
	printf(_("SEE ALSO"));
	printf("\n\tllines(1), llexec(1), dbverify(1), btedit(1)");
	printf("\n");
}
