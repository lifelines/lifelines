/*
 * @progname       pedigreelhs.ll
 * @version        none
 * @author         Wetmore, Manis, Hume Smith
 * @category       
 * @output         Text
 * @description
 *
 *   Select and produce an ancestor report for the person selected.
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *   more by Hume Smith (who refuses to learn YACL and so may do odd things):
 *   - optional depth limit
 *   - draws helpful lines
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   name is from pedigreel  (long format)
 *
 *   Output is an ASCII file, and will probably need to be printed
 *   using 132 column format.  Sample (depth 2):

    +!
  +-Ivan Cottnam "Cott" SMITH (10 Jan 1932- ) #2
  | +!
+-Hume Cottnam Llewellyn SMITH (18 Dec 1966- ) #1
  | +!
  +-Gail Ida HUME (26 Mar 1943- ) #3
    +!

 * ! indicates more is known beyond that depth.
 */

proc main ()
{
        getindi(indi)
        getintmsg(depth,"How many generations (-1 for all)?")
    dayformat(0)
    monthformat(4)
    dateformat(0)
        call pedigree(1, depth, indi, "", "  ", "  ")
}

proc pedigree (ah, depth, indi, indent, above, below)
{
        if (eq(depth,0)) {
                indent "+!" nl()
        } else {
                if (par, father(indi)) {
                        call pedigree(mul(2,ah), sub(depth,1), par, concat(indent, above), "  ", "| ")
                }

                indent "+-"
                fullname(indi,1,1,50)
                set(flag,0)
                set(birth," ")
                set(death," ")
                if (evt, birth(indi)) {
                        set(flag,1)
                        set(birth, stddate(evt))
                }
                if (evt, death(indi)) {
                        set(flag,1)
                        set(death, stddate(evt))
                }
                if (flag) { " (" birth "-" death ")" }
                " #" d(ah) nl()

                if (par, mother(indi)) {
                        call pedigree(add(1,mul(2,ah)), sub(depth,1), par, concat(indent, below), "| ", "  ")
                }
        }
}
