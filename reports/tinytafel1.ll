/*
 * @progname       tinytafel1.ll
 * @version        3.1
 * @author         Wetmore, Eggert, Chandler
 * @category       
 * @output         TinyTafel
 * @description    

This report will produce a tinytafel report on a person.

tinytafel1

Based on tinytafel1 by Tom Wetmore, ttw@cbnewsl.att.com

Version 1,	  1991, by Tom Wetmore.
Version 2, 11 Jan 1993, by Jim Eggert, eggertj@ll.mit.edu,
			added header, trailer, sorting, date fixing,
			and default moderate interest.	Modified
			empty surname recognition.
Version 3, Jan 1994, J. F. Chandler, fixed count, enhanced date/place guessing.
Version 3.1		Mark guessed places with "?"

This report will produce a tinytafel report on a person.

Output is an ASCII file.  It should be edited to translate any
non-ASCII characters, to shorten long place names (to 14-16
characters), and to indicate interest level after each year:
   [space] No interest (level 0)
   .	   Low interest (level 1)
   :	   Moderate interest (level 2) (default)
   *	   Highest interest (level 3)

You will want to modify the write_tafel_header() procedure to
include your name, address, etc.

Empty surnames or those starting with "_" or " " will not
be written to the report.

See the end of this report for an example of a tinytafel report.
*/

global(tafelset)
global(fdatelist)
global(ldatelist)
global(fplacelist)
global(lplacelist)
global(line_count)

global(fdate)
global(ldate)
global(pdate)
global(fplace)
global(lplace)
global(pplace)
global(sname)
global(datemod) /* value returned by get_modifier */
global(pdmax)
global(pdmin)

/* Assumptions for guessing year of birth */
global(Minpar)	/* assumed minimum age of parenthood */
global(Typicl)	/* typical age for parenthood or marriage */
global(Menopa)	/* assumed maximum age of motherhood */
global(Oldage)	/* assumed age at death */

proc write_tafel_header() {
    forindiset(tafelset,person,index,snum) {set(lines,snum)}
    "N John Q. Public\n"           /* your name, mandatory */
    "A 1234 North Maple\n"         /* address, 0-5 lines */
    "A Homesville, OX 12345-6789\n"
    "A USA\n"
    "T 1 (101) 555-1212\n"         /* telephone number */
    "C 19.2 Baud, Unix System\n"   /* communications */
    "C Send any Email to:  jqpublic@my.node.address\n"
    "B SoftRoots/1-101-555-3434\n" /* BBS system/phone number */
    "D Unix Operating System\n"    /* diskette formats */
    "F LifeLines Genealogy Program for Unix\n"  /* file format */
    "R This is a default header, please ignore.\n"  /* comments */
    "Z " d(lines) "\n"
}

proc main ()
{
    /* Assumptions for guessing year of birth */
    set(Minpar,14)	/* assumed minimum age of parenthood */
    set(Typicl,20)	/* typical age for parenthood or marriage */
    set(Menopa,50)	/* assumed maximum age of motherhood */
    set(Oldage,60)	/* assumed age at death */

    list(plist)
    indiset(tafelset)
    list(fdatelist)
    list(ldatelist)
    list(fplacelist)
    list(lplacelist)
    set(line_count,0)

    getindi(person)
    enqueue(plist, person)
    while (person, dequeue(plist)) {
	call process_line(person, plist)
    }
    namesort(tafelset)
    call write_tafel_header()
    call write_tafelset()
    call write_tafel_trailer()
}

proc write_tafelset() {
    forindiset(tafelset,person,index,snum) {
	soundex(person) " "
	getel(ldatelist,index) ":" /* moderate interest by default */
	getel(fdatelist,index) ":"
	surname(person)
	if (lplace,getel(lplacelist,index)) { "\\" lplace }
	if (fplace,getel(fplacelist,index)) { "/" fplace }
	"\n"
    }
}

proc write_tafel_trailer() {
    "W " date(gettoday()) "\n"
}

proc process_line (person, plist)
{
    call first_in_line(person)
    set(initial,trim(sname,1))
    if (and(and(strcmp(initial, "_"),
		strcmp(initial, " ")),
	    strcmp(sname,""))) {
	set(last, 0)
	while (person) {
	    print(".")
	    if (moth, mother(person)) {
		enqueue(plist, moth)
	    }
	    set(last, person)
	    set(person, father(person))
	    if (strcmp(sname, surname(person))) {
		call last_in_line(last)
		if(person) {call first_in_line(person)}
	    }
	}
    }
}

