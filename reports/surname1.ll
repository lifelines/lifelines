/*
 * @progname       surname1.ll
 * @version        1.0
 * @author         Manis, Wetmore
 * @category       
 * @output         Text, 132 cols
 * @description    

 *  LifeLines Report for quickly getting output of all individuals with a
 * a particular surname.


 *  surname1
 *
 *  LifeLines Report for quickly getting output of a particular
 *  surname.
 *  by Cliff Manis, and Tom Wetmore
 *  22 March 1993
 *
 *  Sample 132 column output at end of this report.
 */

proc main ()
{
        indiset(idx)
        getindiset(idx)
        dayformat(0)
        monthformat(4)
        dateformat(0)
        set(tday, gettoday())

        print(nl())
        print("begin sorting") print(nl())
        namesort(idx)
        print("done sorting") print(nl())

        col(1) "\nSurname Report by:       Cliff Manis\n"
        col(1) "                         P. O. Box 33937\n"
        col(1) "                         San Antonio, Texas  78265-3937\n"
        col(1) "                         Phone:   1-512-654-9912\n\n"
        col(1) "        Date:            " stddate(tday) "\n\n"
        col(1) "This report is formatted 132 columns per line.\n\n"
        col(1) "Index of all persons in database with this surname\n\n"
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

/* Output format of above report:

Surname Report by:       Cliff Manis
                         P. O. Box 33937
                         San Antonio, Texas  78265-3937
                         Phone:   1-512-654-9912

        Date:            24 Mar 1993

This report is formatted 132 columns per line.

Index of all persons in database with this surname

Individual                       Brth Deat First Spouse                   Father                   Mother
------------------------------------------------------------------------------------------------------------------------
HARRIS, -----                              KINDER, Ann
HARRIS, Agnes                              BRADSHAW, Henry
HARRIS, George Brison            1877 1956                                HARRIS, Frederick        MANUS, Mary Amanda
HARRIS, Hope Catewood            1951                                     HARRIS, Richard Calhoun  RANKIN, Nuva Jean
HARRIS, Jesse Mae                          TIPTON, Burrell Andrew
HARRIS, Lenora                   1881 1962 BRADSHAW, Ralph Rankin
HARRIS, Linda                              BENTON, David Edward
HARRIS, Lucy Jane                1879 1948                                HARRIS, Frederick        MANUS, Mary Amanda
HARRIS, Marvin                             SUMMIT, Agness
HARRIS, Mary Elizabeth           1875 1950                                HARRIS, Frederick        MANUS, Mary Amanda
HARRIS, Melvinia Josephine       1864 1944 RHODES, William Wesley
HARRIS, Nathan Washington        1870 1947                                HARRIS, Frederick        MANUS, Mary Amanda
HARRIS, Nora Emma                1880      MANESS, William Curtis
HARRIS, Rebecca                            RHOTON, David Jesse
HARRIS, Terri                              DAUGHERTY, Dana
HARRIS, William Latham           1872 1927                                HARRIS, Frederick        MANUS, Mary Amanda

-end of report-
*/
