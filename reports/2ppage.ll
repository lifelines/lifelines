/*
 * @progname    2ppage.ll
 * @version     1.0
 * @author      Wetmore, Manis
 * @category
 * @output      Text, 80 cols
 * @description
 *
 *   It will produce a report of all INDI's in the database, with
 *   two records printed per page.  Record 1 and 2 will be on the
 *   first page.
 *
 *   2ppage               (2 INDI's per page)
 *
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1991,
 *   and it has been modified many times since.
 *
 *   It will produce a report of all INDI's in the database, with
 *   two records printed per page.  Record 1 and 2 will be on the
 *   first page.
 *
 *   It will produce ASCII file output.

This short report show how many different things may be done with
the report language.

These next two paragraphs were by Tom, when he sent me the following
report - which we had been discussing via email.  At the time I wanted
this format for a book I was writing for a Mother-in-Law.

        This report is using the `pagemode' feature.  This allows use
use of the `pos' command to go to any row and column coordinate on a page.
The routine `pageout' dumps the current page to the output file, and
prepares for the program to compose another page.

        Notice how the `gettoday' function is used to get today's date to
print out.  Also note that the `mod' function is used to put every other
person on the top half, and the other every other person on the bottom
half.  Also note that the variable `page' counts the page numbers.

*/

proc main ()
{
        pagemode(66,80)
        monthformat(4)

        set(tday, save(stddate(gettoday())))

        set(page, 1)
        forindi(i, n) {
                if (mod(n,2)) {
                        pos(2,1)
"  = = = =  MANES / MANIS  Family  History  &  Genealogy  = = =  " tday nl()
                        pos(65,1)
"  = = =   Cliff Manis, PO Box 33937, San Antonio, TX 78265  = = " d(page)
nl ()
                        set(page, add(page,1))
                        pos(4,1)
                        call oneout(i)
                } else {
                        pos(34,1)
                        call oneout(i)
                        pageout()
                }
        }
        if (mod(n,2)) {
                pageout()
        }
}
proc oneout (i)
{
        set(f, father(i))
        set(m, mother(i))

        "  FULL NAME:   " name(i) col(46) "(" key(i) ")" nl() nl()
        "     FATHER:   " name(f) col(46) "(" key(f) ")" nl()
        "     MOTHER:   " name(m) col(46) "(" key(m) ")" nl() nl()
        "  Born:        " stddate(birth(i))  " at " place(birth(i)) nl()
        call outmarriages(i) nl()
        "  Died:        " stddate(death(i))  " at " place(death(i)) nl() nl()
        call outchildren(i)
}
proc outmarriages (i)
{
        spouses(i, s, f, n) {
                if (eq(1, n)) {
                        "  Married:     " stddate(marriage(f)) nl()
                        "  Married to:  " name(s) col(46) "("key(s)")" nl()
                } else {
                        "  Remarried:   " stddate(marriage(f)) nl()
                        "  Remarried to:" name(s) col(46) "("key(s)")" nl()
                }
        }
}
proc outchildren (i)
{
        set(j, 0)
        families(i, f, s, n) {
                set(j, add(j, nchildren(f)))
        }
        "  Number of Children:     " d(j) nl()
        set(j, 1)
        families(i, f, s, n) {
                children(f, c, m) {
                        "   " d(j) ".  " name(c) col(46) "("key(c)")"
                        col(57) "Born:  " stddate(birth(c)) nl()
                        set(j, add(j,1))
                }
        }
}


/*
 * Sample output


  = = = =  MANES / MANIS  Family  History  &  Genealogy  = = =  15 Jan 1993

  FULL NAME:   Wilee "Wyley" WORWICK         (3)

     FATHER:   Wyley WORWICK                 (1)
     MOTHER:   Wife of Wyley WORWICK         (2)

  Born:               1824 at
  Married:     31 Oct 1844
  Married to:  Martha D. JOHNSON             (4)

  Died:           Nov 1874 at Union Co, TN

  Number of Children:     10
   1.  Tempia Catherine WARWICK              (5)        Born:         1846
   2.  Louisa Mahayla WARWICK                (6)        Born:         1848
   3.  Margarett WARWICK                     (7)        Born:         1850
   4.  Mary WARWICK                          (8)        Born:         1852
   5.  Matilda WARWICK                       (9)        Born:         1854
   6.  Calaway WARWICK                       (10)       Born:  29 Jul 1855
   7.  Jamima WARWICK                        (11)       Born:         1858
   8.  Nancy Elizabeth WARWICK               (12)       Born:         1860
   9.  Rebecca WARWICK                       (13)       Born:         1864
   10.  Martha WARWICK                       (14)       Born:     Sep 1869








  FULL NAME:   Martha D. JOHNSON             (4)

     FATHER:                                 ()
     MOTHER:                                 ()

  Born:               1825 at NC
  Married:     31 Oct 1844
  Married to:  Wilee "Wyley" WORWICK         (3)
  Remarried:
  Remarried to:William PETREE                (22)

  Died:         at

  Number of Children:     10
   1.  Tempia Catherine WARWICK              (5)        Born:         1846
   2.  Louisa Mahayla WARWICK                (6)        Born:         1848
   3.  Margarett WARWICK                     (7)        Born:         1850
   4.  Mary WARWICK                          (8)        Born:         1852
   5.  Matilda WARWICK                       (9)        Born:         1854
   6.  Calaway WARWICK                       (10)       Born:  29 Jul 1855
   7.  Jamima WARWICK                        (11)       Born:         1858
   8.  Nancy Elizabeth WARWICK               (12)       Born:         1860
   9.  Rebecca WARWICK                       (13)       Born:         1864
   10.  Martha WARWICK                       (14)       Born:     Sep 1869







  = = =   Cliff Manis, PO Box 33937, San Antonio, TX 78265  = = 2

 - end of report -

 */
