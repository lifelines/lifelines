/*
 * @progname       stats.ll
 * @version        10.0
 * @author         Jim Eggert
 * @category       
 * @output         Text
 * @description    Compute statistics on dates, ages, counts in the DB.

This LifeLines report program computes mean statistics of various
quantities binned over other quantities.  The quantities it knows
about are ages at and dates of birth, christening, first and last
marriage, first and last child's birth, death, burial, and today; the
number of children, siblings, and marriages; and sex, surname, first
name, soundex, and any simple GEDCOM tag.  These can be combined
nearly arbitrarily and evaluated over the whole database, or
restricted to ancestors or descendants of a chosen individual or to
members of a predetermined set.  Further restrictions on the
individuals included in the statistics can be based on any quantity
that the program knows about.  The program will optionally print out
the names of all the individuals included in the statistics.

For example, you can produce statistics of
    the age at death of as a function of birth year,
    dage vs byear
    the number of children of females named Smith as a
    function of year of first marriage,
    kids vs myear | sex = F & surname = Smith
    the number of spouses for male vs female blacksmiths,
    families vs sex | occu = blacksmith
    the age at last childbirth as a function of place of marriage.
    qage vs mplace
    the names of all Joneses who lived to be greater than 80
    unity vs unity | surname = Jones & dage > 80
All this without writing any programs of your own.

If a particular statistic for an individual is unavailable, and if the
global variable not_strict is nonzero (as it is in the distribution
version of this report, then certain guesses are allowed as to the
value of that statistic.  So far, these guesses are few.  Birth year
and month are guessed from baptismal date, and death year and month
are guessed from burial date.

The user is prompted for what quantity to plot vs what to bin over.
Each is to be given as a specification string of the form
    e<time> or e<place> or <count> or <label>
   where e is the event code, <time> and <place> are place codes,
   <count> is the count codes, and <label> is the label code.
event codes
    b birth (can get year or month from christening)
    c christening (can get year or month from birth)
    d death (can get year or month from burial)
    e burial (entombment) (can get year or month from death)
    m marriage (first)
    n marriage (last)
    p parturition or childbirth (first)
    q parturition (last)
    t today (most useful with age)
time codes
    age (in years)
    fage (father's age)
    mage (mother's age)
    year
    month
    day
place codes
    place (tplace is always the empty string)
count codes
    unity (always returns one, good for simple histograms or averages)
    kids
    families
    siblings
    siblingorder
label codes
    sex
    surname
    firstname
    soundex
    <tag> (returns the value of the first GEDCOM node with this tag)
          (a useful example is OCCU for occupation)

For example, to compute statistics of age at death vs year of first
marriage, enter dage then myear then the bin size for myear.
To compute statistics of the number of children versus occupation,
enter kids then occu.

The statistics can also be restricted according to any of the known
characteristics.  The user is prompted for any constraints, to be entered
one at a time.  For example, if you want to accumulate statistics only
if the person is female, enter the constraint
    sex = F
or if you want to allow only people born after 1800
    byear > 1800

The allowed equality symbols are =, !=, <, >, <=, and >=.  For
strings, two additional symbols are allowed, contains and isin.
Multiple constraints are allowed, just enter return if you don't want
any more.  Please note that the constraints are composed of three
parts: left hand side, inequality symbol, and right hand side.  You
must use exactly one space between each pair of parts.

Case is not important.  You can't compute averages of label codes,
as that would not be meaningful.

stats - a LifeLines database statistical extraction report program
    by Jim Eggert (EggertJ@crosswinds.net)
    Version 1 (14 Dec 1992)
    Version 2 (17 Dec 1992) added restrictions,
        unity, general GEDCOM tag, e<place>, and today
    Version 3 (20 Dec 1992) added sorting
    Version 4 (30 Jan 1993) bugfix, modified find_bin to use requeue()
        Requires LifeLines v2.3.3 or later
    Version 5 (30 Jun 1993) changed bubblesort to listsort
        listsort code by John Chandler (JCHBN@CUVMB.CC.COLUMBIA.EDU)
    Version 6 (2 July 1993) added firstname, changed user interface
    Version 7 (16 Mar 1995) changed listsort to quicksort,
                modernized, fixed kids bug, added fage and mage
        Requires LifeLines v3.0.1 or later
    Version 8 (4 Sep 1995) added min and max, no printing of bogus 0s
    Version 9 (29 Sep 1995) fixed minor bugs, added indiset, substrings
    Version 10 (15 Jan 2000) now uses eqstr, fixed quicksort bug

*/

