/*
 * @progname       namefreq.ll
 * @version        3.0
 * @author         Chandler
 * @category       
 * @output         Text
 * @description    

This report counts occurrences of all first (given) names in the
database.  Individuals with only surnames are not counted.  If the
surname is listed first, the next word is taken as the given name.

namefreq

Tabulate frequency of first names in database.

Version 1 - 1993 Jun 16 - John F. Chandler
Version 2 - 1993 Jun 18 (sort output by frequency)
Version 3 - 1995 Mar 8  (requires LL 3.0 or higher)
			(Uses Jim Eggert's Quicksort routine)

The output file is normally sorted in order of decreasing frequency,
but the sort order can be altered by changing func "compare", e.g.,
comment out the existing "set" and uncomment the one for alphabetical
order.

This program works only with LifeLines.

*/
global(name_counts)	/* used by comparison in sorting by frequency */

/* Comparison function for sorting.  Same convention as strcmp. */
func compare(astring,bstring) {
/* alphabetical:
	return(strcmp(astring,bstring)) */
/* decreasing frequency: */
	if(ret,sub(lookup(name_counts,bstring),lookup(name_counts,astring))){
		return(ret)
	}
	return(strcmp(astring,bstring))
}

/*
   quicksort: Sort an input list by generating a permuted index list
   Input:  alist  - list to be sorted
   Output: ilist  - list of index pointers into "alist" in sorted order
   Needed: compare- external function of two arguments to return -1,0,+1
		    according to relative order of the two arguments
*/
proc quicksort(alist,ilist) {
    set(len,length(alist))
    set(index,len)
    while(index) {
	setel(ilist,index,index)
	decr(index)
    }
    call qsort(alist,ilist,1,len)
}

/* recursive core of quicksort */
proc qsort(alist,ilist,left,right) {
    if(pcur,getpivot(alist,ilist,left,right)) {
	set(pivot,getel(alist,getel(ilist,pcur)))
	set(mid,partition(alist,ilist,left,right,pivot))
	call qsort(alist,ilist,left,sub(mid,1))
	call qsort(alist,ilist,mid,right)
    }
}

/* partition around pivot */
func partition(alist,ilist,left,right,pivot) {
    while(1) {
	set(tmp,getel(ilist,left))
	setel(ilist,left,getel(ilist,right))
	setel(ilist,right,tmp)
	while(lt(compare(getel(alist,getel(ilist,left)),pivot),0)) {
	    incr(left)
	}
	while(ge(compare(getel(alist,getel(ilist,right)),pivot),0)) {
	    decr(right)
	}
	if(gt(left,right)) { break() }
    }
    return(left)
}

/* choose pivot */
func getpivot(alist,ilist,left,right) {
    set(pivot,getel(alist,getel(ilist,left)))
    set(left0,left)
    incr(left)
    while(le(left,right)) {
	set(rel,compare(getel(alist,getel(ilist,left)),pivot))
	if (gt(rel,0)) { return(left) }
	if (lt(rel,0)) { return(left0) }
	incr(left)
    }
    return(0)
}

proc main ()
{
	list(namelist)
	table(name_counts)
	list(names)
	list(ilist)

	forindi (indi, num) {
		if(not(mod(num,20))) {print(".")}
		extractnames(inode(indi), namelist, ncomp, sindx)
		set(gindx,1) if(eq(sindx,1)) { set(gindx,2) }
		set(fname, save(getel(namelist, gindx)))
		if( or( gt(sindx,1), gt(ncomp,sindx))) {
			if(nmatch, lookup(name_counts, fname)) {
				set(nmatch, add(nmatch, 1))
			}
			else {
				enqueue(names, fname)
				set(nmatch, 1)
			}
			insert(name_counts, fname, nmatch)
		}
	}
	"Frequency of given names (first only) in the database\n\n"
	"Name              Occurrences\n\n"

	call quicksort(names,ilist)
	forlist(ilist, index, num) {
		set(fname,getel(names,index))
		fname
		set(nmatch, lookup(name_counts, fname))
		col(sub(25, strlen(d(nmatch))))
		d(nmatch) "\n"
	}
}
