/*
 * @progname       pedigreel.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text, 132 cols
 * @description    
 *
 *   select and produce a ancestor report for the person selected.
 *   Ancestors report format, which print the event in long format.
 *   Output is an ASCII file, and will probably need to be printed
 *   using 132 column format.
 *
 *   pedigreel
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   name is from pedigreel  (long format)
 *
 *   An example of the output may be seen at end of this report.
 */

proc main ()
{
	set (nl,nl())
	getindi(indi)
	call pedigree(0, 1, indi)
	nl()
}

proc pedigree (in, ah, indi)
{
	if (par, father(indi)) {
		call pedigree(add(1,in), mul(2,ah), par)
	}
	print(name(indi)) print(nl())
	col(mul(8,in)) fullname(indi,1,1,50)
	if (evt, birth(indi)) { ", b. " long(evt) }
	" (" d(ah) ")" nl()
	if (par, mother(indi)) {
		call pedigree(add(1,in), add(1,mul(2,ah)), par)
	}
}

/*  Sample output of this report.    132 Column Format.  

    This report was requested for "Fuller Ruben Manes".


                               John MANESS, b. ca 1770-1780 (16)
                       Samuel P. MANES, b. ca 1780-90 (8)
               William Thomas MANES, b. 26 Nov 1828, Hamblen, Tennessee (4)
                       Fanny (MANES), b. ca 1790-1800 (9)
       William Bowers MANES, b. 6 Jan 1868, Hamblen Co, TN ? (2)
                               James BOWERS (20)
                       Anderson BOWERS, b. ca 1803, TN (10)
                               Martha  (21)
               Martha A. BOWERS, b. 14 APR 1829, TN (5)
                                               William COWAN (88)
                                       Samuel COWAN (44)
                                               Mrs. (COWAN) (89)
                               Christopher Columbus COWAN, b. About 1765 (22)
                                       Mrs (COWAN) (45)
                       Lurina Viney "Vina" COWAN, b. 1808, TN (11)
                               Mary BOYD, b. 1772, Boyd's Creek, Sevier Co, TN (23)
Fuller Ruben MANES, b. 19 Nov 1902, Union Valley, Sevier Co, TN (1)
                       Henry B. CANTER, b. ca 1820, VA (12)
               James H. CANTER, b. ca 1847, Claiborne Co, TN (6)
                       Polina (CANTER), b. ca 1822 (13)
       Cordelia "Corda" F. CANTER, b. 7 Dec 1869, Jonesboro, Washington Co, TN (3)
                       James WHITEHORN, b. VA (14)
               Martha Marie WHITEHORN, b. 22 DEC 1846, Washington Co, TN ? (7)
                                       Thomas FOSTER (60)
                               Kennedy "Kan" Powell FOSTER, b. 1814 (30)
                       Martha "Patsy" FOSTER, b. Tennessee (15)
                                       David CASON (62)
                               Rebecca KERSAWN, b. 1818, NC (31)
                                       Mary  (63)

*/

/* End of Report */

