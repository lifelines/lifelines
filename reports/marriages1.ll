/*
 * @progname       marriages1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text, 80 cols
 * @description    
 *
 *   select and produce an a output report of all marriages in
 *   the database, with date of marriage if known.
 *
 *   marriages1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *
 *   select and produce an a output report of all marriages in
 *   the database, with date of marriage if known.
 *
 *   Output is an ASCII file, and may be printed using 80 column format.
 *
 *   An example of the output may be seen at end of this report.
 */

proc main ()
 {
 	indiset(idx)
 	forindi(indi, n) {
 		if (and(male(indi),gt(nspouses(indi),0))) {
 			addtoset(idx,indi,0)
 			print("y")
 		} else {
 			print("n")
 		} 
 	}
 	print(nl())
 	print("beginning sort")
 	print(nl())
 	namesort(idx)   
 	print("ending sort")
 	print(nl())
 	col(1) "Male Person"
 	col(30) "Date"
 	col(50) "Female Person"
 	col(1)
 	"-----------------------------------------"
 	"-------------------------------------"
 	forindiset(idx,husb,val,n) {
 		col(1) fullname(husb, 1,0,29)
 		spouses(husb,wife,famv,m) {
 			col(30) trim(date(marriage(famv)), 20)
 			col(50) fullname(wife, 1,0,29)
 		}
 		print(".")
 	}
 	nl()
 	print(nl())
}

/*  Sample output of this report.

Male Person                  Date                Female Person
------------------------------------------------------------------------------
BARTH, Johann Ludwig                             ____, Hanna
BIRD, Jacob                                      ____, Mrs.
BIRD, John                                       SHRADER, Elizabeth
BOWERS, Anderson             ABT    1828         COWAN, Lurina Viney "Vina"
BOWERS, James                                    ____, Martha
BRADSHAW, John F.                                CLENDENIN, Agnes "Annie"
CANTER, Henry B.                                 ____, Polina
CANTER, James H.             20 APR 1867         WHITEHORN, Martha Marie
CASON, David                 ca 1790             ____, Mary

*/

/* End of Report */