global(ret_type)  /* return type 0=none, 1=string, 2=integer */
global(minbin)    /* maintained by find_bin */
global(maxbin)    /* ditto */
global(deltabin)  /* zero for binning over non-numeric types */
global(not_strict)/* 0=strict dates, 1=estimate dates */
global(accuracy)  /* 10^digits after the decimal */
global(log)       /* 0=don't, 1=do write an individual log */
global(rlist)     /* list of restriction variable names */
global(tlist)     /* list of restriction variable types */
global(clist)     /* list of restriction inequality codes */
global(vlist)     /* list of restriction values */
global(vtable)    /* table of known return value types */
global(xvaltable) /* table of known bin strings */

proc main() {
  call init_vtable() /* set up the table of return value types */
  set(not_strict,1)
  set(accuracy,100)  /* compute and print to 0.01 */
  list(ysums)   /* bin sums */
  list(ycounts) /* bin counts */
  list(ymins)   /* bin mins */
  list(ymaxs)   /* bin maxes */
  list(xvals)   /* bin values */
  list(relist)  /* contains restriction equations */
  list(rlist)   /* contains restriction LHSs */
  list(tlist)   /* contains restriction LHS types */
  list(clist)   /* contains restriction inequalities */
  list(vlist)   /* contains restriction RHSs */
  list(ilist)   /* index list for sorting */
  list(restriction_tokens) /* contains restriction equation elements */
  table(xvaltable) /* table of bin labels */
  indiset(people)
  set(sepline,concat("-------------------------------------",
                     "-------------------------------------\n"))

  while (1) {
    getstrmsg(ystring,"Collect statistics of ")
    set(ystring,save(upper(ystring)))
    if (ne(value_type(ystring),1)) { break() }
    print("Can't do statistics on ") print(ystring) print("\n")
  }
  getstrmsg(xstring,"versus ")
  set(xstring,save(upper(xstring)))
  set(deltabin,0)
  if (ne(value_type(xstring),1)) {
    while (1) {
      getintmsg(deltabin,"with bin width ")
      if (deltabin) { break() }
      print("Can't have zero bin width\n")
    }
    set(minbin,1000000)
    set(maxbin,neg(minbin))
  }
  getintmsg(log,"logging people (0=no, 1=yes) ")
  getintmsg(who,
      "over set (0=all, 1=descendants, 2=ancestors, 3=indiset) ")
  if (or(eq(who,1),eq(who,2))) {
    getindimsg(of,"of ")
    set(ofkey,save(key(of)))
  }
  elsif (eq(who,3)) { getindiset(people) }
  while (1) {
    getstrmsg(restriction_equation,"restricted by ")
    if (eqstr(restriction_equation,"")) { break() }
    while(not(empty(restriction_tokens)))
      { set(junk,dequeue(restriction_tokens)) }
    extracttokens(restriction_equation,restriction_tokens,nt," ")
    if (nt) {
      set(restriction,save(upper(dequeue(restriction_tokens))))
      set(inequality,save(dequeue(restriction_tokens)))
      if (inequality) {
        set(type,value_type(restriction))
        if (eq(type,1)) {
          set(value,save(dequeue(restriction_tokens)))
        }
        else {
          set(value,atoi(dequeue(restriction_tokens)))
        }
        enqueue(rlist,restriction)
        enqueue(tlist,type)
        enqueue(clist,inequality)
        enqueue(vlist,value)
        enqueue(relist,save(restriction_equation))
      }
    }
  }
  if (log) {
    sepline
    "Log of individuals and their values used in the statistics\n"
    sepline
    "key" col(8) "name" col(50) xstring col(65) ystring "\n"
    sepline
  }

  if (eq(who,0)) {
    forindi(person,pnum) {
      call stat_person(person,xstring,ystring,
                 ycounts,ysums,ymins,ymaxs,xvals)
    }
  }
  else {
    if (ne(who,3)) {
      addtoset(people,of,0)
      if (eq(who,1)) { set(people,descendentset(people)) }
      elsif (eq(who,2)) { set(people,ancestorset(people)) }
      addtoset(people,of,0)
    }
    forindiset(people,person,pval,pnum) {
      call stat_person(person,xstring,ystring,
                             ycounts,ysums,ymins,ymaxs,xvals)
    }
  }

  set(sort_it,0)
  if (eq(deltabin,0)) {
    getintmsg(sort_it,"Sort the output? (0=no, 1=yes)")
  }
  if (sort_it) { call quicksort(xvals,ilist) }
  else { forlist(ycounts,ycount,ynum) { enqueue(ilist,ynum) } }

  if (log) { sepline }
  sepline
  "Statistics of " ystring " binned by " xstring
  "\nfor "
  if (eq(who,0)) { "all individuals" }
  elsif (eq(who,1)) { "descendants of " ofkey " " name(indi(ofkey)) }
  elsif (eq(who,2)) { "ancestors of "  ofkey " " name(indi(ofkey)) }
  elsif (eq(who,3)) { "selected set of individuals" }
  "\nin database " database()
  if (not(empty(relist))) {
    " subject to"
    while(not(empty(relist))) {
      "\n"
      dequeue(relist)
    }
  }
  "\n"
  sepline
  col(6) xstring if (gt(deltabin,0)) { " range" }
  col(30) "bin" col(38) "total" col(50) "min" col(60) "max"
  col(70) "average\n"
  "bin" col(6)
  if (gt(deltabin,0)) { "from" col(18) "to" set(binx,minbin) }
  else { "label" }
  col(30) "count" col(38) ystring col(50) ystring col(60)
  ystring col(70) ystring "\n"
  sepline
  set(bin,0)
  set(allycount,0)
  set(allysum,0)
  set(allymin,999999)
  set(allymax,neg(999999))
  forlist(ilist,index,num) {
    set(ycount,getel(ycounts,index))
    set(ysum,getel(ysums,index))
    set(ymin,getel(ymins,index))
    set(ymax,getel(ymaxs,index))
    incr(bin)
    d(bin) col(6)
    if (gt(deltabin,0)) {
      d(binx)
      set(binx,add(binx,deltabin))
      col(18) d(binx)
    }
    else { getel(xvals,index) }
    col(30) d(ycount) set(allycount,add(allycount,ycount))
    col(38) dd(ysum,ycount) set(allysum,add(allysum,ysum))
    col(50) dd(ymin,ycount) if (lt(ymin,allymin)) { set(allymin,ymin) }
    col(60) dd(ymax,ycount) if (gt(ymax,allymax)) { set(allymax,ymax) }
    col(70)
    if (gt(ycount,0)) {
      d(div(ysum,ycount)) "."
      set(frac,div(mul(mod(ysum,ycount),accuracy),ycount))
      call print_frac(frac)
    }
    else { "--" }
    "\n"
  }
  sepline
  "all" col(6)
  if (gt(deltabin,0)) {
    dd(minbin,allycount) col(18) dd(maxbin,allycount)
  }
  else { xstring }
  col(30) d(allycount)
  col(38) dd(allysum,allycount)
  col(50) dd(allymin,allycount)
  col(60) dd(allymax,allycount)
  col(70)
  if (gt(allycount,0)) {
    d(div(allysum,allycount)) "."
    set(frac,div(mul(mod(allysum,allycount),accuracy),allycount))
    call print_frac(frac)
  }
  else { "--" }
  "\n"
  sepline
}

