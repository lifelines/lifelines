/*
 * @progname    allfam10c.ll
 * @version     1.1
 * @author      Cliff Manis, Jim Eggert
 * @category
 * @output      Text
 * @description
 *
 *   Display all families in the DB, 1 per page.
 *
 *   allfam10c
 *   by:  Cliff Manis  <cmanis@csoftec.csf.com>, modified by Jim Eggert
 *   Family Report for LifeLines
 */

proc main()
{
        forfam(fam,fnum) {
                call fam10c(fam)
                ""  /* this is the form-feed character, octal 014 hex 0C*/
                      /* You may have to change it for your needs */
        }
}

proc fam10c (fam)
{
/*        getfam(fam) */
        dayformat(0)
        monthformat(4)
        dateformat(0)
        set(tday, gettoday())
        set (nl,nl())
        set(h,husband(fam))
        set(w,wife(fam))
        col(55) "Date: " stddate(tday) nl
        col(0) "Family Report (fam10)"
        nl nl
        col(0) "HUSBAND:   "  fullname(h,1,1,50)
        col(63) "(RN=" key(h) ")"
        nl nl
        set(evt, birth(h))
        col(0) "Born:  " stddate(evt) col(25) "Place:  " place(evt)
        set(evt, marriage(fam))
        col(0) "Marr:  " stddate(evt) col (25) "Place:  " place(evt)
        set(evt, death(h))
        col(0) "Died:  " stddate(evt) col(25) "Place:  " place(evt)
        nl nl
        col(0) "HUSBAND'S FATHER:   " name(father(h))
        col(63) "(RN=" key(father(h)) ")"
        nl
        col(0) "HUSBAND'S MOTHER:   " name(mother(h))
        col(63) "(RN=" key(mother(h)) ")"
        nl nl
        col(0) "WIFE:   "  fullname(w,1,1,50)
        col(63) "(RN=" key(w) ")"
        nl nl
        set(evt, birth(w))
        col(0) "Born:  " stddate(evt) col(25) "Place:  " place(evt)
        set(evt, death(w))
        col(0) "Died:  " stddate(evt) col(25) "Place:  " place(evt)
        nl nl
        col(0) "   WIFE'S FATHER:   " name(father(w))
        col(63) "(RN=" key(father(w)) ")"
        col(0) "   WIFE'S MOTHER:   " name(mother(w))
        col(63) "(RN=" key(mother(w)) ")"
        nl nl
        col(0) "========================================================================"
        nl
        col(0) "#  M/F" col(12) "Childrens Names" col(63) "RECORD NUM"
        nl
        col(0) "========================================================================"
        nl
        children(fam, child, num) {
                col(0) d(num)
                col(4) sex(child)
                col(12) name(child) col(63) "(RN=" key(child) ")"
                col(4) "Born:" col(13) stddate(birth(child))
                col(26) place(birth(child))
                nl

                col(4) "Died:" col(13) stddate(death(child))
                col(26) place(death(child))
                nl

                families(child, fvar, svar, num) {
                        if (eq(num,1)) {
                                col(4) "Marr:" col(13) stddate(marriage(fvar))
                                col(26) if (svar) { name(svar)
                                col(63) "(RN=" key(svar) ")" }
                                        else { " " }
                                nl
                        }
                }
                if (eq(nfamilies(child),0)) { " " nl }
                        col(4) "---------------------------------------------------------"
        }
}

/* End of Report */
