/*
 * @progname       nonpatronymics.ll
 * @version        1.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    
 *
 *   Find all cases of nonpatronymic inheritances in the database.
 *   If the child's surname is not identical to the father's surname,
 *   print both out.  If the two surnames have different soundex
 *   codes, undent the printout.  Print statistics at the end.
 *
 *   nonpatronymics
 *
 *   Code by Jim Eggert, eggertj@ll.mit.edu
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Jim Eggert, in 1992.
 *
 *   Output is an ASCII file.
 */

proc main ()
{
	set(n,0)
	set(ns,0)
	set(header,0)
	forindi(indi,num1) {
		if (fath,father(indi)) {
			if (ne(0,strcmp(surname(indi),surname(fath)))) {
				if (eq(header,0)) {
					"Dissimilar surnames" nl()
					"   Similar surnames" nl()
					set(header,1)
				}
				if (eq(strcmp(save(soundex(indi)),
					save(soundex(fath))),0)) {
					"   "
					set(ns,add(ns,1))
				}
				d(num1) " " name(indi)
				" <> "
				name(fath)
				nl()
				set(n,add(n,1))
			}
		}
	}
	nl() d(num1) " individuals scanned." nl()
	d(n) " nonpatronymic inheritances found"
	if (eq(n,0)) { "." nl() }
	else { "," nl()
		d(sub(n,ns)) " of which were soundex-dissimilar." nl()
	}
}
