/*   
 * @progname       genancc1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         GEDCOM
 * @description    
 *
 * This program is useful for extracting 
 * data when a person requests data about someone from your database.
 * The output of this report is a GEDCOM file of the following: 
 * all ancestors, 
 * all spouses and 
 * all children of all ancestors and 
 * all descendents of a person, 
 * as well as the person him/herself 
 * and his/her spouses.  
 *
 *   genancc1
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
 * For example, the output of this report is a GEDCOM file of the
 * following: 
 *
 * all ancestors, 
 * all spouses and 
 * all children of all ancestors and 
 * all descendents of a person, 
 * as well as the person him/herself 
 * and his/her spouses.  
 *
 * This form of the program is probably the most useful for extracting 
 * data when a person requests data about someone from your database.
 */
 
proc main ()
{
 		indiset(set1)		/*declare an indi set*/
 		indiset(set2)		/*declare another indi set*/

 		getindi(indi)		/*ask user to identify person*/
 		addtoset(set1, indi, n)	/*add that person to set1*/

 		set(set2, ancestorset(set1))   /* for ancestors */

                set(set1, union(set1, set2))    /* combine set1 and set2 */
		set(set2, spouseset(set1))
                set(set1, union(set1, set2))    /* combine set1 and set2 */
		set(set2, childset(set1))
                set(set1, union(set1, set2))    /* combine set1 and set2 */

 		gengedcom(set1)		/*write final set as GEDCOM file*/
}

/* end of report */

