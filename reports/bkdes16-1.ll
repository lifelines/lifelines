/* 
 * @progname       bkdes16-1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text
 * @description    
 *
 *   It will produce a report of all descendents of a person,
 *   and is presently designed for 16 pitch, HP laserjet III.
 *   This report produces an ASCII file, in output format.
 *
 *   bkdes16-1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   and it has been modified many times since.
 *
 */
 
proc main ()
{
        set (nl,nl())
        getindi(indi)
        col(10) "Report by:  Cliff Manis   MANIS / MANES Family History   P. O. Box 33937   San Antonio, TX  78265-3937 "
        nl()
        col(10) "Phone:  1-512-654-9912"
        nl()
        col(10) "Date:   27 Jun 1992"
        nl() nl()
        col(10)"DESCENDANTS OF: " name(indi) nl() nl()
        call pout(0, indi)
}
proc pout(gen, indi)
{
	col(10) print(name(indi)) print(nl())
        set(ndots, 0)
        while (lt(ndots, gen)) {". " set(ndots, add(1,ndots))}
        "* " name(indi)
        if (e, birth(indi)) {", b. " long(e) }
        nl()
        col(10) spouses(indi,sp,fam,num) {
                set(ndots, 0)
                col(10) while (lt(ndots, gen)) {"  " set(ndots,add(1,ndots))}
                "    m. " name(sp) nl()
        }
        set(next, add(1,gen))
        if (lt(next,15)) {
                families(indi,fam,sp,num) {
                        children(fam, child, no) {
                                call pout(next, child)
                        }
                }
        }
}

/* end of report */

