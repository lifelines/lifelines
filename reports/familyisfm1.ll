/* 
 * @progname       familyisfm1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text, 132 cols
 * @description    
 *
 *   It will produce a report of all the INDI's in the database,
 *   in the format as seen at end of report.  May be sorted easily
 *   to see the Father or Mother column sorted report.
 *   The report name come from: isfm (Indi Spouse Father Mother)
 *   It is designed for 16 pitch, HP laserjet III, 132 column
 *   (ASCII output).
 *
 *   familyisfm1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1991,
 *   and it has been modified many times since.
 *
 */
 

/* 
 *  familyisfm1 
 */

proc main ()
{
        indiset(idx)		
        forindi(indi,n) {
                addtoset(idx,indi,n) 	
                print(d(n)) print(" ")
        }
        print(nl())
        print("begin sorting") print(nl())
        namesort(idx) 
        print("done sorting") print(nl())
        col(1) "INDEX OF ALL PERSONS IN DATABASE"
        col(1) "Individual"
        col(34) "Brth"
        col(39) "Deat"
        col(44) "First Spouse"
        col(75) "Father"
        col(106) "Mother"
        col(1) "----------------------------------------"
        "----------------------------------------"
        "----------------------------------------"
        forindiset(idx,indi,v,n) { 
                col(1) fullname(indi,1,0,29)
                col(34) year(birth(indi))
                col(39) year(death(indi))
                if(gt(nspouses(indi), 0)) {
                        spouses(indi, spou, fam, n) {
                                if (eq(1,n)) {
                                        col(44) fullname(spou,1,0,29)
                                }
                        }
                }
                if(fath,father(indi)) {
                        col(75) fullname(fath,1,0,29)
                }
                if(moth,mother(indi)) {
                        col(106) fullname(moth,1,0,29)
                }
        }
        nl()
        print(nl())
}

/* Sample output of this report.

INDEX OF ALL PERSONS IN DATABASE
Individual                       Brth Deat First Spouse                   Father                   Mother
------------------------------------------------------------------------------------------------------------------------
CUNNINGHAM, Margaret                       COLQUHOUN, Sir_John
DE_COLQUHOUN, Sir_Humphry        1280 1330                                DE_COLQUHOUN, Sir_Ingelramus
DE_COLQUHOUN, Sir_Ingelramus     1250                                     DE_COLQUHOUN, Sir_Robert
DE_COLQUHOUN, Sir_Robert         1310 1390 ____, Lady_of_Luss             DE_COLQUHOUN, Sir_Humphry
DE_COLQUHOUN, Sir_Robert         1220 1280                                DE_KILPATRICK, Umfridus
DE_KILPATRICK, Umfridus          1190 1260
DENTON, Denise Marie             1955      MANESS, Marion
DOUGLAS, Archibald                         DUNBAR, Elizabeth
DUNBAR, Elizabeth                     1485 DOUGLAS, Archibald             DUNBAR, James
HAMILTON, Judith                 1662      CALHOUN, Alexander

*/

/* End of Report */

