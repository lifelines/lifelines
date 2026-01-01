/*
 * @progname    onthisday.ll
 * @version     1 of 2025-12-28
 * @author      Nicholas Strauss
 * @category
 * @output      Text
 * @description

Report lists people born, died on this day of year, marriages.
Useful for not neglecting relatives.
Now, you have no excuse.

This program works only with LifeLines.

*/
proc main ()
{
        indiset(idx)
        getindiset(idx)
        dayformat(0)
        monthformat(4)
        dateformat(0)
        set(tday, gettoday())
        extractdate(tday, td, tm, ty)
	  /*	  set( td , 10)
		  set( tm, 7)*/

        col(1) "\nOn this day Report by:       N. Strauss\n"
        col(1) "                         1 Bitterfield Ct.\n"
        col(1) "                         Ballwin, MO 63011\n"
        col(1) "        Date:            "    stddate(tday) "\n\n"

        col(1) "This report d=" d(td) "m= " d(tm) "y= " d(ty) "\n\n"

        /* Iterate over whole database */
        forindi (indi, num) {

	  /* if birthday recorded for individual */
	  /* Extract birthday for individual */
	  if (bth, birth(indi)) {
              extractdate(bth, birthday, birthmonth, birthyear)
		if (and(eq(birthday, td), eq(birthmonth, tm))){
         col(1) name(indi) "birthday d=" d(birthday) "m= " d(birthmonth) "y= " d(birthyear) "\n\n"
		}
	   }
	  /* if deathday recorded for individual */
	  /* Extract deathday for individual */
	  if (dth, death(indi)) {
              extractdate(dth, deathday, deathmonth, deathyear)
		if (and(eq(deathday, td), eq(deathmonth, tm))){
        col(1) name(indi) "death d=" d(deathday) "m= " d(deathmonth) "y= " d(deathyear) "\n\n"
		}
	   }
	}

	forfam(family,fnum) {
	  if (evt, marriage(family)){
	    if (date(evt)){
	      set(he, husband(family))
	      set(she, wife(family))
	      extractdate(evt, marrday, marrmonth, marryear)
	      if (and(eq(marrday, td), eq(marrmonth, tm))){
		col(1) " marriage "  name(he) " " name(she) " " stddate(evt)
		  /* col(1) "marr d=" d(marrday) "m= " d(marrmonth) "y= " d(marryear) "\n\n"*/
		  }
	    }
	  }
	}
}