func dd(value,count) {
  if (count) { return(d(value)) }
  return("--")
}

proc init_vtable() {
  table(vtable)
  list(initials)
  enqueue(initials,"B")
  enqueue(initials,"C")
  enqueue(initials,"M")
  enqueue(initials,"N")
  enqueue(initials,"P")
  enqueue(initials,"Q")
  enqueue(initials,"D")
  enqueue(initials,"E")
  enqueue(initials,"T")
  list(numbers)
  enqueue(numbers,"AGE")
  enqueue(numbers,"FAGE")
  enqueue(numbers,"MAGE")
  enqueue(numbers,"YEAR")
  enqueue(numbers,"MONTH")
  enqueue(numbers,"DAY")
  forlist(initials,initial,lnum) {
    forlist(numbers,number,nnum) {
      insert(vtable,save(concat(initial,number)),2)
    }
    insert(vtable,save(concat(initial,"PLACE")),1)
  }
  insert(vtable,"UNITY",2)
  insert(vtable,"KIDS",2)
  insert(vtable,"FAMILIES",2)
  insert(vtable,"SIBLINGS",2)
  insert(vtable,"SIBLINGORDER",2)
}

/* compute a value type from the value specification string */
func value_type(spec_string) {
  /* print(spec_string," ",d(lookup(vtable,spec_string)),"\n") */
  if (vt,lookup(vtable,spec_string)) { return(vt) }
  return(1) /* SEX, SURNAME, FIRSTNAME, SOUNDEX, or GEDCOM tag */
}