proc first_in_line (person)
{
    call set_year_place(person)
    set(fdate, pdate)
    set(pl, pplace)
    if (not(pl)) {	/* try for a supportable guess */
	list(places)
	if(fath,father(person)) {
	    if(pl,place(death(fath))) {enqueue(places,save(pl))}
	    if(pl,place(birth(fath))) {enqueue(places,save(pl))}
	    families(fath,fam,sp,spi) {
	        if(pl,place(marriage(fam))) {enqueue(places,save(pl))}
	    }
	}
	if(moth,mother(person)) {
	    if(pl,place(death(moth))) {enqueue(places,save(pl))}
	    if(pl,place(birth(moth))) {enqueue(places,save(pl))}
	}
	families(person,fam,sp,spi) {
	    if(pl,place(marriage(fam))) {enqueue(places,save(pl))}
	}
/* the person's place of death is often misleading */
/*	if(pl,place(death(person))) {enqueue(places,save(pl))} */
	set(npl,length(places))
	while (gt(npl,1)) {
	    set(pl,dequeue(places))
	    set(npl,sub(npl,1))
	    set(ind,1)
	    while(le(ind,npl)) {
		if(not(strcmp(pl,getel(places,ind)))) {set(npl,neg(1))}
		set(ind,add(ind,1))
	    }
	}
	if(ge(npl,0)) {set(pl,0)}
	if(pl) {set(pl,concat(pl,"?"))}
    }
    set(fplace,save(pl))
    set(sname,save(surname(person)))
}

proc last_in_line (person)
{
    call set_year_place(person)
    set(ldate, pdate)
    set(lplace, pplace)
    set(line_count,add(line_count,1))
    addtoset(tafelset,person,line_count)
    if (and(strcmp(ldate,"????"), gt(strcmp(ldate,fdate),0))) {
	print("\nInconsistent dates for surname ")
	print(sname)
    }
    enqueue(ldatelist,save(ldate))
    enqueue(fdatelist,save(fdate))
    enqueue(lplacelist,save(lplace))
    enqueue(fplacelist,save(fplace))
}

/* set global variable datemod to +1 if event's date is marked AFT,
   -1 if marked BEF, and 0 otherwise */

proc get_modifier(event)
{   set (datemod,0)
    if (junk,date(event)) {
	set (junk,trim(junk,3))
	if(not(strcmp(junk,"AFT"))) { set (datemod,1) }
	elsif(not(strcmp(junk,"BEF"))) { set (datemod,neg(1)) }
    }
}

/* get birth-year for given person -- use whatever clues available, in
this order.  The culture-dependent limits are defined in "main".

	1. birth
	2. baptism
	3. birth of older sibling (+2)
	4. birth of younger sibling (-2)
	5. baptism of younger sibling (upper limit only)
	6. birth of parent (+14: lower limit only)
	7. death of parent (upper limit only)
	8. marriage or birth of first child (-20: recursive)
	9. marriage or birth of first child (-14: recursive upper limit)
	9. birth of last child (-50: lower limit only)
	10. death, known to be a parent (-60)
	11. death, not known to be a parent
*/
proc set_year (person)
{   set (maxyr,9999)			/* set upper bound */
    set (minyr,0)			/* and lower bound */
    set (guess,0)			/* clear "best" guess */
    if (yr, year(birth(person))) {	/* solid data */
	call get_modifier(birth(person))
	set (iyr,atoi(yr))
	if(ge(datemod,0)) {set(minyr,iyr)}
	if(le(datemod,0)) {set(maxyr,iyr)}
	if(datemod) {set (yr,0)}
    }
    if (not(yr)) {
	if (yr, year(baptism(person))) {	/* pretty good guess */
	    set(iyr,atoi(yr))
	    call get_modifier(baptism(person))
	    if(and(le(datemod,0),lt(iyr,maxyr))) {set(maxyr,iyr)}
	    set (guess, iyr)
	}

	if(sibl,prevsib(person)) {	/* try older sibling */
	    if (yr, year(birth(sibl))) {
		call get_modifier(birth(sibl))
		if(ge(datemod,0)) {
		    set (iyr,atoi(yr))
		    if(gt(iyr,minyr)) {set(minyr,iyr)}
		    if(not(or(guess,datemod))) {set(guess,add(iyr,2))}
		}
	    }
	}
	if(sibl,nextsib(person)) {	/* try younger sibling */
	    if (yr, year(birth(sibl))) {
		call get_modifier(birth(sibl))
		if(le(datemod,0)) {
		    set (iyr,atoi(yr))
		    if(lt(iyr,maxyr)) {set(maxyr,iyr)}
		    if(not(or(guess,datemod))) {set(guess,sub(iyr,2))}
		} else {set(yr,0)}
	    }
	    if (not(yr)) {
		if (yr, year(baptism(sibl))) {
		    set(iyr,atoi(yr))
		    call get_modifier(baptism(sibl))
		    if(and(le(datemod,0),lt(iyr,maxyr))) {set(maxyr,iyr)}
		}
	    }
	}

	if(sp,mother(person)) {		/* set limits from mother */
	    if(yr,year(birth(sp))) {
		call get_modifier(birth(sp))
		set(iyr,add(atoi(yr),Minpar))
		if(and(ge(datemod,0),gt(iyr,minyr))) {set(minyr,iyr)}
	    }
	    if(yr,year(death(sp))) {
		call get_modifier(death(sp))
		set(iyr,atoi(yr))
		if(and(le(datemod,0),lt(iyr,maxyr))) {set(maxyr,iyr)}
	    }
	}


	if(sp,father(person)) {		/* set limits from father */
	    if(yr,year(birth(sp))) {
		call get_modifier(birth(sp))
		set(iyr,add(atoi(yr),Minpar))
		if(and(ge(datemod,0),gt(iyr,minyr))) {set(minyr,iyr)}
	    }
	    if(yr,year(death(sp))) {
		call get_modifier(death(sp))
		set(iyr,add(atoi(yr),1))
		if(and(le(datemod,0),lt(iyr,maxyr))) {set(maxyr,iyr)}
	    }
	}

	set(maryr,9999)			/* marriage date or upper limit */
	set(marbest,9999)		/* best guess at marriage date */
	set(lastbirth,0)
	families(person,fam,sp,spi) {	/* check on marriage/chidren */
	    if(yr, year(marriage(fam))) {
	        call get_modifier(marriage(fam))
	        set(iyr,atoi(yr))	/* go by marriage date */
	        if(and(le(datemod,0),lt(iyr,maryr))) {set(maryr,iyr)}
	        if(and(le(datemod,0),lt(iyr,marbest))) {set(marbest,iyr)}
	    }
	    if(or(eq(maryr,9999),female(person))) {
	        children (fam,child,famchi) {
	    	call set_year(child)	/* recurse on children */
	    	if(lt(pdmax,maryr)) {set(maryr,pdmax)}
	    	if(strcmp(pdate,"????")) {
	    	    set(iyr,atoi(pdate))
	    	    if(lt(iyr,marbest)) {set(marbest,iyr)}
	    	}
	    	if(gt(pdmin,lastbirth)) {set(lastbirth,pdmin)}
	    			/* get earliest & latest child */
	        }
	    }
	}
	if(eq(marbest,9999)) {set(marbest,maryr)}
	if(lt(maryr,9999)) {
	    set(iyr,sub(maryr,Minpar))	/* assume biological limit */
	    if(lt(iyr,maxyr)) {set(maxyr,iyr)}
	    if(not(guess)) {set(guess,sub(marbest,Typicl))}  /* typical age */
	}
	if(gt(lastbirth,0)) {
	    set(iyr,sub(lastbirth,Menopa))	/* another biological limit */
	    if(gt(iyr,minyr)) {set(minyr,iyr)}
	}
	if (yr, year(death(person))) {call get_modifier(death(person))}
	elsif (yr, year(burial(person))) {call get_modifier(burial(person))}
	if (yr) {
	    set (iyr, atoi(yr))
	    if(and(le(datemod,0),lt(iyr,maxyr))) {set(maxyr,iyr)}
	    if(not(guess)) {			/* still need a guess? */
		if(nfamilies(person)) {
		    set(guess,sub(iyr,Oldage))} /* died old */
		else {set(guess,iyr)}		/* no family => died young */
	    }
	}

	if (gt(guess,maxyr)) { set(guess,maxyr) } /* apply limit, in case... */
	if (lt(guess,minyr)) { set(guess,minyr) }
	if (gt(guess,0)) {set (yr,d(guess))}
    }
    if (not(yr)) { set (yr, "????") }
    set(pdate, save(yr))		/* values returned */
    set(pdmin,minyr)
    set(pdmax,maxyr)
}

