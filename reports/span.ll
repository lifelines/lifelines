/*
 * @progname       span
 * @version        1.1
 * @author         Stephen Dum
 * @category       
 * @output         text
 * @description    

Scan the database and report on the range of ages between birth to marriage,
birth of parent to birth of child, and age at death.  Generates a histogram
of the results and reports minimum, maximum and average values.  Designed
to be run with llexec, with a command like 'llexec database -x span'.

Note, the resultant histogram will normally fit nicely in a 80 column window,
(except death range, which could take more like 132 columns)
if it doesn't it's usually because of some bogus dates (like seeing a mothers
age as -8 or 70 at the birth of a child.)  This script contains added
complexity to identify the min and max cases, however, the script verify.ll
will report all the outlying cases in one pass.  The average value is
indicated on the histogram by using asterisks (*).

Also, you can disable the scripts using of dates that are estimates.
However, this is done by modifing the script rather than having the script
prompt for an answer, as this is expected to be a less likely case.

         Stephen Dum (stephen.dum@verizon.net)
         Version 1,  2 November 2005 
*/

global(dohist)
global(estdate)

proc main()
{
    set(dohist,1)      /* generate histograms */
    set(estdate,0)     /* skip estimated dates */
    /* we accumulate 4 statistics from the database and store them in
     * the following lists
     */
    list(hus_mar)      /* marriage age of husband */
    list(wif_mar)      /* marriage age of wife */
    list(hus_child)    /* husbands age at birth of child */
    list(wif_child)    /* wifes age at birth of child */
    list(death_ages)   /* age at death */
    /* to assist in identifing the unusual extreme situations, 
     * (like where it reports a husband was married at 192 years old
     * or at -46 years old
     * we keep some auxilary data for these lists
     */
    list(hus_mar_id)      /* Family and husband keys */
    list(wif_mar_id)      /* Family and wife keys */
    list(hus_child_id)    /* Family, husband and child key */
    list(wif_child_id)    /* Family, wife and child key */
    list(death_ages_id)    /* Family, husband and child key */

    forfam(fam, cnt) {
       list(hus_dates)    /* husband birth dates */
       list(wif_dates)    /* wife birth dates */
       list(child_dates)  /* child birth dates */
       list(hus_id)       /* husband id */
       list(wif_id)       /* wife id */
       list(child_id)     /* child id */
	/* first process the family and get birth dates for husband, wife and
	 * children 
	 */
	set(marr_date,get_marriage_date(fam))

	fornodes(fam,node) {
	    if (eqstr(tag(node),"HUSB")) {
	        if (val, get_birth_date(indi(value(node)))) {
		    push(hus_dates,val)
		    push(hus_id,key(indi(value(node))))
		}
	    } elsif (eqstr(tag(node),"WIFE")) {
	        if (val, get_birth_date(indi(value(node)))) {
		    push(wif_dates,val)
		    push(wif_id,key(indi(value(node))))
		}
	    } elsif (eqstr(tag(node),"CHIL")) {
	        if (val, get_birth_date(indi(value(node)))) {
		    push(child_dates,val)
		    push(child_id,key(indi(value(node))))
		}
	    }
	}
	/*
	print ("Length of hus_dates ",d(length(hus_dates)),nl())
	print ("Length of wif_dates ",d(length(wif_dates)),nl())
	print ("Length of child_dates ",d(length(child_dates)),nl())
	*/
        /* we now have parents and children dates if any, see if
	 * we know enough to process them
	 * 
	 * First - look at marriage date vs parents
         */
	if (marr_date) {
	    forlist(hus_dates,val,cnt) {
	        push(hus_mar,sub(marr_date,val))
	        push(hus_mar_id,concat(key(fam)," ",getel(hus_id,cnt)))
	    }
	    forlist(wif_dates,val,cnt) {
	        push(wif_mar,sub(marr_date,val))
	        push(wif_mar_id,concat(key(fam)," ",getel(wif_id,cnt)))
	    }
	}

	 /* Second
	       for each parent - child pair
	  */
	 forlist(hus_dates,val,cnt) {
	     forlist(child_dates,val1,cnt1) {
	        push(hus_child,sub(val1,val))
	        push(hus_child_id,
		     concat(key(fam)," ",getel(hus_id,cnt)," ",getel(child_id,cnt1))
	        )
	     }
	 }
	 forlist(wif_dates,val,cnt) {
	     forlist(child_dates,val1,cnt1) {
	        push(wif_child,sub(val1,val))
	        push(wif_child_id,
		     concat(key(fam)," ",getel(wif_id,cnt)," ",getel(child_id,cnt1))
	        )
	     }
	 }
    }
    forindi(indi,cnt) {
	if (val, get_birth_date(indi)) {
	    if (val2,get_death_date(indi)) {
		push(death_ages,sub(val2,val))
		push(death_ages_id,key(indi))
	    }
	}
    }
    print(nl())
    if (not(dohist)) {
	print("                        min    ave     max pairs keys of match",nl())
    }
    call output(hus_mar,hus_mar_id,     "Male Marriage Age  ")
    call output(wif_mar,wif_mar_id,     "Female Marriage Age")
    call output(hus_child,hus_child_id, "Husband-Child Age  ")
    call output(wif_child,wif_child_id, "Wife-Child Age     ")
    call output(death_ages,death_ages_id,"Death Age         ")
}

