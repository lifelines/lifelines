/*
 * @version        1.0
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate codeset conversion
*/

char_encoding("ASCII")

require("lifelines-reports.version:1.3")
option("explicitvars") /* Disallow use of undefined variables */
include("st_aux")

/* entry point in case not invoked via st_all.ll */
proc main()
{
	call testConvert()
}

proc testConvert()
{
	call initSubsection()
	call convert_tests()
	call reportSubsection("convert tests")
}

proc convert_tests()
{
	/* we have to use bytecodes, b/c any non-ascii will be
	first converted by the report parser to internal codeset,
	which depends on the current database :( */

	/* trivial test */
	call checkconv("silver fox", "UTF-8", "silver fox", "ISO-8859-1")
	call checkconv("silver fox", "UTF-8", "silver fox", "ISO-8859-2")

	/* LATIN SMALL LETTER N WITH TILDE */
	call checkconv("$C3$B1", "UTF-8", "$F1", "ISO-8859-1")

	/* LATIN SMALL LETTER A WITH ACUTE u000E1 */
	call checkconv("$C3$A1", "UTF-8", "$E1", "ISO-8859-1")
	call checkconv("$C3$A1", "UTF-8", "$E1", "ISO-8859-2")
	call checkconv("$C3$A1", "UTF-8", "$E1", "ISO-8859-3")
	call checkconv("$C3$A1", "UTF-8", "$E1", "ISO-8859-4")
	call checkconv("$C3$A1", "UTF-8", "$E1", "ISO-8859-15")

	/* LATIN CAPITAL LETTER L WITH STROKE u00141 */
	call checkconv("$C5$81", "UTF-8", "$A3", "ISO-8859-2")

	/* LATIN SMALL LETTER S WITH CEDILLA u0015F */
	call checkconv("$C5$9F", "UTF-8", "$BA", "ISO-8859-2")
	call checkconv("$C5$9F", "UTF-8", "$BA", "ISO-8859-3")

	/* LATIN SMALL LETTER G WITH CEDILLA u00123 */
	call checkconv("$C4$A3", "UTF-8", "$BB", "ISO-8859-4")


	/* LATIN CAPITAL LETTER N WITH CARON */
/*	call checkconv("$C3$A1", "UTF-8", "$u00147_", "ISO-8859-2") */
}

/* test a conversion and its reverse */
proc checkconv(bc1, cs1, bc2, cs2)
{
	call checkconv_1way(bc1, cs1, bc2, cs2)
	call checkconv_1way(bc2, cs2, bc1, cs1)
}

/* test a single conversion */
proc checkconv_1way(bc1, cs1, bc2, cs2)
{
	set(str1, bytecode(bc1, "raw"))
	set(str2, bytecode(bc2, "raw"))
	if (ne(convertcode(str1, cs1, cs2), str2)) {
		set(fstr, concat("convertcode(bytecode(", bc1
			, ",raw),", cs1, ",", cs2
			, ") <> bytecode(", bc2, ",raw) FAILURE"))
		call reportfail(fstr)
	} else { incr(testok) }
}

