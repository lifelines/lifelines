/*
 * @progname       pdesc1
 * @version        1.0
 * @author         Wetmore, Manis, Jones, Eggert
 * @category       
 * @output         Text, 132 cols
 * @description    
 *
 *   pdesc1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *   With modifications by:  James P. Jones
 *   With modifications by:  Jim Eggert (unknown spouse bugfix)
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990.
 *
 *   It will select and produce a descendant report for the person
 *   selected.   Children of each spouse are printed below that spouse.
 *
 *   Descendants report format, which print the date in long format.
 *
 *   Output is an ASCII file, and will probably need to be printed
 *   using 132 column format.
 *
 *   An example of the output may be seen at end of this report.
 */

proc main ()
{
        getindi(indi)
        col(35) "DECENDANCY CHART\n\n"
        col(5)
"=======================================================================\n\n"
        call pout(0, indi)
}
proc pout(gen, indi)
{
        print(name(indi)) print("\n")
        col(add(5,mul(4,gen)))
        d(add(gen,1)) "-- " call outp(indi)
        set(next, add(1,gen))
        families(indi,fam,sp,num) {
                col(add(5,mul(4,gen)))
                " sp-"
                call outp(sp)
                if (lt(next,15)) {
                        children(fam, child, no) {
                                call pout(next, child)
                        }
                }
        }
}

proc outp(indi)
{
        fullname(indi, 1, 1, 40)
        " (" long(birth(indi)) " - " long(death(indi)) ")\n"
}

/*  Sample report format, and this example would need 132 column printing.

    Note, the children of spouses, are below that spouse.


                                  DECENDANCY CHART

    =======================================================================

    1-- John Franklin NEWMAN (4 MAY 1830, Jefferson Co, TN - 18 SEP 1921, Harriman, Roane Co, TN)
     sp-Mary M. GILBREATH (16 MAY 1827, Jefferson County, TN - 16 JUN 1860, Jefferson County, TN)
        2-- Catherine Cole NEWMAN (18 DEC 1853 - 2 FEB 1920, Jefferson County, TN)
         sp-William Newton MCMURRAY ( - )
        2-- James Aaron NEWMAN (31 DEC 1855 - 23 JAN 1926)
         sp-Priscilla Ellen PALMER ( - )
     sp-Elizabeth "Eliza" Catherine MCGUIRE (14 FEB 1830 - 28 MAY 1864)
        2-- George Arthur NEWMAN (18 JUN 1862, Jefferson Co., TN - 25 DEC 1883)
         sp-Florence Elizabeth BRADSHAW ( - )
            3-- Ester NEWMAN ( - )
            3-- Emma Kate NEWMAN ( - )
     sp-Mary Jean CORBETT (9 OCT 1843, Jefferson Co, TN - 2 NOV 1918, Jefferson Co, TN)
        2-- Andrew Johnson NEWMAN (16 APR 1869, Jefferson Co, TN - )
         sp-Lillian ALEXANDER ( - )
        2-- Martha Gilbreath NEWMAN (21 JUN 1871, Jefferson Co, TN - 12 APR 1947)
         sp-John Wesley MCGEE ( - )
            3-- Mary Helen MCGEE ( - )
                4-- Maxie Rae MCGEE ( - )
                 sp-John BIBLE ( - )
                    5-- Martha BIBLE ( - )
                    5-- Stacie BIBLE ( - )
            3-- Ewell NEWMAN ( - )

*/

/* End of Report */
