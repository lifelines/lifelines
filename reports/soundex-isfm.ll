/*
 * @progname       soundex-isfm.ll
 * @version        1.0
 * @author         Wetmore, Manis, Eggert
 * @category       
 * @output         Text, 132 cols
 * @description    
 *
 *   This program will  produce a report of all the INDI's in the database,
 *   in the format as seen at end of report.  May be sorted easily
 *   to see the Father or Mother column sorted report.
 *
 *   soundex-isfm
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com, 1991
 *   Modifications by Cliff Manis, cmanis@csoftec.csf.com, 1992
 *   Modifications by Jim Eggert, atc.ll.mit.edu!eggertj Fri Feb 26 1993
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   This report can be used to output everyone in the database,
 *   or selected by a single soundex code.  The soundex code
 *   can be entered either by knowing the code, or by selecting
 *   an individual and using his/her code.
 *
 *   The report name come from: isfm (Indi Spouse Father Mother)
 *   It is designed for 16 pitch, HP laserjet III, 132 column, and
 *   also those who have X-Windows, 132 columns video.
 *
 *   This report produces an ASCII output file.
 */

proc main ()
{
        indiset(idx)
        getintmsg(smethod,
                "0=all persons, 1=given Soundex, 2=Soundex of a given person")
        if (eq(smethod,1)) {
                getstrmsg(scode,
                        "Enter desired Soundex code (return=any, Z999=unknown)")
                if (scode) { set(scode,save(upper(scode))) }
        }
        elsif (eq(smethod,2)) {
                getindimsg(person,"Enter name of person with desired Soundex")
                if (person) {
                        set(scode,save(soundex(person)) )
                }
        }
        if (scode) { print("Using Soundex code ") print(scode) print("\n") }
        else { print("Using all persons in database\n") }
        set(count,0)
        forindi(indi,n) {
                set(getit,1)
                if (scode) {
                        if (strcmp(scode,soundex(indi))) { set(getit,0) }
                }
                if (getit) {
                        addtoset(idx,indi,n)
                        if (scode) {
                                set(count,add(count,1))
                                print(d(count)) print("/")
                        }
                        print(d(n)) print(" ")
                }
        }
        print("\nbegin sorting\n")
        namesort(idx)
        print("done sorting\n")
        col(1) "INDEX OF ALL PERSONS IN DATABASE"
        if (scode) { " WITH SOUNDEX CODE: " scode }
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


/*      Sample output of report   (132 columns)

INDEX OF ALL PERSONS IN DATABASE WITH SOUNDEX CODE: D340

Individual                       Brth Deat First Spouse                   Father                   Mother
------------------------------------------------------------------------------------------------------------------------
DUDLEY, Alexander                1645                                     DUDLEY, Richard          SEAWELL, Mary
DUDLEY, Ambrose                  1665      DUDLEY, Wife_of Ambrose        DUDLEY, Ambrose          DUDLEY, Wife_of Col_Ambrose
DUDLEY, Ambrose                  1649      DUDLEY, Wife_of Col_Ambrose    DUDLEY, Richard          SEAWELL, Mary
DUDLEY, Christopher              1715 1781                                DUDLEY, Robert           CURTIS, Elizabeth
DUDLEY, Dorcas                   1704 1765 ROUNTREE, William              DUDLEY, Ambrose          DUDLEY, Wife_of Ambrose
DUDLEY, Edward                   1605 1655 PRITCHARD, Elizabeth
DUDLEY, James                    1645 1741 WELCH, Mary                    DUDLEY, Richard          SEAWELL, Mary
DUDLEY, Richard                  1623 1687 SEAWELL, Mary                  DUDLEY, Edward           PRITCHARD, Elizabeth
DUDLEY, Robert                   1647 1701 RANSOM, Elizabeth              DUDLEY, Richard          SEAWELL, Mary
DUDLEY, Robert                   1691 1745 CURTIS, Elizabeth              DUDLEY, Robert           RANSOM, Elizabeth
DUDLEY, Wife_of Ambrose          1640      DUDLEY, Ambrose
DUDLEY, Wife_of Col_Ambrose      1645      DUDLEY, Ambrose
DUDLEY, William                  1621 1672 CARY, Elizabeth                DUDLEY, Edward           PRITCHARD, Elizabeth

  -- end of sample
*/
