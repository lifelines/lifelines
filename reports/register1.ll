/*
 * @progname       register1.ll
 * @version        1.0
 * @author         Wetmore
 * @category       
 * @output         nroff
 * @description    
 *
 *   It will produce a report of all descendents of a person,
 *   and is presently designed for 12 pitch, HP laserjet III.
 *   All NOTE and CONT lines from data will be printed in the this report.
 *   This report will produce a paginated output.   It is similiar
 *   to the report 'regvital1'.
 *
 *   register1
 *
 *   This report does NOT have a footer and header
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   and it has been modified many times since.
 *
 *   This report produces a nroff output, and to produce the
 *   output, use:  nroff filename > filename.out
 *
 */


proc main ()
{
    getindi(indi)
    ".de hd" nl()
    "'sp .8i" nl()
    ".." nl()
    ".de fo" nl()
    "'bp" nl()
    ".." nl()
    ".wh 0 hd" nl()
    ".wh -.8i fo" nl()
    ".de CH" nl()
    ".sp" nl()
    ".in 16n" nl()
    ".ti 0" nl()
    "\h'5n'\h'-\w'\\$1'u'\\$1\h'8n'\h'-\w'\\$2'u'\\$2\h'2n'" nl()
    ".." nl()
    ".de IN" nl()
    ".sp" nl()
    ".in 0" nl()
    ".." nl()
    ".de GN" nl()
    ".br" nl()
    ".ne 2i" nl()
    ".sp 2" nl()
    ".in 0" nl()
    ".ce" nl()
    ".." nl()
    ".de P" nl()
    ".sp" nl()
    ".in 0" nl()
    ".ti 5" nl()
    ".." nl()
    ".po 3" nl()
    ".ll 7i" nl()
    ".ls 1" nl()
    ".na" nl()
    list(ilist) list(glist)
    table(stab) indiset(idex)
    enqueue(ilist,indi)  enqueue(glist,1)
    set(curgen,0)  set(out,1)  set(in,2)
    while (indi,dequeue(ilist)) {
        print("OUT: ") print(d(out))
        print(" ") print(name(indi)) print(nl())
	set(thisgen,dequeue(glist))
	if (ne(curgen,thisgen)) {
	    ".GN" nl() "GENERATION " d(thisgen) nl() nl()
	    set(curgen,thisgen)
	}
        ".IN" nl() d(out) ". "
        insert(stab,save(key(indi)),out)
        call longvitals(indi)
	addtoset(idex,indi,0)
	set(out,add(out,1))
	families(indi,fam,spouse,nfam) {
	    ".P" nl()
	    if (spouse) { set(sname, save(name(spouse))) }
	    else        { set(sname, "_____") }
	    if (eq(0,nchildren(fam))) {
		name(indi) " and " sname
		" had no children." nl()
	    } elsif (and(spouse,lookup(stab,key(spouse)))) {
		"Children of " name(indi) " and " sname " are shown "
		"under " sname " (" d(lookup(stab,key(spouse))) ")." nl()
	    } else {
		"Children of " name(indi) " and " sname":" nl()
		children(fam,child,nchl) {
                    set(haschild,0)
                    families(child,cfam,cspou,ncf) {
			if (ne(0,nchildren(cfam))) { set(haschild,1) }
		   }
		   if (haschild) {
                        print("IN:  ") print(d(in))
                        print(" ") print(name(child)) print(nl())
			enqueue(ilist,child)
			enqueue(glist,add(1,curgen))
			".CH " d(in) " " roman(nchl) nl()
			set (in, add (in, 1))
                        call shortvitals(child)
		    } else {
                        ".CH " qt() qt() " " roman(nchl) nl()
                        call longvitals(child)
			addtoset(idex,child,0)
		    }
		}
	    }
	}
    }
}
proc shortvitals(indi)
{
	name(indi)
	set(b,birth(indi)) set(d,death(indi))
	if (and(b,short(b))) { ", b. " short(b) }
	if (and(d,short(d))) { ", d. " short(d) } nl()
}
proc longvitals(i)
{
	name(i) "." nl()
	set(e,birth(i))
	if(and(e,long(e))) { "Born " long(e) "." nl() }
	if (eq(1,nspouses(i))) {
        	spouses(i,s,f,n) {
			"Married"
			set(nocomma,1)
			call spousevitals(s,f)
        	}
	} else {
		spouses(i,s,f,n) {
			"Married " ord(n) ","
			call spousevitals(s,f)
		}
	}
	set(e,death(i))
	if(and(e,long(e))) { "Died " long(e) "." nl() }
	fornotes(inode(i), note) {
		note nl()
	}
}

