/*
 * @progname       related_spouses.ll
 * @version        2.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program identifies spouses with known common ancestors.  For each
marriage of related spouses, the spouses' names are printed, along
with the first common ancestor in each branch of the ancestry tree,
and the number of intervening generations for the husband and wife,
respectively.

related_spouses - a LifeLines program to identify related spouses
	by Jim Eggert (eggertj@atc.ll.mit.edu)
	Version 1,  31 March 1993 (first release)
	Version 2,  15 March 1995 (use new set functions, generation numbers)

*/

proc main() {
    indiset(husb_ancestors)
    indiset(wife_ancestors)
    indiset(common_ancestors)
    forfam(family,fnum) {
	if (hubby,husband(family)) {
	    if (wifey,wife(family)) {
/* find common ancestors */
		indiset(oneset)
		addtoset(oneset,hubby,0)
		set(husb_ancestors,ancestorset(oneset))
		addtoset(husb_ancestors,hubby,0)
		indiset(oneset)
		addtoset(oneset,wifey,0)
		set(wife_ancestors,ancestorset(oneset))
		addtoset(wife_ancestors,wifey,0)
		set(common_ancestors,intersect(husb_ancestors,wife_ancestors))
		set(cnum,lengthset(common_ancestors))

/* find lowest common ancestors (common_ancestors - too_common_ancestors) */
		if (cnum) {
/* Make wife ancestor generation table wcat */
		    indiset(wca)
		    set(wca,intersect(wife_ancestors,husb_ancestors))
		    table(wcat)
		    forindiset(wca,person,wgen,wnum) {
			insert(wcat,key(person),wgen)
		    }

		    indiset(lowest_common_ancestors)
		    set(lowest_common_ancestors,
			difference(common_ancestors,
				   ancestorset(common_ancestors)))
		    set(lca_length,lengthset(lowest_common_ancestors))
/* print out lowest common ancestors */
		    key(family) " "
		    key(husband(family)) " " name(husband(family))
		    " and "
		    key(wife(family)) " " name(wife(family))
		    "\n have " d(lca_length)
		    " lowest common ancestor"
		    if (gt(lca_length,1)) { "s" }
		    col(60) "hgen" col(70) "wgen\n"
		    forindiset(lowest_common_ancestors,lca,hgen,lnum) {
			"  " key(lca) " " name(lca)
			col(60) d(hgen)
			col(70) d(lookup(wcat,key(lca))) "\n"
		    }
		}
	    }
	}
    }
}