/* get the specified value for a person */
func get_val(person,spec_string) {
  list(namelist)
  set(ret_type,0)
  set(event,0)
  set(initial,save(trim(spec_string,1)))
  if (eqstr(initial,"B")) {
    set(e,birth(person))
    set(e1,baptism(person))
    set(event,1)
  }
  elsif (eqstr(initial,"C")) {
    set(e,baptism(person))
    set(e1,birth(person))
    set(event,1)
  }
  elsif (eqstr(initial,"D")) {
    set(e,death(person))
    set(e1,burial(person))
    set(event,1)
  }
  elsif (eqstr(initial,"E")) {
    set(e,burial(person))
    set(e1,death(person))
    set(event,1)
  }
  elsif (eqstr(initial,"M")) {
    families(person,fam,spouse,fnum) {
      set(e,marriage(fam))
      set(event,1)
      break()
    }
  }
  elsif (eqstr(initial,"N")) {
    families(person,fam,spouse,fnum) {
      set(e,marriage(fam))
      set(event,1)
    }
  }
  elsif (eqstr(initial,"P")) {
    families(person,fam,spouse,fnum) {
      if (child,firstchild(fam)) {
        set(e,birth(child))
        set(e1,baptism(child))
        set(event,1)
        break()
      }
    }
  }
  elsif (eqstr(initial,"Q")) {
    families(person,fam,spouse,fnum) {
      if (child,lastchild(fam)) {
        set(e,birth(child))
        set(e1,baptism(child))
        set(event,1)
      }
    }
  }
  elsif (eqstr(initial,"T")) {
    set(e,gettoday())
    set(event,1)
  }
  if (eq(event,1)) {
    if (e) { extractdate(e,day,month,year) }
    if (e1) { extractdate(e1,day1,month1,year1) }
    if (eqstr(spec_string,concat(initial,"YEAR"))) {
      if (year) { set(ret_type,2) return(year) }
      if (and(not_strict,year1)) {
        set(ret_type,2) return(year1)
      }
      return(0)
    }
    if (eqstr(spec_string,concat(initial,"MONTH"))) {
      if (month) { set(ret_type,2) return(month) }
      if (and(not_strict,month1)) {
        set(ret_type,2) return(month1)
      }
      return(0)
    }
    if (eqstr(spec_string,concat(initial,"DAY"))) {
      if (day) { set(ret_type,2) return(day) }
      return(0)
    }
    if (eqstr(spec_string,concat(initial,"AGE"))) {
      set(byear,0)
      if (b,birth(person)) { extractdate(b,bday,bmonth,byear) }
      if (and(not(byear),not_strict)) {
        if (b,baptism(person)) { extractdate(b,bday,bmonth,byear) }
      }
      if (byear) {
        if (year) {
          set(ret_type,2) return(sub(year,byear))
        }
        if (and(not_strict,year1)) {
          set(ret_type,2) return(sub(year1,byear))
        }
      }
      return(0)
    }
    if (eqstr(spec_string,concat(initial,"FAGE"))) {
      set(byear,0)
      if(b,birth(father(person))) { extractdate(b,bday,bmonth,byear) }
      if (and(not(byear),not_strict)) {
        if(b,baptism(father(person))) {
          extractdate(b,bday,bmonth,byear)
        }
      }
      if (byear) {
        if (year) {
                   set(ret_type,2) return(sub(year,byear))
        }
        if (and(not_strict,year1)) {
          set(ret_type,2) return(sub(year1,byear))
        }
      }
      return(0)
    }
    if (eqstr(spec_string,concat(initial,"MAGE"))) {
      set(byear,0)
      if(b,birth(mother(person))) { extractdate(b,bday,bmonth,byear) }
      if (and(not(byear),not_strict)) {
        if(b,baptism(mother(person))) {
          extractdate(b,bday,bmonth,byear)
        }
      }
      if (byear) {
        if (year) {
          set(ret_type,2) return(sub(year,byear))
        }
        if (and(not_strict,year1)) {
          set(ret_type,2) return(sub(year1,byear))
        }
      }
      return(0)
    }
    if (eqstr(spec_string,concat(initial,"PLACE"))) {
      set(ret_val,save(place(e)))
      if (and(not_strict,eqstr(ret_val,""))) {
        set(ret_val,save(place(e1)))
      }
      set(ret_type,1)
      return(ret_val)
    }
  }
  if (eqstr(spec_string,"KIDS")) {
    set(nkids,0)
    families(person,fam,spouse,fnum) {
      set(nkids,add(nkids,nchildren(fam)))
    }
    set(ret_type,2)
    return(nkids)
  }
  if (eqstr(spec_string,"FAMILIES")) {
    set(ret_type,2)
    return(nfamilies(person))
  }
  if (eqstr(spec_string,"SIBLINGS")) {
    if (fam,parents(person)) {
      set(ret_type,2)
      return(nchildren(fam))
    }
    return(0)
  }
  if (eqstr(spec_string,"SIBLINGORDER")) {
    if (fam,parents(person)) {
      set(ret_type,2)
      set(cnum,1)
      set(child,person)
      while(child,prevsib(child)) { incr(cnum) }
      return(cnum)
    }
    return(0)
  }
  if (eqstr(spec_string,"UNITY")) {
    set(ret_type,2)
    return(1)
  }
/* The next four lines will work even if you comment them out.
   Sex is a powerful force, I guess. */
  if (eqstr(spec_string,"SEX")) {
    set(ret_type,1)
    return(save(sex(person)))
  }
  if (eqstr(spec_string,"SURNAME")) {
    set(ret_type,1)
    return(save(surname(person)))
  }
  if (eqstr(spec_string,"FIRSTNAME")) {
    set(ret_type,1)
    extractnames(inode(person), namelist, ncomp, sindx)
    if( or( gt(sindx,1), gt(ncomp,sindx))) {
      set(gindx,1) if(eq(sindx,1)) { set(gindx,2) }
      return(save(getel(namelist, gindx)))
    }
    return("")
  }
  if (eqstr(spec_string,"SOUNDEX")) {
    set(ret_type,1)
    return(save(soundex(person)))
  }
  fornodes(inode(person),node) {
    if (eqstr(tag(node),spec_string)) {
      set(ret_type,1)
      return(save(value(node)))
    }
  }
  return(0)
}

