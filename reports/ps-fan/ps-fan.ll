/*
 * @progname       ps-fan1.ll
 * @version        1993-08-16
 * @author         Andrew Deacon (deacon@inf.ethz.ch)
 * @category       
 * @output         PostScript
 * @description    
 *
 *                 Write a PostScript fan chart.
 *
 *   Code (by Tom re-arranged) by Andrew Deacon, deacon@inf.ethz.ch
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   This report was adapted from a file made by Cliff Manis using the
 *   GEDCHART software written by Tom Blumer.
 *
 *   Output is a PostScript file. The file "ps-fan.ps" is included
 *   when the report is generated. This file consists of the PostScript
 *   commands used by the GEDCHART software written by Tom Blumer.
 *
 */

global(PS_HDR_FILE)

proc main ()
{
        set (nl,nl())
        getindi(indi)
        set(PS_HDR_FILE, "ps-fan.ps")   /* PostScript Header file name */
        copyfile(PS_HDR_FILE)
        call pedigree(0, 1, 1, indi)
        "showpage" nl()                 /* PostScript Tail command */
}

proc pedigree (in, lev, ah, indi)
{
        "(" fullname(indi,1,1,50) ")"
        " (" if (evt, birth(indi)) { "b. " date(birth(indi)) } ")"
        " (" if (evt, death(indi)) { "d. " date(death(indi)) } ")"
        " " d(in)
        " " d(sub(ah, lev))
        " i"
        nl()

        if (lt(in,4)) {
            if (par, father(indi)) {
                call pedigree(add(1,in), mul(2,lev), mul(2,ah), par)
            }
            if (par, mother(indi)) {
                call pedigree(add(1,in), mul(2,lev), add(1,mul(2,ah)), par)
            }
        }
}
