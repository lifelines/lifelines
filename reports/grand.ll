/*
 * @progname       grand
 * @version        1.1
 * @author         Stephen Dum
 * @category
 * @output         text
 * @description

For a selected individual this program outputs a list of children,
grand children, great grand children and great great grand children.

Output format is simple text, roughly 80 columns. Each list is sorted
by date person 'entered' the family either by birth date or adoption.

Note - if multiple children have the same birth date, they are all given
the same rank. Thus numbering can appear to repeat.  I.E. you might see
1., 2., 2., 4. ... if the second and third child were born on the same date.

grand - a LifeLines database program
    by Stephen Dum  stephen.dum@verizon.net
    Version 1,  15 December 2002
    Version 1.1,  3 June 2007  - minor update
*/

global(gkdates)           /* list of numeric versions of dates for sorting */
global(refind)            /* list used to hold indexes of dates sorted*/
global(adopt_event)
proc main()
{
    dayformat(0)          /* leave spaces in single digit days */
    monthformat(4)        /* print month as Jan... */
    dateformat(0)         /* use 'da mon year' order */

    list(gkdates)
    list(refind)

    /* for finding children */
    list(par)             /* list of selected individual */
    list(kids)            /* children of selected individual */
    list(kids_par)        /* and their parent -- unused and all same but
                             needed for compatibility with later calls */
    list(kids_adop)       /* list of adoption flag */

    /* for finding grand children. at a given index is person in gkids
     * and at the same index in gkids_par is the parent */
    list(gkids)           /* list of grand children */
    list(gkids_par)       /* index into kids array to parent */
    list(gkids_adop)      /* adoption flag */

    /* for finding great grand children */
    list(ggkids)          /* list of great grand children */
    list(ggkids_par)      /* index into gkids array to the parent */
    list(ggkids_adop)     /* adoption flag */

    /* for finding great great grand children */
    list(gggkids)         /* list of great great grand children */
    list(gggkids_par)     /* index into ggkids array so we can get parent */
    list(gggkids_adop)    /* adoption flag */

    /* for finding great great great grand children */
    list(ggggkids)         /* list of great great great grand children */
    list(ggggkids_par)     /* index into gggkids array so we can get parent */
    list(ggggkids_adop)    /* adoption flag */

    /* select individual for report */
    set(indi0, NULL)
    set(count,5)
    while (not(indi0)) {
       getindi(indi0,"Enter person to find grand children for:")
        if (not(indi0)) {
            print("Individual not found in database.",nl())
	    decr(count)
	    if (not(count)) {
	       print("aborting", nl())
	       return(0)
	    }
        }
    }
    enqueue(par,indi0)

    /* put out header */
    call print_header(indi0)

    /* compute children of selected individual */
    call compute_child(par,kids,kids_par,kids_adop)

    /* and print children */
    if (not(length(kids))) {
        print("No children!",nl())
	return(0)
    }
    /* for children only 1st 3 params and last are used */
    call print_kids(kids, kids_par, kids_adop, kids, kids_par,
		    kids,  kids_par,  kids, 0)

    /* compute grand children */
    call compute_child(kids,gkids,gkids_par,gkids_adop)
    if(length(gkids)) {
	call print_kids(gkids, gkids_par, gkids_adop, kids, kids_par,
			kids,  kids_par,  kids, 1)

	/* compute great grand children */
	call compute_child(gkids,ggkids,ggkids_par,ggkids_adop)
	if(length(ggkids)) {
	    /* print grand children */
	    call print_kids(ggkids, ggkids_par, ggkids_adop, gkids, gkids_par,
	                    kids, kids_par, kids, 2)

	    /* compute great great grand children */
	    call compute_child(ggkids,gggkids,gggkids_par,gggkids_adop)
	    if (length(gggkids)) {
		call print_kids(gggkids, gggkids_par, gggkids_adop,
		                ggkids, ggkids_par,
		                gkids,   gkids_par, kids, 3)

		/* compute great great great grand children */
		call compute_child(gggkids,ggggkids,ggggkids_par,ggggkids_adop)
		if (length(ggggkids)) {
		    call print_kids(ggggkids, ggggkids_par, ggggkids_adop,
		                    gggkids, gggkids_par,
				    ggkids,   ggkids_par, gkids, 4)
		}
	    }
	}
    }
}

