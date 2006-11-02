/*
 * @progname       ssdi_aid.ll
 * @version        3
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
indicate that an SSDI entry has already been found.  If not, it
outputs a line about that person.

The output persons are in database order.  Women are output in
with their last married name.  To alphabetize the names in the text
report, you can use Unix sort:

  sort -b +1 ss.out > ss.sort

The program optionally generates HTML output with buttons to search
the Rootsweb online SSDI database.

ssdi_aid - a LifeLines program to aid in the use of the U.S. Social
           Security Death Index
          by Jim Eggert (eggertj@ll.mit.edu)
          Version 1, 28 June 1995
          Version 2, 22 November 1996
          Version 3, 11 January 2005 (changed to Rootsweb site)

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
  set(oldage,90)       /* normal maximum death age */
  set(byearstart,1850) /* no one born before then can be in the SSDI */

  set(unknownname,"<") /* for women, any spouse whose surname contains this
                          is considered to have an unknown surname */

  getindi(person)
  while(person) {
    addtoset(pset,person,1)
    getindi(person)
  }

  getintmsg(minage,"Enter minimum age for listing:")

  getintmsg(html,"Enter 0 for text, 1 for html output:")

  if (html) {
    getintmsg(includebyears,"Enter 1 to include birth years in database query")
    "<HTML>\n"
    "<HEAD>\n"
    "<TITLE> SSDI Aid Report </TITLE>\n"
    "</HEAD>\n"
    "<BODY>\n"
    "Press a button to query Rootsweb's online SSDI database
     for that individual."
    "<HR>\n"
  }

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
  print("' descendants")
  set(pset,union(pset,descendantset(pset)))
  print("... done.\n")

  set(thisyear,atoi(year(gettoday())))
  set(byearend,sub(thisyear,minage))

  print("Traversing individuals...")
  forindiset(pset,person,pval,pnum) {
    set(star,1)
    fornotes(inode(person),note) {
      if (index(note,"SSDI:",1)) { set(star,0) }
    }
    if (star) {
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
/*    set(byear,sub(byear_est,byear_est_delta)) */
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

      if (or(ge(dyear,1940),
             and(not(dyear),le(byear,byearend),ge(byear,byearstart)))) {
        set(nfam,nfamilies(person))
        set(myname,fullname(person,1,0,namewidth))
        set(mysurname,surname(person))
        if (and(female(person),ne(nfam,0))) {
          set(maidenname,save(concat(", ",fullname(person,1,1,100))))
          families(person,fam,spouse,famnum) {
            if (spousesurname,surname(spouse)) {
              if (strlen(spousesurname)) {
                if (not(index(spousesurname,unknownname,1))) {
                  set(mysurname,spousesurname)
                  set(myname,
                        trim(concat(upper(spousesurname),maidenname),namewidth))
                  if (ne(famnum,nfam)) {
                    set(myname,
                        trim(concat("+",myname),namewidth))
                  }
                }
              }
            }
          }
        }
        if (html) {
          "<form method=post action=" qt()
          "http://ssdi.rootsweb.com/cgi-bin/ssdi.cgi" qt()
          "><input type=hidden name=lastname value=" qt()
          mysurname qt()
          "><input type=hidden name=firstname value=" qt()
          list(givennamelist)
          extracttokens(givens(person),givennamelist,nnames," ")
          dequeue(givennamelist) qt() ">"
          if (includebyears) {
            if (lt(byear_est_delta,2)) {
              "<input type=hidden name=birth value=" qt()
              d(byear) qt() ">"
            }
          }
          "<input type=submit name=submit value=" qt() key(person) qt()
          "><input type=hidden name=nt value=" qt() "exact" qt() ">"
        } else { 
          key(person) col(8)
        }
        myname
        if (html) {
          " " bdate " " long(d) "</form>"
        }
        else {
          col(bcol) bdate col(dcol) long(d)
        }
        nl()
      }
    }
  }
  if (html) {
    "</BODY>\n"
    "</HTML>\n"
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
