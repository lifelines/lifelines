/*
 * @progname       alive.ll
 * @version        6
 * @author         Jim Eggert
 * @category       
 * @output         Text
 * @description

This report program is for finding people living in a given year.
This finds who in the database, or among ancestors or descendants of
an individual, was likely alive in a certain year.  Good for looking at
population snapshots like censuses, tax rolls, etc.

Version 1, 13 July 1994, by Jim Eggert, eggertj@ll.mit.edu
Version 2, 14 July 1994, by Jim Eggert, fixed bug in estimate_byear
Version 3, 22 July 1994, by Jim Eggert, fixed another bug in estimate_byear,
					minor format improvement
Version 4, 15 March 1995, by Frank Flaesland, added support for listing places
Version 5, 17 March 1995, J.F.Chandler, modified to prune place list
Version 6, 27 Aug   1997, by Jim Eggert, fixed bug in print_header()

** SourceForge Versions:
**
** Revision 1.6  2004/07/19 05:54:54  dr_doom
** Merge Vincent Broman Changes to reports
**
** Revision 1.4  2000/11/29 12:17:11  nozell
** Fix typo.
**
** Revision 1.3  2000/11/28 21:39:45  nozell
** Add keyword tags to all reports
** Extend the report script menu to display script output format
**
** Revision 1.2  2000/11/11 17:48:13  pere
** Get this report working.  Fixed type problems and handle empty
** place tags without crashing.  Add meta information.
**
**
*/

global(byear)
global(byear_delta)
global(byear_est)
global(byear_est_delta)
global(dyear_est)
global(dyear_est_delta)
global(old_age)
global(maximum_age)
global(mother_age)
global(father_age)
global(years_between_kids)
global(first_person)
global(who)
global(of)
global(places)

proc main() {
    /* Assumptions for guessing year of birth */
    set(old_age,60)	/* assumed age at death */
    set(maximum_age,120)/* maximum possible age */
    set(mother_age,23)	/* assumed age of first motherhood */
    set(father_age,25)	/* assumed age of first fatherhood */
    set(years_between_kids,2) /* assumed years between children */

    indiset(people)
    set(first_person,1)

    getintmsg(who,
	"Find live persons (0=all, 1=desc, 2=desc and spouses, 3=anc) ")
    if (who) { getindimsg(of,"of ") }
    getintmsg(when,"alive in which year?")
    set(places, 1)
    getstrmsg(yesno,"List possible places? (y/n) ")
    if (strlen(yesno)) {
      if (strcmp(upper(trim(yesno,1)),"Y")) { set(places,0) }
    }

    if (eq(who,0)) {
	forindi(person,pnum) {
	    call alive(person,when)
	}
    }
    else {
	addtoset(people,of,0)
	if (or(eq(who,1),eq(who,2))) { set(people,descendentset(people)) }
	elsif (eq(who,3)) { set(people,ancestorset(people)) }
	addtoset(people,of,0)
	if (eq(who,2)) { set(people,union(people,spouseset(people))) }
	forindiset(people,person,pval,pnum) {
	    call alive(person,when)
	}
    }
}

proc print_person(person) {
    key(person)
    col(9) fullname(person,0,1,50)
    col(61) "("
    if (gt(byear_est_delta,1)) { "c" }
    d(byear_est)
    "-"
    if (gt(dyear_est_delta,1)) { "c" }
    d(dyear_est)
    ")\n"
    if (eq(places,1)) { call print_places(person) }
}

proc print_places(person) {
    list(place_names)
    table(places_seen)
    traverse (inode(person), node, level) {
	if (eq(strcmp(tag(node), "PLAC"), 0)) {
	    set(p, value(node))
	    if(lookup(places_seen,p)) { continue() }
	    insert(places_seen, p, 1)
	    extractplaces(node, place_names, num_places)
	    if (gt (num_places,0)) {
		"            " pop(place_names)
		while (p, pop(place_names)) { ", " p }
		"\n"
	    }
	}
    }
}

proc print_header(year) {
    set(current_year,strtoint(year(gettoday())))
    if (ge(year,current_year)) { set(future,1) } else { set(future,0) }

  "________________________________________________________________________\n"
    "List of "
    if (eq(who,0)) { "all persons" }
    elsif (or(eq(who,1),eq(who,2))) { "descendants" }
    elsif (eq(who,3)) { "ancestors" }
    if (ge(who,1)) { " of\n" key(of) " " fullname(of,0,1,70) "\n" }
    else { " " }
    if (eq(who,2)) { "and their spouses" }
    if (future) { "who are likely to be" }
    else { "who are likely to have been" }
    " alive in " d(year) "\n\n"
  "________________________________________________________________________\n"
    "Key" col(9) "Name" col(61) "(born-died)\n"
  "________________________________________________________________________\n"
}