/* compute children from list of parents
 * p   - list of parents
 * c   - children being computed
 * c_p - indexes into p corresponding to each child
 * a   - list of adoption dates (or null) for each child
 *
 * p is passed in, c,c_p and a are outputs and assumed to be zero lenght
 *             at call
 * gkdates - dates to sort by
 */
proc compute_child(p,c,c_p,a) {
    /* clear out the gkdates list - easy it's global*/
    list(gkdates)

    forlist(p, e, i) {
	families(e, f, indi, j) {
	    children(f, nextchild, k) {
		/* if child is already in the list, we want the
		 * one with the earliest date only
		 * This is rare, we could set adopt to -1 and use
		 * as a flag to print twice, but not count twice
		 */
		if (birth(nextchild)) {
		    set(sortdate,get_date(birth(nextchild)))
		} else {
		    set(sortdate,0)
		}
		if (x,isadopted(nextchild,f,e)) {
		    /*
		     * if adopted, put adopt_date into gkdates
		     * This makes listing
		     * include adopted as date joined family
		    */
		    if (date(adopt_event)) {
			set(sortdate, get_date(adopt_event))
		    }
		}
		if (dupind,finddup(c,nextchild)) {
		    /* found duplicate
		     * if new sortdate is smaller than previous, use it
		     */
		    if (lt(sortdate,getel(gkdates,dupind))) {
		        setel(c_p,dupind,i)
			setel(a,dupind,x)
			setel(gkdates,dupind,sortdate)
		    }
		} else {
		    enqueue(gkdates, sortdate)
		    enqueue(c, nextchild)
		    enqueue(c_p,i)
		    enqueue(a,x)
		}
	    }
	}
    }
}

/* finddup(clist,ind_child)
 * see if ind_child is already in the list, if so, return
 * the index for the child - we could use inlist() but
 * it doesn't give us the index of the match
 */
func finddup(clist,ind_child) {
  forlist(clist,e, i) {
      if (eq(e,ind_child)) { return(i) }
  }
  return(0)
}

/* print_kids - for lower levels not all arrays are used
 * k1    - list of children being printed
 * g1    - index into k2 for parents of children
 * a     - list of adopted flags
 * k2    - list of parents of children
 * g2    - index into k3 for grand parents
 * k3    - list of grandparents
 * g3    - index into k4 for great grandparents
 * k4    - list of great grandparents
 * index - how many levels to print
 */
proc print_kids(k1, g1, a, k2, g2, k3, g3, k4, level) {
    list(refind)
    set(adopted, 0)     /* count number of adopted children */
    /* refind is used to get to names corresponding to elements of
     * the gkdates list after sorting
     */
    set(len,length(gkdates))
    set(index,len)
    while(index) {
	setel(refind,index,index)
	decr(index)
    }
    sort(refind,gkdates)

    set(dups,0)

    /* print out the title for the section */
    set(title,start_section(level))

    /* Iterate over values in refind and print out the data
     * lasti - last printed rank for individual
     * lastd - birth date of previous entry for same date check
     * count - child rank
     *
     * index - child rank to print for this individual
     */
    set(lasti,1)
    set(lastd,getel(gkdates,1))
    set(count,0)
    forlist(refind, ind, i) {
	set(cur_per, getel(k1, ind))
	set(cur_per_par_ind, getel(g1, ind))

	/* list all children with same birth date as same number
	 * also, second marriages and adoptions may cause child to be
	 * listed twice, it's easiest to remove here, since data is sorted
	 * by birthdate.
	 */
	incr(count)
	if (ne(lastd,getel(gkdates,i))) {
	    /* dates are different */
	    set(index,count)
	    set(lasti,count)
	    set(lastd,getel(gkdates,i))
	} else {
	    /* date same, keep using same index value */
	    set(index,lasti)
	}
	set(adopt,getel(a,ind))
        /* uncomment next 3 lines if you want adopted children to listed,
         * but not counted
	if (adopt) {
	    "--"
	    decr(count)
	}
        */
        d(index)
	/* print first line */
	"." col(5) name(cur_per,false)
	col(36) date(birth(cur_per))
	if (eq(level, 0)) {
	    if(adopt) {
		incr(adopted)
		col(49) "Adopt:" adopt 
	    }
	    nl()
	} else {
	    col(49) name(getel(k2,cur_per_par_ind),false) nl()
	    if(adopt) {
		incr(adopted)
		col(5) "Adopt:" adopt 
	    }
	    nl()
	}
	if (ne(date(death(cur_per)), 0)) {
	    col(5) "died: " date(death(cur_per))
	    if (lt(level,2)) {
		nl()
	    }
	}
	if (gt(level, 1)) {
	    set(gpar,getel(g2,cur_per_par_ind))
	    col(23) name(getel(k3,gpar),false)
	}
	if (gt(level,2)) {
	    col(49) name(getel(k4,getel(g3,gpar)),false)
	}
	if (gt(level,1)) {
		nl()
	}
    }

    /* print section summary */
    set(count,sub(length(k1),dups))
    nl()
    d(count)  " " title
    if (adopted) { " (" d(adopted) " adopted)" }
    nl()
    print(d(count), " ", title)
    if (adopted) { print(" (",d(adopted)," adopted)") }
    print(nl())
}

