/*
 * @progname       gender_order.ll
 * @version        4
 * @author         Jim Eggert
 * @category       
 * @output         Text
 * @description    

This program computes gender order statistics for children in all the
families in a database.  Genders are considered to be ternary: male
(M), female (F), or unknown (U).  Children in a family form a pattern
of genders by birth order, e.g. MFFM for a family consisting of a boy,
two girls, then a boy.  The frequency of these patterns is calculated.
In addition to the complete gender pattern for a family, initial
gender patterns are computed.  For the above example, the initial
patterns are . (no children), M, MF, MFF, and MFFM.  The frequency of
these initial patterns can be used to answer questions such as how
many families with a boy then two girls go on to have another boy.

For example, suppose you want to know what fraction of families with a
child gender pattern P (e.g., P=MFFM) have no more children, have a
boy next (PM), have a girl next (PF), and have a child of unknown (to
the database!) gender next (PU).  You can find these fractions as
#complete(P)/#initial(P), #initial(PM)/#initial(P),
#initial(PF)/#initial(P), and #initial(PU)/#initial(P), respectively.
Note that these fractions should add up to 1.  Also note that the
pattern "." denotes no children at all.  As a initial pattern it gives
the total number of families in the database, as a complete pattern the
number of childless families in the database.

You can use either of two compare functions to sort the results
differently.  Rename the one you want to use as compare, the other one
something else (like compare1).

gender_order - a LifeLines gender order statistics program
    by Jim Eggert (EggertJ@crosswinds.net)
        Version 1,  5 August 1993
                listsort code by John Chandler (JCHBN@CUVMB.CC.COLUMBIA.EDU)
        Version 2, 10 August 1993
                added family examples, modified output format slightly
        Version 3, 26 March 1995
                changed listsort to quicksort
    Version 4, 15 Jan 2000
        quicksort bug fix

*/

/* This compare procedure sorts purely alphabetically */
func compare1(astring,bstring) {
    return(strcmp(astring,bstring))
}

/* This compare procedure sorts by length
   and alphabetically within groups of equal length */
func compare(astring,bstring) {
    set(alen,strlen(astring))
    set(blen,strlen(bstring))
    if (lt(alen,blen)) { return(neg(1)) }
    if (gt(alen,blen)) { return(1) }
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
  if (ge(len,2)) { call qsort(alist,ilist,1,len) }
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

proc main() {
    list(patterns)
    list(initial_counts)
    list(complete_counts)
    table(indices)
    list(sorted_indices)
    list(complete_examples)

/* accumulate gender order statistics, even unknown genders */

    enqueue(initial_counts,0)
    enqueue(complete_counts,0)
    enqueue(patterns,".")
    enqueue(complete_examples,"no example")
    insert(indices,".",1)
    set(max_index,1)
    set(max_nc,0)

    set(nextfam,0)
    print("Processing families ")
    forfam(family,fnum) {
        setel(initial_counts,1,fnum)
        set(pattern,"")
        set(index,1)
        if (nc,nchildren(family)) {
            if (gt(nc,max_nc)) { set(max_nc,nc) }
            children(family,child,cnum) {
                if (not(strcmp(sex(child),"F"))) {
                    set(pattern,save(concat(pattern,"F")))
                }
                elsif (not(strcmp(sex(child),"M"))) {
                    set(pattern,save(concat(pattern,"M")))
                }
                else {
                    set(pattern,save(concat(pattern,"U")))
                }
                set(index,lookup(indices,pattern))
                if (index) {
                    setel(initial_counts,index,
                          add(getel(initial_counts,index),1))
                }
                else {
                    set(max_index,add(max_index,1))
                    set(index,max_index)
                    insert(indices,pattern,index)
                    enqueue(patterns,save(pattern))
                    enqueue(initial_counts,1)
                    enqueue(complete_counts,0)
                }
            }
        }
        else {
            if(not(strcmp(getel(complete_examples,1),"no example"))) {
                setel(complete_examples,1,save(key(family)))
            }
        }
        if (not(getel(complete_examples,index))) {
            setel(complete_examples,index,save(key(family)))
        }
        setel(complete_counts,index,add(getel(complete_counts,index),1))
        if (ge(fnum,nextfam)) {
            print(d(fnum)) print(" ")
            set(nextfam,add(nextfam,100))
        }
    }

    print("\nSorting results...")

    call quicksort(patterns,sorted_indices)

/* print out gender order statistics sorted alphabetically */

    print("done\nPrinting results...")

    set(initialcol,add(max_nc,16))
    set(completecol,add(initialcol,12))
    set(examplecol,add(completecol,12))
    "Gender pattern"  col(sub(initialcol,7)) "initial"
    col(sub(completecol,8)) "complete" col(examplecol) "example\n"

    forlist(sorted_indices,index,inum) {
        getel(patterns,index)
        set(initial,getel(initial_counts,index))
        col(sub(initialcol,strlen(d(initial)))) d(initial)
        set(complete,getel(complete_counts,index))
        col(sub(completecol,strlen(d(complete)))) d(complete)
        col(examplecol) getel(complete_examples,index) "\n"
    }
    print("done")
}
