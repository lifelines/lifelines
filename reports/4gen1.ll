/*
 * @progname       4gen1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text, 80 cols
 * @description    
 *
 *   select and produce a ancestor report for the person selected.
 *   Output is an ASCII file, and will probably need to be printed
 *   using 10 or 12 pitch.
 *
 *   4gen1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *
 *   select and produce a ancestor report for the person selected.
 *
 *   Output is an ASCII file, and will probably need to be printed
 *   using 10 or 12 pitch.
 *
 *   An example of the output may be seen at end of this report.
 */


proc main ()
{
	getindi(indi)
	pagemode(64,80)
	call pedout(indi,1,4,1,64)
	pageout()
	print(nl())
}

proc pedout (indi, gen, max, top, bot)
{
	if (and(indi,le(gen,max))) {
		set(gen,add(1,gen))
		set(fath,father(indi))
		set(moth,mother(indi))
		set(height,add(1,sub(bot,top)))
		set(offset,div(sub(height,8),2))
		call block(indi,add(top,offset),mul(10,sub(gen,2)))
		set(half,div(height,2))
		call pedout(fath,gen,max,top,sub(add(top,half),1))
		call pedout(moth,gen,max,add(top,half),bot)
	}
}

proc block (indi, row, col)
{
	print(".")
	set(row,add(3,row))
	set(col,add(3,col))
	pos(row,col)
	name(indi)
	set(row,add(row,1))
	pos(row,col)
	set(e,birth(indi))
	" b. "
	if (and(e,date(e))) { date(e) }
	set(row,add(row,1))
	pos(row,col)
	" bp. "
	if (and(e,place(e))) { place(e) }
}

/*   Sample output of the 4gen1 report:
     Person requested was:  a c /manis 
 
 
                                William Thomas MANES
                                 b. 26 Nov 1828
                                 bp. Hamblen, Tennessee
 
                      William Bowers MANES
                       b. 6 Jan 1868
                       bp. Hamblen Co, TN ?
 
                                Martha A. BOWERS
                                 b. 14 APR 1829
                                 bp. TN
 
            Fuller Ruben MANES
             b. 19 Nov 1902
             bp. Union Valley, Sevier Co, TN
 
                                James H. CANTER
                                 b. ca 1847
                                 bp. Claiborne Co, TN
 
                      Cordelia "Corda" F. CANTER
                       b. 7 Dec 1869
                       bp. Jonesboro, Washington Co, TN
 
                                Martha Marie WHITEHORN
                                 b. 22 DEC 1846
                                 bp. Washington Co, TN ?
 
  Alda Clifford MANIS
   b. 11 MAR 1939
   bp. Knoxville, Knox Co, TN
 
                                Thomas D.A.F.S. MANIS
                                 b. 1 Feb 1839
                                 bp. Fair Garden, TN or Cocke Co, TN ?
 
                      William Loyd MANIS
                       b. 5 Sep 1872
                       bp. Sevier Co, TN
 
                                Frances Amanda BIRD
                                 b. 8 FEB 1845
                                 bp. Sevier Co, TN
 
            Edith Alberta MANIS
             b. 8 APR 1914
             bp. Dandridge, Jefferson Co, TN
 
                                John Franklin NEWMAN
                                 b. 4 MAY 1830
                                 bp. Jefferson Co, TN
 
                      Lillie Caroline "Carolyn" NEWMAN
                       b. 13 JUN 1881
                       bp. Jefferson Co, TN
 
                                Mary Jean CORBETT
                                 b. 9 OCT 1843
                                 bp. Jefferson Co, TN

*/ 
 
/* End of Report */
