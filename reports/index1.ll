
/* 
 *   index1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   and it has been modified many times since.
 *
 *   It will produce a report of all INDI's in the database, with
 *   sorted names as output.
 *
 *   It is presently designed for 12 pitch, HP laserjet III,
 *   for printing a index of person in the database.
 *
 *   It will produce ASCII file output.
 *
 */

proc main ()
{
	indiset(idx)
	monthformat(4)
	forindi(indi,n) {
		addtoset(idx,indi,n)
		print(".")
	}
	print(nl()) print("indexed ") print(d(n)) print(" persons.")
	print(nl())
	print(nl())
	print("begin sorting") print(nl())
	namesort(idx)
	print("done sorting") print(nl())

col(1) "======================================================================" nl()
col(16) "INDEX OF ALL PERSONS IN DATABASE" nl()
col(1) " " nl()
col(1) " " nl()
col(1) "LAST, First                       Index #  Birthdate       Deathdate" nl()
col(1) "--------------------------------  -------- ------------    ------------" nl()

	forindiset(idx,indi,v,n) {
		col(1) fullname(indi,1,0,30)
		col(35) key(indi)
		col(44) stddate(birth(indi))
		col(60) stddate(death(indi))
		print(".")
	}
	nl()
	print(nl())
}

/* End of Report */