func find_bin(xvalue,ycounts,ysums,ymins,ymaxs,xvals) {
  if (gt(deltabin,0)) { /* numeric data type */
    if (lt(maxbin,minbin)) { /* first time through */
      set(minbin,sub(xvalue,mod(xvalue,deltabin)))
      set(maxbin,add(minbin,deltabin))
      enqueue(ycounts,0)
      enqueue(ysums,0)
      enqueue(ymins,999999)
      enqueue(ymaxs,neg(999999))
      return(1)
    }
    while (lt(xvalue,minbin)) {
      requeue(ycounts,0)
      requeue(ysums,0)
      requeue(ymins,999999)
      requeue(ymaxs,neg(999999))
      set(minbin,sub(minbin,deltabin))
    }
    while (ge(xvalue,maxbin)) {
      enqueue(ycounts,0)
      enqueue(ysums,0)
      enqueue(ymins,999999)
      enqueue(ymaxs,neg(999999))
      set(maxbin,add(maxbin,deltabin))
    }
    return(add(div(sub(xvalue,minbin),deltabin),1))
  }
/* unsorted string data type */
  if (r,lookup(xvaltable,xvalue)) { return(r) }
  enqueue(xvals,xvalue)
  enqueue(ycounts,0)
  enqueue(ysums,0)
  enqueue(ymins,999999)
  enqueue(ymaxs,neg(999999))
  set(r,length(xvals))
  insert(xvaltable,xvalue,r)
  return(r)
}

