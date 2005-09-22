/*
 * @progname       gedn.ll
 * @version        none
 * @author         anon
 * @category       
 * @output         GEDCOM
 * @description
 *
 * The output of this report is a GEDCOM file of the following: 
 * N generations of ancestors, 
 * all spouses and 
 * all children of these ancestors and 
 * all descendents of a person, 
 * as well as the person him/herself 
 */

include("ged_write.li")

proc main ()
{
 	indiset(set1)		/*declare an indi set*/
 	indiset(set2)		/*declare another indi set*/
 	indiset(set3)		/*declare another indi set*/

 	getindi(ind1)		/*ask user to identify person*/
 	if(ind1) {
		getintmsg(maxgen, "Number of Generations")
	  	print("Finding Ancestors... ")
	  	addtoset(set1, ind1, 1)
	  	set(set2, ancestorset(set1))
	  	print(d(lengthset(set2)), nl())
	  	print("Triming Ancestors to ", d(maxgen), " generations... ")
	        forindiset(set2, ind1, ival, icnt) {
	      	  if(le(ival,maxgen)) {
		    addtoset(set3, ind1, ival)
		  }
		}
	  	print(d(lengthset(set3)), nl())

		set(set2, spouseset(set3))	/* add their spouses */
                set(set1, union(set3, set2))    /* combine set1 and set2 */

		set(set2, childset(set1))	/* find everyone's children */
                set(set1, union(set1, set2))    /* combine set1 and set2 */

		call ged_write(set1)		/* write out GEDCOM file */
	}
}
