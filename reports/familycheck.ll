/*
 * @progname    familycheck.ll
 * @version     2000-03-02
 * @author      Dennis Nicklaus nicklaus@fnal.gov
 * @category
 * @output      Text, 80 cols
 * @description
 *
        Consistency checks between indi records and family records
	(making sure links between kids and spouses go both ways.)
	make sure each family that a person says he is a spouse of
	has him as a spouse, and, vice-versa,
	 make sure each person that a family says is a spouse thinks he
	 is a spouse of that family
	It also checks when a person says he is a child in a family that
	the family has that person as a child. 
	And vice-versa, that every child in a family thinks he
	is a child of that family.

	Written by Dennis Nicklaus nicklaus@fnal.gov, 1997.
*/
/* Modifications:
 * 02-mar-00 pbm report multiple HUSB, WIFE, FAMC
 * 01-mar-00 pbm optionally allow SEX U, and missing SEX records
 * 25-sep-99 pbm check for a child in a family more than once
 * 19-feb-99 pbm check for multiple SEX records
 * 13-feb-99 pbm report a child in family when reporting a family with
 *               no parents.
 *		 always show keys of family and individual involved.
 *		 display all messages on screen and write to file.
 */

global(ALLOWSEXU)	/* set to 1 if "SEX U" is allowed */
global(WARNNOSEX)	/* set to 1 to warn about INDIs with no SEX record */

proc main ()
{
    /* user customization section. change the following if desired: */
    set(ALLOWSEXU, 1)	/* 1: "SEX U" should not generate a warning */
    set(WARNNOSEX, 0)	/* 0: don't warn if INDI has no SEX record */
    /* end of user customization section */

    print("processing each person and family in the database...")

    forindi(person, number) {
    	
	call checksex(person)
        families(person, fam, spouse, nfam) {
		set(okboss,0)
		set(s, child(root(fam)))
		while(s) {
		  if(or(eqstr(tag(s), "HUSB"), eqstr(tag(s), "WIFE"))) {
		    if(v, value(s)) {
		      if(reference(v)) {
			if(eqstr(substring(v, 2,sub(strlen(v),1)),key(person))){
			  set(okboss, add(okboss,1))
			}
		      }
		    }
		  }
		  set(s, sibling(s))
		}
		if (eq(0,okboss)){
		  print("\nperson ",key(person)," is not a spouse in ",key(fam))
		  "person " key(person) " is not a spouse in " key(fam) nl()
		}
		elsif(ne(1,okboss)){
		  print("\nperson ",key(person)," is a spouse in ",key(fam),
		  	" ",d(okboss)," times")
		  "person " key(person) " is a spouse in " key(fam)
		  	" " d(okboss) " times" nl()
		}
	
	}
	/* now check that this person is a child in the family
	   he thinks he is (and only once) */
	set(fcnt, 0)
	set(s, child(root(person)))
	while(s) {
	  if(eqstr(tag(s), "FAMC")) {
	    if(v, value(s)) {
	      set(fcnt, add(fcnt, 1))
	      if(reference(v)) {
		set(okboss,0)
		children(fam(v),child,num){
		  if (eq(person,child)) {set(okboss, add(okboss,1))}
		}
		set(x, substring(v, 2, sub(strlen(v),1)))
	        if (eq(0,okboss)){
		   print("\nperson ",key(person)," is not in family ", x)
		   "person " key(person) " is not in family "  x nl()
	        }
	        if (gt(okboss, 1)) {
		   print("\nperson ",key(person)," is in family ",x," ",
		   	d(okboss), " times")
		   "person " key(person) " is in family " x " "
		   	d(okboss) " times" nl()
		}
	      }
	    }
	  }
	  set(s, sibling(s))
	}
	if(gt(fcnt, 1)) {
	  print("\nperson ",key(person)," is a child in ", d(fcnt), " families")
	  "person " key(person) " is a child in " d(fcnt) " families" nl()
	}
    }

  /* now check families so that for every spouse the family says is in the 
     family, that spouse also thinks he/she is in the family. */
  /* the family keys aren't terribly useful in LL (or out since LL
     will change the key numbers on import), so print out the key
     of the indi involved, also */

    forfam(fam, number) {
	set(wcnt,0)
	set(hcnt,0)
	set(s, child(root(fam)))
	while(s) {
	  if(or(eqstr(tag(s), "HUSB"), eqstr(tag(s), "WIFE"))) {
	    if(eqstr(tag(s), "HUSB")) {
	      set(hcnt, add(hcnt,1))
	      if(gt(hcnt, 1)) {
	        print("\nfamily ",key(fam)," has more then one husband ",
			substring(value(s), 2, sub(strlen(value(s)),1)))
	        "family " key(fam) " has more then one husband "
			substring(value(s), 2, sub(strlen(value(s)),1))
	      }
	    }
	    if(eqstr(tag(s), "WIFE")) {
	      set(wcnt, add(wcnt,1))
	      if(gt(wcnt, 1)) {
	        print("\nfamily ",key(fam)," has more then one wife ",
			substring(value(s), 2, sub(strlen(value(s)),1)))
	        "family " key(fam) " has more then one wife "
			substring(value(s), 2, sub(strlen(value(s)),1))
	      }
	    }
	    if(v, value(s)) {
	      if(reference(v)) {
	        set(i, indi(v))
		if(eq(i,0)) {
		  print("\nmissing person ",v," in family ",key(fam))
		  "missing person " v " in family " key(fam) nl()
		}
		else {
		  set(okboss,0)
           	  families(i, fam2, spouse, nfam) {
		    if (eq(fam,fam2)){  set(okboss,add(okboss,1))}
	   	  }
		  if(eq(okboss, 0)) {
		    print("\nperson ",key(i),
		    	" is not linked as a spouse to family ", key(fam))
		    "person " key(i) 
		    	" is not linked as a spouse to family " key(fam)
		  }
		  if(gt(okboss, 1)) {
		    print("\nperson ",key(i),
		    	" is linked as a spouse to family ", key(fam),
			" ",d(okboss)," times")
		    "person " key(i) 
		    	" is linked as a spouse to family " key(fam)
			" " d(okboss) " times" 
		  }
		}
	      }
	    }
	  }
	  set(s, sibling(s))
	}
	if (eq(add(hcnt, wcnt),0)) {
	   print("\nno parents in family ",key(fam))
	   "no parents in family " key(fam)
	   children(fam,child,num){
	     print(" ",key(child))
	     " " key(child)
	     break()
	   }
	   nl()
	}
	children(fam,child,num) {
	  set(ccnt, 0)
	  set(s, child(root(child)))
	  while(s) {
	    if(eqstr(tag(s), "FAMC")) {
	      if(v, value(s)) {
	        if(reference(v)) {
	          if(eqstr(substring(v,2,sub(strlen(v),1)), key(fam))) {
		    set(ccnt, add(ccnt, 1))
		  }
		}
	      }
	    }
	    set(s, sibling(s))
	  }
	  if(eq(ccnt,0)) {
	    print("\nchild ",key(child)," is not linked to family ",key(fam))
	    "child " key(child) " is not linked to family " key(fam) nl()
	  }
	  if(gt(ccnt,1)) {
	    print("\nchild ",key(child)," is linked to family ",key(fam),
	    	" ", d(ccnt)," times")
	    "child " key(child) " is linked to family " key(fam)
	    	" " d(ccnt) " times" nl()
	  }
	}
    }
}

