/* 
 * @progname       fam16rn1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text
 * @description    
 *
 *   This program produces a family report of the person (husband), wife, 
 *   their children, and some data about the children's marriages.
 *   It is presently designed for 16 pitch, HP laserjet III,
 *   printing a single page of information about that family.
 *
 *   fam16rn1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   and it has been modified many times since.
 *
 *   Output is an ASCII file.
 *
 */
 
proc main ()
{
        getfam(fam)

	dayformat(0)
	monthformat(4)
	dateformat(0)
 	set(tday, gettoday())	
        set (nl,nl())
        set(h,husband(fam))
        set(w,wife(fam))
        col(6) "Report by:   Cliff Manis  " 
        nl
        col(19) "MANIS / MANES Family History"
        col(50) "P. O. Box 33937   San Antonio, TX  78265-3937"
        nl nl nl
        col(6) "HUSBAND:   "  fullname(h,1,1,50) " (RN=" key(h) ")" 
        col(80) "Report date: " stddate(tday)
	nl nl
        set(evt, birth(h))
        col(6) "Born:  " stddate(evt) col(35) "Place:  " place(evt)
        set(evt, marriage(fam))
        col(6) "Marr:  " stddate(evt) col(35) "Place:  " place(evt)
        set(evt, death(h))
        col(6) "Died:  " stddate(evt) col(35) "Place:  " place(evt)
        col(6) "HUSBAND'S" col(50) "HUSBAND'S"
        col(6) "FATHER:   " name(father(h)) " (RN=" key(father(h)) ")" 
        col(50) "MOTHER:   " name(mother(h)) " (RN=" key(mother(h)) ")" 
        nl nl
        col(6) "WIFE:   "  fullname(w,1,1,50) " (RN=" key(w) ")" 
	nl nl
        set(evt, birth(w))
        col(6) "Born:  " stddate(evt) col(35) "Place:  " place(evt)
        set(evt, death(w))
        col(6) "Died:  " stddate(evt) col(35) "Place:  " place(evt)
        col(6) "WIFE'S" col(50) "WIFE'S"
        col(6) "FATHER:   " name(father(w)) " (RN=" key(father(w)) ")" 
        col(50) "MOTHER:   " name(mother(w)) " (RN=" key(mother(w)) ")"
        nl nl
	col(6) "==============================================="
	col(53) "=======================================" 
        col(92) "==========================" nl
        col(8) "M/F"
        col(22) "CHILDREN"
        col(45) "WHEN BORN"
	col(62) "WHEN DIED"
        col(82) "WHERE BORN" 
	nl 
        col(45) "1st MARRIAGE"
	col(62) "SPOUSE"
        nl 
	col(6) "==============================================="
	col(53) "=======================================" 
        col(92) "==========================" nl
        children(fam, child, num) {
                col(6) d(num)
                col(9) sex(child)
                col(11) name(child) " (RN=" key(child) ")"
                col(45) stddate(birth(child))
                col(62) stddate(death(child))
                col(82) place(birth(child))
                families(child, fvar, svar, num) {
                        if (eq(num,1)) {
                                col(45) stddate(marriage(fvar))
                                col(62) if (svar) { name(svar) " (RN=" key(child) ")" }
                                        else { " " }
                                nl nl
                        }
                }
                if (eq(nfamilies(child),0)) { " " nl nl }
        }
}

/* End of Report */