func filter_person(person) {
  forlist(rlist,restriction,rnum) {
    set(rtype,getel(tlist,rnum))
    set(inequality,getel(clist,rnum))
    set(cvalue,getel(vlist,rnum))
    set(ret_val,get_val(person,restriction))
    if (eq(ret_type,rtype)) {
      if (eq(rtype,2)) { /* numeric */
        if (eqstr(inequality,"=")) {
          if (not(eq(ret_val,cvalue))) { return(0) }
        }
        elsif (eqstr(inequality,"!=")) {
          if (not(ne(ret_val,cvalue))) { return(0) }
        }
        elsif (eqstr(inequality,">")) {
          if (not(gt(ret_val,cvalue))) { return(0) }
        }
        elsif (eqstr(inequality,"<")) {
          if (not(lt(ret_val,cvalue))) { return(0) }
        }
        elsif (eqstr(inequality,">=")) {
          if (not(ge(ret_val,cvalue))) { return(0) }
        }
        elsif (eqstr(inequality,"<=")) {
          if (not(le(ret_val,cvalue))) { return(0) }
        }
        else { return(0) } /* error condition */
      }
      else { /* string */
        if (eqstr(inequality,"=")) {
          if (not(eq(strcmp(ret_val,cvalue),0))) { return(0) }
        }
        elsif (eqstr(inequality,"!=")) {
          if (not(ne(strcmp(ret_val,cvalue),0))) { return(0) }
        }
        elsif (eqstr(inequality,">")) {
          if (not(gt(strcmp(ret_val,cvalue),0))) { return(0) }
        }
        elsif (eqstr(inequality,"<")) {
          if (not(lt(strcmp(ret_val,cvalue),0))) { return(0) }
        }
        elsif (eqstr(inequality,">=")) {
          if (not(ge(strcmp(ret_val,cvalue),0))) { return(0) }
        }
        elsif (eqstr(inequality,"<=")) {
          if (not(le(strcmp(ret_val,cvalue),0))) { return(0) }
        }
        elsif (eqstr(inequality,"contains")) {
          if (not(index(ret_val,cvalue,1))) { return(0) }
        }
        elsif (eqstr(inequality,"isin")) {
          if (not(index(cvalue,ret_val,1))) { return(0) }
        }
        else { return(0) } /* error condition */
      }
    }
    else { return(0) } /* error condition */
  }
  return(1)
}

proc stat_person(person,xstring,ystring,ycounts,ysums,ymins,ymaxs,xvals) {
  if (filter_person(person)) {
    set(value,get_val(person,ystring))
    if (eq(ret_type,2)) {
      set(ret_val,get_val(person,xstring))
      if (ret_type) {
        if (log) {
          key(person) col(8) name(person) col(50)
          if (eq(ret_type,1)) { ret_val } else { d(ret_val) }
          col(65) d(value) "\n"
        }
        set(bin,find_bin(ret_val,ycounts,ysums,ymins,ymaxs,xvals))
        setel(ycounts,bin,add(getel(ycounts,bin),1))
        setel(ysums,bin,add(getel(ysums,bin),value))
        if (lt(value,getel(ymins,bin))) { setel(ymins,bin,value) }
        if (gt(value,getel(ymaxs,bin))) { setel(ymaxs,bin,value) }
      }
    }
  }
}

proc print_frac(frac) {
  set(check,div(accuracy,10))
  while (gt(check,1)) {
    if (lt(frac,check)) { "0" }
    set(check,div(check,10))
  }
  d(frac)
}

func compare(astring,bstring) {
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
