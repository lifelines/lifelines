/*
 * @progname       namefreq
 * @version        2.0
 * @author         Chandler
 * @category       
 * @output         Text
 * @description    

namefreq

Tabulate frequency of first names in database.

Version 1 - 1993 Jun 16 - John F. Chandler
Version 2 - 1993 Jun 18 (sort output by frequency)

This report counts occurrences of all first (given) names in the
database.  Individuals with only surnames are not counted.  If the
surname is listed first, the next word is taken as the given name.

The output file is normally sorted in order of decreasing frequency,
but the sort order can be altered by changing proc "compare", e.g.,
comment out the existing "set" and uncomment the one for alphabetical
order.

This program works only with LifeLines.

*/
global(comparison)      /* for sorting */
global(name_counts)     /* used by comparison in sorting by frequency */

proc compare(astring,bstring) {
/* alphabetical:
        set(comparison,strcmp(astring,bstring)) */
/* decreasing frequency: */
        set(comparison,sub(lookup(name_counts,bstring),lookup(name_counts,astring)))
}

proc listsort(alist,ilist) {
/*
   Input:  alist  - list of strings (could use numbers, instead)
   Output: ilist  - list of index pointers into "alist" in sorted order
   Needed: compare- external "function" of two arguments to set global
                    variable "comparison" to -1,0,+1 according to relative
                    order of the two arguments

   Uses an internal list as a work area.  This is significantly faster
   than a bubble sort.

*/
        list(wlist)

        set(list_size,length(alist))
        set(completion,0)
        set(index,0)
        while (lt(index,list_size)) {
                set(index,add(index,1))
                setel(ilist,index,index)
        }
        while (lt(completion,list_size)) {
                set(count,0)
                while (lt(count,list_size)) {
                        set(count,add(count,1))
                        set(index,dequeue(ilist))
                        set(test,getel(alist,index))
                        if(eq(count,1)) {
                                set(work_min,test)
                                set(work_max,test)
                        }
                        call compare(test,work_max)
                        if(ge(comparison,0)) {
                                enqueue(wlist,index)
                                set(work_max,test)
                        } else { call compare(test,work_min)
                                while(gt(comparison,0)) {
                                        enqueue(ilist,dequeue(wlist))
                                        set(work_min,getel(alist,getel(wlist,1)))
                                        call compare(test,work_min)
                                }
                                requeue(wlist,index)
                                set(work_min,test)
                        }
                }
                set(completion,length(wlist))
                while (not(empty(wlist))) { enqueue(ilist,dequeue(wlist)) }
        }
}

proc main ()
{
        list(namelist)
        table(name_counts)
        list(names)
        list(ilist)

        forindi (indi, num) {
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

        call listsort(names,ilist)
        forlist(ilist, index, num) {
                set(fname,getel(names,index))
                fname
                set(nmatch, lookup(name_counts, fname))
                col(sub(25, strlen(d(nmatch))))
                d(nmatch) "\n"
        }
}