proc alive(person,year) {
    set(dyear_est,0)
    call estimate_byear(person)
    if (byear_est) {
	if (and(le(byear_est,add(year,byear_est_delta)),
		gt(byear_est,sub(year,maximum_age)))) {
	    set(dyear_est,atoi(year(death(person))))
	    if (not(dyear_est)) {
		set(dyear_est,atoi(year(burial(person))))
	    }
	    else { set(dyear_est_delta,0) }
	    if (not(dyear_est)) {
		set(dyear_est,add(byear_est,old_age))
		set(dyear_est_delta,20)
	    }
	    else { set(dyear_est_delta,1) }
	    if (ge(dyear_est,year)) {
		if (first_person) {
		    call print_header(year)
		    set(first_person,0)
		}
		call print_person(person)
	    }
	}
    }
}

proc estimate_byear(person) {
    set(byear_est,0)
    set(byear_est_delta,neg(1))
    call get_byear(person)
    if (byear) {
	set(byear_est,byear)
	set(byear_est_delta,byear_delta)
    }
    else { /* estimate from siblings */
	set(older,person)
	set(younger,person)
	set(yeardiff,0)
	set(border,0)
	set(this_uncertainty,1)
	while (and(not(byear_est),or(older,younger))) {
	    set(older,prevsib(older))
	    set(younger,nextsib(younger))
	    set(yeardiff,add(yeardiff,years_between_kids))
	    set(this_uncertainty,add(this_uncertainty,1))
	    if (older) {
		set(border,add(border,1))
		call get_byear(older)
		if (byear) {
		    set(byear_est,add(byear,yeardiff))
		    set(byear_est_delta,this_uncertainty)
		}
	    }
	    if (and(not(byear_est),younger))  {
		call get_byear(younger)
		if (byear) {
		    set(byear_est,sub(byear,yeardiff))
		    set(byear_est_delta,this_uncertainty)
		}
	    }
	}
    }
    if (not(byear_est)) { /* estimate from parents' marriage */
	if (m,marriage(parents(person))) { extractdate(m,bd,bm,my) }
	if (my) {
	    set(byear_est,add(add(my,mul(years_between_kids,border)),1))
	    set(byear_est_delta,add(border,1))
	}
    }
    if (not(byear_est)) { /* estimate from first marriage */
	families(person,fam,spouse,fnum) {
	    if (eq(fnum,1)) {
		if (m,marriage(fam)) { extractdate(m,bd,bm,my) }
		if (my) {
		    if (female(person)) { set(byear_est,sub(my,mother_age)) }
		    else { set(byear_est,sub(my,father_age)) }
		    set(byear_est_delta,5)
		}
		else {
		    children(fam,child,cnum) {
			if (not(byear_est)) {
			    call get_byear(child)
			    if (byear) {
				if (female(person)) {
				set(byear_est,sub(sub(byear,
					mul(sub(cnum,1),years_between_kids)),
					mother_age))
				}
				else {
				set(byear_est,sub(sub(byear,
					mul(sub(cnum,1),years_between_kids)),
					father_age))
				}
				set(byear_est_delta,add(5,cnum))
			    }
			}
		    }
		}
	    }
	}
    }
    if (not(byear_est)) { /* estimate from parents' birthyear */
	call get_byear(mother(person))
	if (byear) {
	    set(byear_est,add(byear,mother_age))
	}
	else {
	    call get_byear(father(person))
	    if (byear) {
		set(byear_est,add(byear,father_age))
	    }
	}
	if (byear) {
	    set(byear_est_delta,5)
	    set(older,person)
	    while(older,prevsib(older)) {
		set(byear_est,add(byear_est,years_between_kids))
		set(byear_est_delta,add(byear_est_delta,1))
	    }
	}
    }
}

proc get_byear(person) {
    set(byear,0)
    if (person) {
	if (b,birth(person)) { extractdate(b,day,month,byear) }
	if (byear) {
	    set(byear_delta,0)
	    set(dstring,trim(date(b),3))
	    if (not(strcmp(dstring,"BEF"))) { set(byear_delta,3) }
	    elsif (not(strcmp(dstring,"AFT"))) { set(byear_delta,3) }
	    elsif (not(strcmp(dstring,"ABT"))) { set(byear_delta,2) }
	}
	else {
	    if (b,baptism(person)) { extractdate(b,day,month,byear) }
	    if (byear) {
		set(byear_delta,1)
		set(dstring,trim(date(b),3))
		if (not(strcmp(dstring,"BEF"))) { set(byear_delta,3) }
		elsif (not(strcmp(dstring,"AFT"))) { set(byear_delta,3) }
		elsif (not(strcmp(dstring,"ABT"))) { set(byear_delta,2) }
	    }
	}
    }
}
