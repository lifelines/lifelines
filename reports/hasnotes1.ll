/*
 * @progname       hasnotes1.ll
 * @version        1.1
 * @author         Wetmore, Manis
 * @category       
 * @output         Text
 * @description    
 *
 *   It will produce a report of all the numbers and names (INDI's)
 *   in the database which have a "NOTE" line at level 1 in the record.
 *   It is designed for 10 or 12 pitch, HP laserjet III, or any
 *   other printer (ASCII output).

 *   hasnotes1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in Sep 1992,
 *
 */

proc main ()
{
	"PERSONS IN THE DATABASE WITH NOTES" nl() nl()
	forindi (i, n) {
		set(r, inode(i))
		set(notfound, 1)
		fornodes (r, n) {
			if (and(notfound, eq(0, strcmp("NOTE", tag(n))))) {
				set(notfound, 0)
				key(i) col(8) name(i) nl()
			}
		}
	}
}

/*  Sample output of report.  

PERSONS IN THE DATABASE WITH NOTES

I1     Alda Clifford MANIS
I2     Fuller Ruben MANES
I3     Edith Alberta MANIS
I4     William Bowers MANES
I5     Cordelia "Corda" F. CANTER
I6     William Loyd MANIS
    (all these INDI's did have a NOTE line)

*/

/* End of Report */

