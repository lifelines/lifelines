/*
 * @progname       8gen1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text, 132 cols
 * @description    
 *
 *   Produce an 8 generation descendant report for the person selected.
 *   Output is an ASCII file, and will probably need to be printed
 *   using 132 column format.
 *
 *   8gen1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *
 *   Produce an 8 generation descendant report for the person selected.
 *
 *   Output is an ASCII file, and will probably need to be printed
 *   using 132 column format.
 *
 *   An example of the output, is not included because it would add
 *   20k to this report.
 */

proc main ()
{
	getindi(indi)
	set (nl, nl())
	pagemode(260,132)
	call pedout(indi,1,8,1,256)
	pageout()
	print(nl())
}

proc pedout (indi, gen, max, top, bot)
{
	if (le(gen,max)) {
		set(gen,add(1,gen))
		set(fath,father(indi))
		set(moth,mother(indi))
		set(height,add(1,sub(bot,top)))
		set(offset,div(sub(height,1),2))
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
	set(e,birth(indi))
	pos(row,col)
	if (indi) { name(indi) 
	", "
	if (and(e,date(e))) { date(e) }
	", "
	if (and(e,place(e))) { place(e) }
	}

	else      { "   |--------" }
}

/* End of Report */

