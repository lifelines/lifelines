/*
 * @progname       exer_8859-1
 * @version        0.01 (2002/07/23)
 * @author         Perry Rapp
 
 * @category       test

 * @output         mixed

 * @description    8859-1 subset of exercise.ll tests

*/


char_encoding("ISO-8859-1") 

require("lifelines-reports.version:1.3")
option("explicitvars") /* Disallow use of undefined variables */

proc finnish_8859_1()
{
	if (not(set_and_check_locale("fi_FI", "Finnish"))) {
		return()
	}
	call check_collate("A", "Z", "A", "Z")
	call check_collate("Z", "Ä", "Z", "Adia")
	call check_collate("Ä", "Ö", "Ä", "Odia")
}
proc spanish_8859_1()
{
	if (not(set_and_check_locale("es", "Spanish"))) {
		return()
	}
	call check_collate("A", "Z", "A", "Z")
	call check_collate("N", "Ñ", "N", "Ntilde")
	call check_collate("Ñ", "O", "Ntilde", "O")
}
proc testCollate_8859_1()
{
	call finnish_8859_1()
	call spanish_8859_1()
}


