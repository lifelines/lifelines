/*
 * @progname       exer_UTF-8
 * @version        0.1 (2002/07/23)
 * @author         Perry Rapp
 
 * @category       test

 * @output         mixed

 * @description    UTF-8 subset of exercise.ll tests.


*/



char_encoding("UTF-8")

require("lifelines-reports.version:1.3")
option("explicitvars") /* Disallow use of undefined variables */



proc finnish_UTF_8()
{
	if (not(set_and_check_locale("fi_FI", "Finnish"))) {
		return()
	}
	call check_collate("A", "Z", "A", "Z")
	call check_collate("Z", "Ä", "Z", "Adia")
	call check_collate("Ä", "Ö", "Adia", "Odia")
	call check_collate("A", "Z", "A", "Z")
	call check_collate("Z", "Ä", "Z", "Adia")
	call check_collate("Ä", "Ö", "Adia", "Odia")
}
proc polish_UTF_8()
{
	if (not(set_and_check_locale("pl_PL", "Polish"))) {
		return()
	}
	call check_collate("A", "Z", "A", "Z")
	call check_collate("L", "Ł", "L", "Lstroke")
	call check_collate("Ł", "M", "Lstroke", "M")
}
proc spanish_UTF_8()
{
	if (not(set_and_check_locale("es", "Spanish"))) {
		return()
	}
	call check_collate("A", "Z", "A", "Z")
	call check_collate("N", "Ñ", "N", "Ntilde")
	call check_collate("Ñ", "O", "Ntilde", "O")
}
proc testCollate_UTF_8()
{
	call finnish_UTF_8()
	call polish_UTF_8()
	call spanish_UTF_8()
}

