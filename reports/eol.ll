/*
 * @progname    eol.ll
 * @version     4 of 1995-01
 * @author      Tom Wetmore and John Chandler
 * @category
 * @output      Text, 80 cols
 * @description
 *
Say you want to know who all of your end-of-line ancestors are, that
is, your direct ancestors whose parents you have not yet discovered;
these are the people most of us spend most of our time on researching.
Here is a program that will produce the list.  Any ancestor will be
listed at most once, even in cases where lines cross.  Each person
is shown with date and place of birth and death -- a "guess" is made
for the year of birth if it is not known.

Set the Do_all variable to 1 if you want the end-of-line list to
include even persons with an unknown surname.

This program shares the birth-guessing subroutine with tinytafel.
*/

global(plist)
global(abbvtab)

/* Global definitions for birth-guessing subroutine */
global(pdate)
global(pplace)
global(datemod) /* value returned by get_modifier */
global(pdmax)
global(pdmin)

/* Assumptions for guessing year of birth */
global(Minpar)	/* assumed minimum age of parenthood */
global(Typicl)	/* typical age for parenthood or marriage */
global(Menopa)	/* assumed maximum age of motherhood */
global(Oldage)	/* assumed age at death */

proc main () {
	set(Do_all,0)	/* if 0, then ignore surnameless persons */

	/* Assumptions for guessing year of birth */
	set(Minpar,14)	/* assumed minimum age of parenthood */
	set(Typicl,20)	/* typical age for parenthood or marriage */
	set(Menopa,50)	/* assumed maximum age of motherhood */
	set(Oldage,60)	/* assumed age at death */

	list(ilist)
	list(plist)
	list(pnlist)
	table(seen)
	table(abbvtab)
	indiset(set)
	getindi(indi)
	monthformat(4)
	"END OF LINE ANCESTORS OF " fullname(indi,1,1,30) "\n\n"
	call setupabbvtab()
	enqueue(ilist, indi)
	while(indi, dequeue(ilist)) {
		set(show, 1)
		if (par, father(indi)) {
			set(do_this,Do_all)
			if(not(Do_all)) {
				extractnames (inode(par),pnlist,n,s)
				set(do_this, strcmp(getel(pnlist,s),""))
			}
			if(do_this) {
				enqueue(ilist, par)
				set(show, 0)
			}
		}
		if (par, mother(indi)) {
			set(do_this,Do_all)
			if(not(Do_all)) {
				extractnames (inode(par),pnlist,n,s)
				set(do_this, strcmp(getel(pnlist,s),""))
			}
			if(do_this) {
				enqueue(ilist, par)
				set(show, 0)
			}
		}
		if (show) {
			set(pkey, key(indi))
			if(not(lookup(seen,pkey))) {
				insert(seen,pkey,1)
				addtoset(set, indi, pkey)
			}
		}
	}
	namesort(set)
	forindiset (set, indi, val, num) {
		col(1) fullname(indi,1,0,27)
		call set_year_place(indi)
		call showevent(29, birth(indi), pdate, pplace)
		call showevent(55, death(indi), 0, 0)
		nl()
	}
}

proc showevent (column, event, apdate, applace)
{
	col(column)
	set(column, add(column, 12))
	if(year(event)) {
		stddate(event) sp()
	}
	elsif(apdate) { "      c" apdate " " }
	extractplaces(event, plist, num)
	if (and(applace,eq(num,0))) {
		call extractstr(applace,plist)
		set(num,length(plist))
	}
	if (gt(num, 0)) {
		col(column)
		set(last, getel(plist, num))
		if (yes, lookup(abbvtab, last)) {
			set(last, yes)
		}
		trim(last, 10)
	}
}

proc extractstr (string,list) {
	list(list)
	call ext_step(list,string,1,strlen(string),0)
}
proc ext_step(list,string,start,len,nth) {
	if(gt(start,len)) {return()}
	set(nth,add(1,nth))
	if (not(strcmp(substring(string,start,start)," "))) {
		set(start,add(1,start))
	}
	set(end, sub(index(string, ",", nth),1))
	if(lt(end,0)) {set(end,len)}
	enqueue (list, substring(string,start,end))
	if (lt(end,len)) {call ext_step(list,string,add(end,2),len,nth)}
}

proc setupabbvtab ()
{
	insert(abbvtab, "Connecticut", "CT")
	insert(abbvtab, "Connecticut Colony", "CT")
	insert(abbvtab, "New Haven Colony", "CT")
	insert(abbvtab, "Massachusetts", "MA")
	insert(abbvtab, "Plymouth Colony", "MA")
	insert(abbvtab, "New York", "NY")
	insert(abbvtab, "England", "ENG")
	insert(abbvtab, "Holland", "HOL")
	insert(abbvtab, "Maryland", "MD")
	insert(abbvtab, "Wales", "WLS")
	insert(abbvtab, "Isle of Man", "IOM")
	insert(abbvtab, "Nova Scotia", "NS")
	insert(abbvtab, "Ireland", "IRE")
	insert(abbvtab, "Rhode Island", "RI")
	insert(abbvtab, "prob England", "ENG?")
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
    set(pdate, yr)		/* values returned */
    set(pdmin,minyr)
    set(pdmax,maxyr)
}

proc set_year_place (person)
{
    call set_year (person)
    set(pl, place(birth(person)))
    if (not(pl)) {set(pl, place(baptism(person)))}
    set(pplace, pl)
}
