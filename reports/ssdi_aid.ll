/*
 * @progname       ssdi_aid.ll
 * @version        1
 * @author         Jim Eggert (eggertj@ll.mit.edu)
 * @category       
 * @output         Text
 * @description

This LifeLines report program generates a text file that lists
people who are likely to be in the Social Security Death Index.
The SSDI starts in 1962 and is periodically updated to include
more recent years.  This program guesses birth and death years
to make its determinations.  If it finds a person likely to be
in the SSDI, it searches for the string SSDI in their notes to
indicate that an SSDI entry has already been found.  If so, it
prints an asterisk to indicate found entries.

The output persons are in database order.  Women are output in
their own name and any married name.  To alphabetize the names,
you can use Unix sort:

  sort -b +1 ss.out > ss.sort

ssdi_aid - a LifeLines program to aid in the use of the U.S. Social
           Security Death Index
          by Jim Eggert (eggertj@ll.mit.edu)
          Version 1, 28 June 1995
*/

global(byear_delta)
global(byear_est)
global(byear_est_delta)

global(mother_age)
global(father_age)
global(years_between_kids)
global(oldage)

proc main() {
  indiset(pset)

  set(mother_age,23)  /* assumed age of first motherhood */
  set(father_age,25)  /* assumed age of first fatherhood */
  set(years_between_kids,2) /* assumed years between children */
  set(oldage,90)

  getindi(person)
  while(person) {
    addtoset(pset,person,1)
    getindi(person)
  }

  getintmsg(minage,"Enter minimum age for listing:")

  set(namewidth,50)  /* change this value as needed */
  "key" col(8) "@LAST, First Middle [MAIDEN]"
  set(bcol,add(8,namewidth))
  col(bcol) "Birthdate"
  set(dcol,add(25,namewidth))
  col(dcol) "Death\n"

  print("Finding descendants")
  set(pset,union(pset,spouseset(pset)))
  set(pset,union(pset,descendantset(pset)))
  print("' spouses")
  set(pset,union(pset,spouseset(pset)))
  print("... done.\n")

  set(thisyear,atoi(year(gettoday())))
  set(byearend,sub(thisyear,minage))

  print("Traversing individuals...")
  forindiset(pset,person,pval,pnum) {
    set(byear,0)
    set(bdate,"")
    if (b,birth(person)) {
      extractdate(b,bday,bmonth,byear)
      set(bdate,date(b))
    }
    if (not(byear)) {
      if (b,baptism(person)) {
        set(bdate,date(b))
      }
    }
    call estimate_byear(person)
    set(byear,sub(byear_est,byear_est_delta))
    if(and(byear_est,not(strlen(bdate)))) {
        set(bdate,save(concat("c ",d(byear_est))))
    }

    set(dyear,0)
    if (d,death(person)) {
      extractdate(d,dday,dmonth,dyear)
    }
    if (not(dyear)) {
      if(d,burial(person)) {
        extractdate(d,dday,dmonth,dyear)
      }
    }
    if (dyear) {
      if (or(index(date(d),"ABT",1),eq(dmonth,0))) { set(dyear,add(dyear,5)) }
      if (index(date(d),"AFT",1)) {
        set(oldyear,add(byear,oldage))
        if (gt(oldyear,dyear)) { set(dyear,oldyear) }
      }
    }

    if (or(ge(dyear,1962),
           and(not(dyear),le(byear,byearend)))) {
      set(star,0)
      fornotes(inode(person),note) {
        if (index(note,"SSDI",1)) { set(star,1) }
      }
      if (star) { "*" } else { " " }
      key(person) col(8)
      set(nsp,nspouses(person))
      set(no_mname,1)
      if (and(female(person),ne(nsp,0))) {
        set(maidenname,save(concat(", ",fullname(person,1,1,100))))
        spouses(person,spouse,fam,spnum) {
          if (eq(spnum,nsp)) {
            if (spousesurname,surname(spouse)) {
              trim(concat(upper(spousesurname),maidenname),namewidth)
              set(no_mname,0)
            }
          }
        }
        if (no_mname) { "*" }
      }
      if (no_mname) { fullname(person,1,0,namewidth) }
      col(bcol) bdate col(dcol) long(d) nl()
    }
  }
}

proc estimate_byear(person) {
    set(byear_est,0)
    set(byear_est_delta,neg(1))
    if (byear,get_byear(person)) {
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
                if (byear,get_byear(older)) {
                    set(byear_est,add(byear,yeardiff))
                    set(byear_est_delta,this_uncertainty)
                }
            }
            if (and(not(byear_est),younger))  {
                if (byear,get_byear(younger)) {
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
                if (b,birth(spouse)) { extractdate(b,bd,bm,by) }
                if (m,marriage(fam)) { extractdate(m,bd,bm,my) }
                if (by) {
                    if (female(person)) {
                        set(byear_est,add(by,sub(father_age,mother_age)))
                    }
                    else {
                        set(byear_est,sub(by,sub(father_age,mother_age)))
                    }
                    set(byear_est_delta,5)
                }
                elsif (my) {
                    if (female(person)) { set(byear_est,sub(my,mother_age)) }
                    else { set(byear_est,sub(my,father_age)) }
                    set(byear_est_delta,5)
                }
                else {
                    children(fam,child,cnum) {
                        if (not(byear_est)) {
                            if (byear,get_byear(child)) {
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
        if (byear,get_byear(mother(person))) {
            set(byear_est,add(byear,mother_age))
        }
        else {
            if (byear,get_byear(father(person))) {
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

func get_byear(person) {
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
    return(byear)
}
