/*
 * @progname       namesformat1.ll
 * @version        1.0
 * @author         Manis
 * @category       
 * @output         Text
 * @description    
 *
 *   This program produces a report of how the names format may be printed
 *   using the LifeLines Report Generator.
 *
 *   namesformat1
 *
 *   Code by Cliff Manis, cmanis@csoftec.csf.com
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Cliff Manis in 1991
 *
 *   It will produce a report of how the names format may be printed
 *   using the LifeLines Report Generator.
 *
 *   It is designed for 10 or 12 pitch, HP laserjet III, or any
 *   other printer.
 *
 *   Output is an ASCII file.
 *
 *   An example of the output may be seen at end of this report.
 *
 */

proc main ()
{
	set (nl,nl())
	getindi(indi)
	fullname(indi,0,0,30) nl
	fullname(indi,1,0,30) nl
	fullname(indi,0,1,30) nl
	fullname(indi,1,1,30) nl
	fullname(indi,0,0,12) nl
	fullname(indi,1,0,12) nl
	fullname(indi,0,1,12) nl
	fullname(indi,1,1,12) nl
}

/*  Sample of report output

Manis, Alda Clifford        
MANIS, Alda Clifford
Alda Clifford Manis
Alda Clifford MANIS
Manis, A C
MANIS, A C
Alda C Manis
Alda C MANIS
* /

/* End of Report */