proc output(alist,idlist,title) 
{
    list(hist)
    if (length(alist)) {
        set(min,getel(alist,1))
        set(max,min)
        set(min_id,getel(idlist,1))
        set(max_id,min_id)
        set(sum,0)
        forlist(alist,val,cnt) {
	    /* compute histogram data */
            set(x,div(val,365))
	    setel(hist,x,add(getel(hist,x),1))

	    if (gt(min,val)) {
	        set(min,val)
	        set(min_id,getel(idlist,cnt))
	    }
	    if (lt(max,val)) {
	       set(max,val)
	       set(max_id,getel(idlist,cnt))
	    }
	    incr(sum,val)
        }
        set(sum,div(sum,mul(365.0,length(alist))))
        set(min,div(min,365.0))
        set(max,div(max,365.0))

	if (dohist) {
	    /* generate histogram */
	    set(min,int(min))
	    set(hmax,0)
	    forlist(hist,val,cnt) {
		if (gt(val,hmax)) {
		    set(hmax,val)
		}
	    }
	   set(hincr,div(add(hmax,9),10))
	   set(cnt,10)
	   while(cnt) {
	      set(htar,add(mul(sub(cnt,1),hincr),1))
	      if (or(eq(cnt,10),eq(cnt,7),eq(cnt,4),eq(cnt,1))) {
		  print(fl(d(htar),5),"+")
	      } else {
		  print("     |")
	      }
	      forlist(hist,val,cnt1) {
		  if (ge(cnt1,min)) {
		     if (ge(val,htar)) { 
			if (eq(cnt1,int(sum))) {
			    print("*") 
			} else {
			    print("x") 
			}
		     } else {
		        print(" ") 
		     }
		  }
	      }
	      print (nl())

	      decr(cnt)
	    }
	    set(cnt1,min)
	    print( "     ")
	    while(lt(cnt1,max)) {
		incr(cnt1,5)
		print("+----")
	    }
	    print(nl())
	    print( "     ")
	    set(cnt1,sub(min,1))
	    while(lt(cnt1,max)) {
		print(fr(d(cnt1),4))
		incr(cnt1,5)
	    }
	    print("                        min    ave     max pairs keys of match",nl())
	}
        print(title," ",fl(f(min),7),fl(f(sum),7),fl(f(max),8))
        print(fl(d(length(alist)),6)," ",min_id," :: ",max_id,nl())
    } else {
	if (dohist) {
	    print("                        min    ave     max pairs keys of match",nl())
	}
        print(title," ",fl("-",7),fl("-",7),fl("-",8),nl())
    }
    print(nl())
}

/* fl(str,len)
 * insert spaces to right of str, to make it's length is at least len
 */
func fr(str,len) {
    if (lt(strlen(str),len)) {
       set(fil,sub(len,strlen(str)))
       incr(fil)
    } else {
       set(fil,1)
    }
    return(concat(str,substring("                                  ",1,fil)))
}

/* fl(str,len)
 * insert spaces to left of str, to make it's length at least len
 */
func fl(str,len) {
    if (lt(strlen(str),len)) {
       set(fil,sub(len,strlen(str)))
    } else {
       set(fil,0)
    }
    return(concat(substring("                                  ",1,fil),str))
}
func get_marriage_date(fam)
{
    if (m,marriage(fam)) {
       if (strlen(date(m))) {
	  if (estdate) {
	      if (index(date(m),"EST",1)) {
		  return(0)
	      }
	  }
	  extractdate(m,day,month,year)
	  if (year) {
	     return(julian(day,month,year))
	  }
       }
    }
    return(0)
}
func get_birth_date(indi)
{
    if (b,birth(indi)) {
       if (strlen(date(b))) {
	  if (estdate) {
	      if (index(date(b),"EST",1)) {
		  return(0)
	      }
	  }
	  extractdate(b,day,month,year)
	  if (year) {
	     return(julian(day,month,year))
          }
       }
    }
    return(0)
}
func get_death_date(indi)
{
    if (b,death(indi)) {
       if (strlen(date(b))) {
	  if (estdate) {
	      if (index(date(b),"EST",1)) {
		  return(0)
	      }
	  }
	  extractdate(b,day,month,year)
	  if (year) {
	     return(julian(day,month,year))
          }
       }
    }
    return(0)
}

/*
 * The first day that the Gregorian calendar was used in the British Empire 
 * was Sep 14, 1752.  The previous day was Sep 2, 1752
 * by the Julian Calendar.  The year began at March 25th before this date.
 * Computations not corrected for dates before Sep 14, 1752 nor necessarily
 * for other countries.
 */

func julian(day,mon,year) {
    if (gt(mon,2)) { set(mon,sub(mon,3)) }
    else { set(mon,add(mon,9)) decr(year) }

    set(c,div(year,100))
    set(ya, sub(year,mul(100,c)))
    set(jd, add( div(mul(146097,c),4),
	     add(div(mul(1461,ya),4),
		 add(div(add(mul(153,mon),2),5),
		     add(day, 1721119)))))
    /* for our usage this probably doesn't matter
    if (lt(jd,2361222)) {
       print("Warning, Attempt to compute date prior to Brittish use of Gregorian calendar\n")
    }
    */
    return(jd)
}