proc set_year_place (person)
{
    call set_year (person)
    set(pl, place(birth(person)))
    if (not(pl)) {set(pl, place(baptism(person)))}
    set(pplace, save(pl))
}


/*

Here is an example of a tiny tafel by Cliff Manis.

Note that the "Z" line is the number of actual data lines.

N Alda Clifford Manis
A P. O. Box 33937
A San Antonio
A Texas
A 78265-3937
T 1 (512) 654-9912
C 19.2 Baud, Unix System
C Send any Email to:  cmanis@csoftec.csf.com
D Unix Operating System
F LifeLines Genealogy Program for Unix
Z 16
M520 1939 1939 Manis\Knoxville, Knox Co, TN/Knoxville, Knox Co, TN
M520 1780 1902 Manes\Sevier Co, TN ?/Union Valley, Sevier Co, TN
M520 1770 1770 Maness\Sevier Co, Tennessee ?/Sevier Co, Tennessee ?
M520 1805 1914 Manis\North Carolina ?/Dandridge, Jefferson Co, TN
C536 1820 1869 Canter\VA/Jonesboro, Washington Co, TN
B620 1765 1829 Bowers/TN
N550 1730 1881 Newman\Monroe Co., WV/Jefferson Co, TN
B630 1760 1845 Bird\Frederick Co, VA/Sevier Co, TN
B630 1730 1730 Barth\Germany/Germany
F652 1745 1810 Francis\Augusta Co, VA ?/Rutherford Co, NC
W365 1860 1846 Whitehorn\VA/Washington Co, TN ?
C500 1700 1808 Cowan/TN
C613 1720 1843 Corbett\Scotch-Irish Dec/Jefferson Co, TN
R525 1750 1806 Rankin\Scotland/Jefferson Co., TN
S636 1776 1799 Shrader\Virginia/Sevier Co, TN ?
B300 1772 1772 Boyd\Boyd's Creek, Sevier Co, TN/Boyd's Creek, Sevier Co, TN
W 24 September 1992

*/

/* End of Report */
