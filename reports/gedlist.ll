/*
 * @progname       gedlist.ll
 * @version        1.1
 * @author         Paul B. McBride (pbmcbride@rcn.com)
 * @category       
 * @output         GEDCOM
 * @description

	gedlist.ll generates a GEDCOM file for the male line of the
	input individuals.

 Algorithm:
	prompt for people
	add male line of each person to set
	add all children to set
	add all spouses to set
	add all parents to set
	generate GEDCOM file

 Author:	 Paul B. McBride (pbmcbride@rcn.com)

 Version:
	1.1 January 10, 2001   correct prompt
 	1.0 September 27, 2000 created from gdc.ll dated February 28, 1996
 */

include("ged_write.li")
 
proc main ()
{
	indiset(set0)
	indiset(set1)		/*declare an indi set*/
	indiset(set2)		/*declare another indi set*/

	getindiset(set0, "Identify people to include in GEDCOM File")

	if(eq(lengthset(set0),0)) { return() }

	/* add everyone in the male line for each person*/

	forindiset(set0, indi, ival, icnt) {
	    addtoset(set1, indi, 1)	/*add that person to set1*/
	    set(fath, indi)
	    while(fath, father(fath)) {
	      addtoset(set1, fath, 1)	/*add the father to set1*/
	    }
	}

	set(set2, childset(set1))	/* add all the children */
	set(set1, union(set1, set2))    /* combine set1 and set2 */

 	set(set2, spouseset(set1))	/* add all the spouses */
	set(set1, union(set1, set2))    /* combine set1 and set2 */

	set(set2, parentset(set1))	/* find everyone's parents */
        set(set1, union(set1, set2))    /* combine set1 and set2 */

	call ged_write(set1)		/* write out GEDCOM file */
}

/* end of report */
