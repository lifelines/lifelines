/* 
 * @progname       showlines1.ll
 * @version        1.0
 * @author         Wetmore
 * @category       
 * @output         Text
 * @description    

 *   This program will produce a report of all ancestors of a person,
 *   and is presently designed for 10 or 12 pitch, HP laserjet III.

 *   showlines1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1991.
 *
 *
 *   Output is an ASCII file
 *
 */
 

/* showlines */
proc main ()
{
	list(plist)
	getindi(indi)
	monthformat(4)
	print("Each dot is an ancestor.") print(nl())
	"------------------------------------------------------------" nl()
	"ANCESTRAL LINES OF -- " name(indi) nl()
	enqueue(plist, indi)
	while (indi, dequeue(plist)) {
		call show_line(indi, plist)
	}
	print(nl())
}

proc show_line (indi, plist)
{
	"------------------------------------------------------------" nl()
	while (indi) {
		name(indi) col(32) stddate(birth(indi))
		col(45) stddate(death(indi)) nl()
		print(".")
		if (moth, mother(indi)) {
			enqueue(plist, moth)
		}
		set(indi, father(indi))
	}
}
 
/* End of Report */

