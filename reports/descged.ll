/*
 * @progname       descged.ll
 * @version        2000-07
 * @author         Tom Wetmore, Cliff Manis, and Chris Eagle
 * @category       
 * @output         Text
 * @description
 *
 * The output of this report is a GEDCOM file of the following:
 * all descendents of a named individual
 * all spouses of the named indivdual
 * spouses of all descendents of the named individual
 * (i.e. this program looks only down the tree, never up)
 *
 * This form of the program is probably the most useful for extracting
 * data when a person requests data about someone from your database.
 *
 *   July 2000
 *
 *   modified by Chris Eagle from genancc1 by:
 *
 *   by Tom Wetmore, ttw@cbnewsl.att.com
 *         (as sent to Cliff Manis in August 1992)
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   [I have only given it a name and added lots of comments] /cliff
 *
 *   August 1992
 *
 */

proc main ()
{
   indiset(set1)     /*declare an indi set*/
   indiset(set2)     /*declare another indi set*/

   getindi(indi)     /*ask user to identify person*/
   addtoset(set1, indi, n) /*add that person to set1*/

   set(set2, spouseset(set1))      /* get individuals spouse(s) */
   set(set1, union(set1, set2))    /* combine set1 and set2 */

   indiset(set3)  /* set used in determining when to stop */
   set(set3, set1)

   set(set2, childset(set1))     /* find first generation of children */
   set(set1, union(set1, set2))  /* combine set1 and set2 */

   while (lengthset(difference(set1, set3))) {
      set(set2, spouseset(set1))    /* add the childrens spouses */
      set(set1, union(set1, set2))  /* combine set1 and set2 */

      set(set3, set1)               /* remember the previous state */
      set(set2, childset(set1))     /* find more children */
      set(set1, union(set1, set2))  /* combine set1 and set2 */
   }

   gengedcom(set1)      /*write final set as GEDCOM file*/
}

/* end of report */