proc print_header(parent) {
    col(30) "Children of" nl()
    col(30) name(parent,false)  nl()
    families(parent, f, ind, i) {
	col(30) "Spouse: " name(ind, false) nl()
    }
}
func start_section(level) {
    nl()
    if (eq(level,0)) {
	set(title,"Children")
	print_ref(title)
	nl()
	col(5) "Name" col(36) "Birth" nl()
    } elsif (eq(level,1)) {
	set(title,"Grand Children")
	print_ref(title)
	nl()
	col(5) "Name" col(36) "Birth" col(49) "Parent" nl()
    } elsif (eq(level,2)) {
	set(title,"Great Grand Children")
	print_ref(title)
	nl()
	col(5) "Name" col(36) "Birth" col(49) "Parent" nl()
	col(23) "Grand Parent" nl()
    } elsif (eq(level,3)) {
	set(title,"Great Great Grand Children")
	print_ref(title)
	nl()
	col(5) "Name" col(36) "Birth" col(49) "Parent" nl()
	col(23) "Grand Parent" col(49) "Great Grand Parent" nl()
    } else {
	set(title,"Great Great Great Grand Children")
	print_ref(title)
	nl()
	col(5) "Name" col(36) "Birth" col(49) "Parent" nl()
	col(23) "Grand Parent" col(49) "Great Grand Parent" nl()
    }
    return(title)
}

func print_ref(title) {
    set(l,div(sub(80,strlen(title)),2))
    col(l) title  nl()
    set(name, concat(" (Compiled by ",getproperty("user.fullname")," ",
            stddate(gettoday()),")",nl()))
    set(l, add(26,strlen(name)))
    set(l,div(sub(80,strlen(name)),2))
    col(l) name
}

/* check to see if person in family fam is adopted by par */
/* returns 0 if not adopted */
func isadopted(per,fam, par) {
    /*
     * in gedcom 5.5 the structure is
     * 1 INDI
     *   2 ADOP
     *     3 FAMC
     * 	     4 ADOP (BOTH|HUSB|WIFE)
     */
    set(x,xref(fnode(fam)))
    fornodes(inode(per),e) {
	if(eqstr(tag(e),"ADOP")) {
	    /*
	    print("adopt record for ",name(per),nl())
	    */
	    fornodes(e,fam) {
	    /* check for 'FAMC' with value x */
	    if(and(eqstr(tag(fam),"FAMC"),eqstr(value(fam),x))) {
		/* now see if famc has a adop record */
	        /*
	        print("... match FAMC ",x,nl())
	        */
	        fornodes(fam,a) {
	            if(eqstr(tag(a),"ADOP")) {
			/*
			print("...   ADOP ",value(a)," par=",name(par))
			if (male(par)) { print(" m") }
			if (female(par)) { print (" f") }
			print(nl())
			*/
			set(adopt_event,e)
		        if(eqstr(value(a),"HUSB")) {
		          if(male(par)) {
			      if(da,date(e)) { return(da) }
			      return("-")
			  }
		        } elsif(eqstr(value(a),"WIFE")) {
		          if(female(par)) {
			      if(da,date(e)) { return(da) }
			      return("-")
			  }
		        } else {
			    /*
			    if(eqstr(value(a),"BOTH")) { .... }
			    must be "BOTH" (note this has side effect that ""
			    is treated as both
			    */
			    if(da,date(e)) { return(da) }
			    else { return("-")}
			}
		  }
	      }
	   }
	   }
	}
    }
    return(0)
}

/* hack together a integer that can be sorted to represent the date */
func get_date(datenode)
{
    extractdate(datenode,day,month,year)
    return(add(mul(add(mul(year,100), month),100),day))
}