proc spousevitals (spouse,fam)
{
	set(e,marriage(fam))
	if (and(e,long(e))) { nl() long(e) "," }
	nl() name(spouse)
	set(e,birth(spouse))
	if(and(e,long(e)))  { "," nl() "born " long(e) }
	set(e,death(spouse))
	if(and(e,long(e)))  { "," nl() "died " long(e) }
	set(dad,father(spouse))
	set(mom,mother(spouse))
	if (or(dad,mom)) {
		"," nl()
		if (male(spouse))      { "son of " }
		elsif (female(spouse)) { "daughter of " }
		else               { "child of " }
	}
	if (dad)          { name(dad) }
	if (and(dad,mom)) { nl() "and " }
	if (mom)          { name(mom) }
	"." nl()
}


/*   Sample output of this report, it is paginated but I have not shown
     that in this example.


                                   GENERATION 1

         1. Fuller Ruben MANES.  Born 19 Nov 1902, Union Valley, Sevier
         Co, TN.  Married 17 OCT 1936, Knoxville, TN, Edith Alberta MANIS,
         born 8 APR 1914, Dandridge, Jefferson Co, TN, died 18 JUN 1992,
         Knoxville, Knox Co, TN, daughter of William Loyd MANIS and Lillie
         Caroline "Carolyn" NEWMAN.  Died 20 Jun 1980, Knoxville, Knox Co,
         TN.  Fuller's first fifteen years were growing up on a farm.  By
         the time he was 10 years old, he had 9 other brothers and sisters
         to help feed and care for, play with, and the many facets of work
         which had to be done each day.  "Clifford" and "Snowball" were
         some of his nicknames. Pictures show him (many times) in a
         three-piece suit and a man of many places.  As most men, during
         his youth, he was photographed in the presence with several
         different females.  He attended school at Harrison Chilhowee
         Baptist Academy, which a walk of about 5 or 6 miles each way from
         his home.  He boarded at the school dormitory for an unknown
         period of time.

              Children of Fuller Ruben MANES and Edith Alberta MANIS:

                     i   Ellsworth Howard MANIS.  Born 11 MAR 1939,
                         Knoxville, Knox Co, TN.  Died 13 MAR 1939,
                         Knoxville, TN,.  Was the first born of twins,
                         birth two-forty PM, at Harrison-Henderson
                         Hospital.  Ellsworth died at age 44 hours, was a
                         twin to Alda Clifford MANIS.  Buried 13 Mar 1939
                         at Seven Islands Cem, NE Knox County, TN (near
                         Jefferson and Sevier County line).

             2      ii   Alda Clifford MANIS, b. 1939, TN


                                   GENERATION 2


         2. Alda Clifford MANIS.  Born 11 MAR 1939, Knoxville, Knox Co,
         TN.  Married first, 8 SEP 1962, Knoxville, Knox Co, TN, Joyce
         Fern OWENS, born 1 APR 1942, Knoxville, Knox Co, TN, daughter of
         Guy Hixon OWENS and Bertha Mae TURNER.  Married second, 13 FEB
         1984, San Antonio, Texas, Marianne Florence KRAMER, born 19 MAY
         1943, Los Angeles, CA, daughter of Anthony Leo KRAMER and
         Florence Rita BOSSO.  Born at two-forty five PM, Harrison-
         Henderson Hospital.  Twin of Elsworth Howard MANIS.  Clifford was
         born second.

              Children of Alda Clifford MANIS and Joyce Fern OWENS:

             3       i   Gregory Scott MANIS, b. 1963, VA

                    ii   Sheila Ann MANIS.  Born 7 APR 1968, Mexico City,
                         Mexico DF.

              Alda Clifford MANIS and Marianne Florence KRAMER had no
         children.


*/

/* end of report */