proc checksex(i)
{
    set(val, "")
    set(count, 0)
    set(r, inode(i))
    traverse (r, n, x) {
      if(eqstr(tag(n), "SEX")) {
        set(count, add(count,1))
        if(eq(value(n),0)) {
	  print("\nSEX record with no value ",key(i))
	  "SEX record with no value " key(i) nl()
	}
	elsif(or(eqstr(value(n), "M"), eqstr(value(n), "F"),
	         eqstr(value(n), "?"),
		 and(ALLOWSEXU,eqstr(value(n), "U")))) {
	  if(and(ne(count, 1),not(eqstr(value(n),val)))) {
	    print("\nconflicting SEX records ",val," and ",value(n)," ",key(i))
	    "conflicting SEX records " val " and " value(n) " " key(i) nl()
	  }
	  set(val,value(n))
	}
	else {
	  print("\nSEX record with unrecognize value ",value(n)," ",key(i))
	  "SEX record with unrecognize value " value(n) " " key(i) nl()
	  set(val,value(n))
	}
      }
    }
    if(and(WARNNOSEX, eq(count, 0))) {
      print("\nno SEX record ",key(i))
      "no SEX record " key(i) nl()
    }
    elsif(gt(count,1)) {
      print("\ntoo many SEX records (",d(count),") ",key(i))
      "too many SEX records (" d(count) ") " key(i) nl()
    }
}

proc countnodes(n)
{
  set(count,  0)
  if(n) {
    set(count, 1)
    set(t, tag(n))
    while(s, sibling(n)) {
      if(eqstr(tag(s), t)) {
        set(count, add(count,1))
      }
    }
  }
  return(count)
}
