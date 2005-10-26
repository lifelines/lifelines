/*
 * @progname       bias.ll
 * @version        1.4
 * @author         Chandler
 * @category       
 * @output         Text
 * @description    

Ever notice that certain families seem to have all boys or all girls?
Sometimes five or six in a row of all the same sex?  Is this a mere
statistical fluctuation, or is something special happening?
This program gives statistics for male vs female births.  

Compute sex bias based on previous births

Version 1.3 - 1993 Aug 23 - John F. Chandler
Version 1.4 - 1994 Jul 19 (requires LL 3.0 or higher)

Ever notice that certain families seem to have all boys or all girls?
Sometimes five or six in a row of all the same sex?  Is this a mere
statistical fluctuation, or is something special happening?

This program gives statistics for male vs female births.  First, it
tabulates the number of males and females next born after each possible
proportion of previous births in the same family.  In particular, it
gives the sex tally of first-borns (where the proportion of previous
births is 0 males and 0 females), then the tally for second-borns where
the first child was a female (0+1), and so on.  Any combination that
doesn't actually occur in the database is skipped in the report (for
example, if no family is found with more than 3 sons, the tallies for
3+0, 3+1, and so on would all show a total of 0 males, and there would
be no tallies listed for 4+0, 4+1, and so on).

Children of unknown sex are not included in these statistics.

The program next prints out the relative excess of male births
(typically a positive value) over the nominally expected 50%.  For many
files, there is a tendency to include incomplete families with only one
known child; for this reason, "only" children are excluded from these
statistics.  Also, the male excess is computed for two different subsets
of the children: (A) the set of all children not born last, and (B) the
set of all children not born first.  For both of these, there is also a
measure of the variability of the sex ratio to put the percentages in
perspective.  In addition, the program prints out the correlation
between the sex ratio for children already born into a family and the
likelihood of getting a male (or female) as the *next* child.  If the
sample is unbiased, and if the sex of each child is truly random, this
correlation should be 0.

It also tallies the fraction of births matching the sex of the previous
birth in the same family (again, excluding any children of unknown sex).
These results are printed out for a succession of increasingly restricted
cases: first, for all births of non-first-borns; then, for births preceded
by two-in-a-row of the same sex; then, for three-in-a-row; and so on.

Bug: combinations with more than 9 sons or more than 9 daughters are not
listed properly.

This program works only with LifeLines.

*/

global(maxcount)	/* maximum attained runcount */
global(nextsex)		/* sex of next offspring in family */
global(prevsex)		/* sex of previous offspring in family */
global(runcount)	/* number of offspring so far in family */

/* Square Root function. */
func sqrt(x) {
	set(sqrtval,0)
	if(gt(x,0)) {
		set(sqrtval,1)
		set(approx,1)
		set(y,4096)
		while(le(y,x)) {	/* coarse grid */
			set(approx,y)
			set(sqrtval,mul(sqrtval,64))
			set(y,mul(y,4096))
		}
		set(y,mul(approx,4))
		while(le(y,x)) {	/* fine grid */
			set(approx,y)
			set(sqrtval,mul(sqrtval,2))
			set(y,mul(y,4))
		}
		set(count,0)
		while(and(ne(y,sqrtval),lt(count,9))) {
			set(y,div(x,sqrtval))
			set(sqrtval,div(add(y,sqrtval),2))
			set(count,add(1,count))
		}
	}
	return(sqrtval)
}

proc accstep(list) {
	set(x,1)
	while(le(x,runcount)) {
		setel(list,x,add(1,getel(list,x)))
		set(x,add(1,x))
	}
}

proc accum(samsex,difsex) {
	if(gt(runcount,0)) {
		if(strcmp(nextsex,prevsex)) {
			call accstep(difsex)
			set(runcount,0)
		} else { call accstep(samsex) }
	}
	set(prevsex,nextsex)
	set(runcount,add(1,runcount))
	if(gt(runcount,maxcount)) {set(maxcount,runcount)}
}

