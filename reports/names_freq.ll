/*
 * @progname       names_freq.ll
 * @version        3.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

Tabulate frequency of names in database.  Like namefreq (by John
Chandler), but it computes frequencies for the first five given names,
the surname, and the first two post-surnames.

The output can be sorted by frequency or by alphabet, or not at all.

names_freq - a LifeLines names frequency calculation program
        by Jim Eggert (EggertJ@crosswinds.net)
        Version 1,   8 November 1993 (initial release)
                listsort code by John Chandler (JCHBN@CUVMB.CC.COLUMBIA.EDU)
        Version 2,  10 April 1995 changed listsort to quicksort
        Version 3,  15 January 2000 quicksort bug fix
*/

global(indices)         /* table for indexing into the various lists */
global(top_index)       /* number of elements in table and lists */
global(sort_type)       /* 0=none, 1=frequency, 2=alphabet */
global(names)           /* list of all names */
global(givens1)         /* list of counts of names in each position */
global(givens2)
global(givens3)
global(givens4)
global(givens5)
global(surs)
global(posts1)
global(posts2)
global(totals)

func compare(a,b) {
  if (eq(sort_type,1)) {
/* decreasing frequency: */
    if (lt(a,b)) { return(1) }
    if (eq(a,b)) { return(0) }
    return(neg(1))
  }
  else {
    return(strcmp(a,b))
  }
}

/*
   quicksort: Sort an input list by generating a permuted index list
   Input:  alist  - list to be sorted
   Output: ilist  - list of index pointers into "alist" in sorted order
   Needed: compare- external function of two arguments to return -1,0,+1
                    according to relative order of the two arguments
*/
proc quicksort(alist,ilist) {
  set(index,1)
  set(len,length(alist))
  while(le(index,len)) {
    setel(ilist,index,index)
    incr(index)
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
    set(next,getel(alist,getel(ilist,left)))
    set(rel,compare(next,pivot))
    if (gt(rel,0)) { return(left) }
    if (lt(rel,0)) { return(left0) }
    incr(left)
  }
  return(0)
}


proc count_name(name,thiscount) {
  if(index, lookup(indices, name)) {
    setel(thiscount,index,add(getel(thiscount,index),1))
  } else {
/*        print("(") print(name) print(")") */
    incr(top_index)
    set(sname,save(name))
    enqueue(names,sname)
    insert(indices,sname,top_index)
    enqueue(givens1,0)
    enqueue(givens2,0)
    enqueue(givens3,0)
    enqueue(givens4,0)
    enqueue(givens5,0)
    enqueue(surs,0)
    enqueue(posts1,0)
    enqueue(posts2,0)
    enqueue(totals,0)
    setel(thiscount,top_index,add(getel(thiscount,top_index),1))
  }
}

proc main ()
{
  table(indices)
  list(namelist)
  list(names)
  list(givens1)
  list(givens2)
  list(givens3)
  list(givens4)
  list(givens5)
  list(surs)
  list(posts1)
  list(posts2)
  list(totals)
  list(ilist)

  set(top_index,0)
  set(next_num,0)

  print("Counting names...")
  forindi (indi, num) {
    extractnames(inode(indi), namelist, ncomp, sindx)
    forlist(namelist,name,ni) {
      call count_name(name,totals)
    }
    if (and(ge(ncomp,1),or(eq(sindx,0),gt(sindx,1)))) {
      call count_name(getel(namelist,1),givens1)
    }
    if (and(ge(ncomp,2),or(eq(sindx,0),gt(sindx,2)))) {
      call count_name(getel(namelist,2),givens2)
    }
    if (and(ge(ncomp,3),or(eq(sindx,0),gt(sindx,3)))) {
      call count_name(getel(namelist,3),givens3)
    }
    if (and(ge(ncomp,4),or(eq(sindx,0),gt(sindx,4)))) {
      call count_name(getel(namelist,4),givens4)
    }
    if (and(ge(ncomp,5),or(eq(sindx,0),gt(sindx,5)))) {
      call count_name(getel(namelist,5),givens5)
    }
    if (sindx) {
      call count_name(getel(namelist,sindx),surs)
    }
    if (gt(ncomp,sindx)) {
      call count_name(getel(namelist,add(sindx,1)),posts1)
    }
    if (gt(ncomp,add(sindx,1))) {
      call count_name(getel(namelist,add(sindx,2)),posts2)
    }
    if (ge(num,next_num)) {
      print(d(num)) print(" ")
      set(next_num,add(next_num,100))
    }
  }
  print(d(num))

  getintmsg(sort_type,"Sort method (0=no sort, 1=frequency, 2=alphabet)")
  if (sort_type) {
    print("\nSorting ") print(d(top_index)) print(" names...")
  }
  if (eq(sort_type,1)) {
    call quicksort(totals,ilist)
  }
  elsif (eq(sort_type,2)) {
    call quicksort(names,ilist)
  }
  else {
    forlist(names,name,index) { enqueue(ilist,index) }
  }

  print("\nWriting results...")

  "______Frequency of names in the database______\n\n"
"Name                      1st   2nd   3rd   4th   5th   sur post1 post2 total"
"\n\n"
  forlist(ilist, index, num) {
    getel(names,index)

    set(nmatch, getel(givens1,index))
    col(sub(30, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(givens2,index))
    col(sub(36, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(givens3,index))
    col(sub(42, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(givens4,index))
    col(sub(48, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(givens5,index))
    col(sub(54, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(surs,index))
    col(sub(60, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(posts1,index))
    col(sub(66, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(posts2,index))
    col(sub(72, strlen(d(nmatch))))
    d(nmatch)
    set(nmatch, getel(totals,index))
    col(sub(78, strlen(d(nmatch))))
    d(nmatch) "\n"
  }
}