proc main ()
{

list(males)
list(fems)
list(samsex)
list(difsex)

set(totmales,0)
set(totfems,0)
set(onlymales,0)
set(onlyfems,0)

forfam (family, num) {
	set(count,0)
	set(runcount,0)
	children(family,child,fnum) {
		set(nextsex,sex(child))
		if(not(strcmp(nextsex,"M"))) {
			call accum(samsex,difsex)
			if(gt(count,0)) {
				set(totmales,add(1,totmales))
				setel(males,count,add(1,getel(males,count)))
			} else {set(onlymales,add(1,onlymales))}
			set(count,add(count,10))
		}
		elsif(not(strcmp(nextsex,"F"))) {
			call accum(samsex,difsex)
			if(gt(count,0)) {
				set(totfems,add(1,totfems))
				setel(fems,count,add(1,getel(fems,count)))
			} else {set(onlyfems,add(1,onlyfems))}
			if(gt(9,mod(count,10))) {set(count,add(count,1))}
			else { print("More than 9 daughters\n") }
		}
	}
}

/* Initialize statistics */
set(tot,add(totmales,totfems))
set(count,1)
set(nsample,0)
set(sumnfract,0)
set(sumpfract,0)
set(sumsqnfract,0)
set(sumsqpfract,0)
set(prodfract,0)
set(nrecs,0)

"Previous\nbirth       Next\nrecord      birth\nMF         M      F\n"
"00" col(sub(13,strlen(d(onlymales)))) d(onlymales)
col(sub(20,strlen(d(onlyfems)))) d(onlyfems) "  (excluded from statistics)\n\n"

while(lt(count,100)) {
	set(nmales,getel(males,count))
	set(nfems,getel(fems,count))
	if(or(nmales,nfems)) {
		set(nrecs,add(1,nrecs))
		if(lt(count,10)) { "0" }
		d(count) col(sub(13,strlen(d(nmales)))) d(nmales)
		col(sub(20,strlen(d(nfems)))) d(nfems) "\n"
		set(nsample,add(nsample,1))
		set(pboys,div(count,10))
		set(pgirls,mod(count,10))
		set(weight,add(nmales,nfems))
		set(p,add(pboys,pgirls))

/* scales: pf-100, sqpf-10000, nf-100, sqnf-10000, prod-10000
   i.e., express fractions as percent
   This makes integer arithmetic acceptable.
   Note that pfract is too small, on average, by 0.5, etc. */

		set(pfract,div(mul(100,sub(pboys,pgirls)),p))
		set(wtpfr,mul(weight,pfract))
		set(sumpfract,add(sumpfract,wtpfr))
		set(sumsqpfract,add(sumsqpfract,mul(pfract,wtpfr)))
		set(wtnfr,mul(100,sub(nmales,nfems)))
		set(nfract,div(wtnfr,weight))
/*		set(sumnfract,add(sumnfract,wtnfr)) -- use grand difference */
		set(sumsqnfract,add(sumsqnfract,mul(nfract,wtnfr)))
		set(prodfract,add(prodfract,mul(wtnfr,pfract)))
	}
	set(count, add(count,1))
}

"Total:" col(sub(13,strlen(d(totmales)))) d(totmales)
col(sub(20,strlen(d(totfems)))) d(totfems) "\n"
d(nrecs) " birth combinations found\n"
d(tot) " 'next' individuals (excluding firstborns)\n\n"

/* Make approximate corrections for roundoff errors */
set(sqcorr,mul(50,sub(totmales,totfems)))
set(sumnfract,mul(100,sub(totmales,totfems)))
set(sumsqnfract,add(sumsqnfract,sqcorr))
set(procfract,add(prodfract,sqcorr))
set(sumpfract,add(sumpfract,div(tot,2)))
set(sumsqpfract,sub(add(sumsqpfract,sumpfract),div(tot,3)))

set(sumsqpfract,sub(sumsqpfract,div(mul(sumpfract,sumpfract),tot)))
set(sumsqnfract,sub(sumsqnfract,div(mul(sumnfract,sumnfract),tot)))
set(prodfract,sub(prodfract,div(mul(sumpfract,sumnfract),tot)))
set(rssp,sqrt(sumsqpfract))
set(rssn,sqrt(sumsqnfract))
set(correl,div(mul(div(prodfract,rssp),100),rssn))
set(rmsp,sqrt(div(sumsqpfract,tot)))
set(rmsn,sqrt(div(sumsqnfract,tot)))

"Male excess of previous births= " d(div(sumpfract,tot)) "% +/- " d(rmsp) "%\n"
"Male excess of next births    = " d(div(sumnfract,tot)) "% +/- " d(rmsn) "%\n"
"Correlation between previous and next = " d(correl) "%\n"

set(count,1)
"\nFraction of births that match (in sex) a run of previous births in the"
"\nsame family.  Children of unknown sex ignored in this tabulation.\n"
"\nRun" col(sub(13,5)) "Total" col(sub(25,9)) "Matching"
"\nLength" col(sub(13,5)) "Cases" col(sub(23,5)) "Cases" col(sub(29,1)) "%\n"

while(le(count,maxcount)) {
	set(samesex,getel(samsex,count))
	set(diffsex,getel(difsex,count))
	set(allsex,add(diffsex,samesex))
	if(gt(allsex,0)) {
	  d(count) col(sub(13,strlen(d(allsex)))) d(allsex)
	  col(sub(23,strlen(d(samesex)))) d(samesex)
	  set(percent,d(div(mul(100,samesex),allsex)))
	  col(sub(29,strlen(percent))) percent "\n"
	}
	set(count,add(1,count))
	set(birth,"births")
}

}
