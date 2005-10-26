/*
 * @progname       cron.ll
 * @version        1.0
 * @author         Stephen Dum
 * @category       
 * @output         HTML
 * @description    

Generate calendar of birth, death, marriage events arranged by the year, month 
and day that they occurred.  Generates a top level index by year, with actual
events stored in a separate html file for each decade.
Warning, this report requires lifelines version 3.0.27 or later.

         by Stephen Dum (stephen.dum@verizon.net)
         Version 1,  March 2003  

This program was inspired by similar efforts by Mitch Blank (mitch@ctrpnt.com)
but without ever seeing the code he used to do a similar thing.

Before using, there are a few variables that need to be customized for your
own environment.  The changes need to be made in set_static_html_globals() 
below. Note: name and email address are obtained with getproperty() so add
them to your .linesrc (or for windows lines.cfg) file.

By default, program tries to write data files in the directory 
./html/<name of database> -- you must create this directory before running
the program.
*/

/* customization globals - modify values in set_static_html_globals below */
char_encoding("ASCII")
options("explicitvars")
global(base_filename)	/* where to store the results */
global(background)	/* path of background image relative to final html
			 * location, or "" */
global(hi_bg_color)	/* highlighted year background color */
global(lo_bg_color)	/* non-highlighted year background color */
global(db_owner)
global(owner_email)
global(events)
global(dates)
global(month_name)

proc set_static_html_globals() {
    /* customize these globals to customize the output to your taste */
    set(background,"../images/crink2.jpg")  /* set to "" if you have none */
    set(hi_bg_color,"\"#ddb99f\"")
    set(lo_bg_color,"\"#e5d3c5\"")
    set(base_filename, concat("./html/",database(),"/"))  

    /* other globals*/
    set(db_owner, getproperty("user.fullname"))
    set(owner_email, concat("mailto:",getproperty("user.email")))
    list(month_name)
    enqueue(month_name,"January")
    enqueue(month_name,"February")
    enqueue(month_name,"March")
    enqueue(month_name,"April")
    enqueue(month_name,"May")
    enqueue(month_name,"June")
    enqueue(month_name,"July")
    enqueue(month_name,"August")
    enqueue(month_name,"September")
    enqueue(month_name,"October")
    enqueue(month_name,"November")
    enqueue(month_name,"December")
}

/* end of customization globals - customize values assigned in main */

proc main ()
{
    call set_static_html_globals()
    getindi(person,"Enter person to find descendants of (return for all)")
    indiset(thisgen)
    indiset(allgen)
    list(events)
    list(dates)
    /* if a person is entered, the generated list of people include
     * person and spouse, and all the children of either
     * and then recursively the people, their spouses and all the children
     * thereof
    */
    if (person) {
	addtoset(thisgen, person, 0)
	addtoset(allgen, person, 0)
	print("Computing descendents ")
	set(thisgensize,1)
	set(gen,neg(1))
	while(thisgensize) {
	    set(gen,add(gen,1))
	    print(d(gen)," ")
	    indiset(spouse)
	    set(spouse,spouseset(thisgen))
	    set(thisgen,childset(union(thisgen,spouse)))
	    set(allgen,union(allgen,spouse))
	    set(allgen,union(allgen,thisgen))
	    set(thisgensize,lengthset(thisgen))
	}
	print (nl(), "Total of ",d(lengthset(allgen))," individuals",nl())
	forindiset(allgen,indi,val,i) {
	    if (not(mod(i,100))) {
	        print(".")
	    }
	    call add_indi(indi)
	}
    } else {
	print("Traversing all individuals ")
        forindi (indi, val) {
	    if (not(mod(val,100))) {
	        print(".")
	    }
	    call add_indi(indi)
	    set(max,val)
	}
	print (nl(), "Total of ",d(max)," individuals",nl())
    }
    print( d(length(dates))," events generated",nl())
    
    print("sorting data")
    rsort(events,dates)
    
    /* Now print out all the data for each year
     */
    print(nl())

    list(yearmask)
    set(lastyear,-1)
    set(lastmonth,-1)
    set(lastdecade,-1)
    set(in_year,0)
    print( d(length(dates))," events",nl()) 
    while(length(dates)) {
        set(val,pop(dates))
	set(event,pop(events))
	set(year,div(val,10000))
	set(mon, mod(val,10000))
	set(day, mod(mon,100))
	set(mon, div(mon,100))
	set(decade, div(year,10))
	/* print(d(mon),"/",d(day),"/",d(year)," ", event, nl())  debug */

	if (ne(lastdecade,decade)) {
	    if (ne(lastdecade,-1)) {
		if (in_year) {
		    "</table>\n"
		    set(in_year,0)
		}
		call write_tail()
	    }
	    call openfile(concat("dec",d(decade)),concat(d(mul(decade,10)),
	                 " - ",d(add(mul(decade,10),9))," Events"))
	    set(lastdecade,decade)
	}
	if (ne(lastyear,year)) {
	    if (in_year) {
		"</table>\n"
	    }
	    "<p><a name=" d(year) "><h2>" d(year) "</h2></a>" nl()
	    push(yearmask,year)
	    "<table>" nl()
	    set(in_year,1)
	    set(lastyear,year)
	}
	if (mon) {
	    if (day) {
	        set(title,concat(getel(month_name,mon)," ",d(day)))
	    } else {
	        set(title,getel(month_name,mon))
	    }
	} else {
	    if (day) {
	        set(title,d(day))
	    } else {
	        set(title,"")
	    }
	}
	"<tr>\n<td width=\"150\" valign=top align=left>"
	"<font size=4><b>" title "</b></font>\n"
	"<td><font size=4>" event "</font></td>\n"
    }
    if (in_year) {
	"</table>\n"
    }
    call write_tail()

    /* Now print out the index page */

    call openfile("cron","Chronological Event Calendar")
    "This calendar indexes events by the year in which they occurred.\n"
	"<P>Click on the year to see the events that occurred in that year.\n"
	"<hr>\n"
	"<table border=4 width=\"99%\">\n"

    /* The calendar is arranged with 10 years across. */

    set(decade,div(getel(yearmask,1),10))
    set(cur_year,mul(decade,10))
    set(lastdecade,div(getel(yearmask,0),10))
    set(minyear,getel(yearmask,1))
    set(maxyear,getel(yearmask,0))
    forlist(yearmask,this_year,cnt) {
	while(le(cur_year,this_year)) {
	    if (eq(mod(cur_year,10),0)) {
		/* print("processing decade ",d(decade),nl()) / * debug */
		"<tr>" nl()
	    }
	    if (eq(cur_year,this_year)) {
	        /* print year with a link to year page */
		"<td bgcolor=" hi_bg_color
		" align=center><font size=4><a href=\""
                "dec" d(decade) ".html#" d(cur_year) "\">" 
		d(cur_year) "</a>\n</font></td>\n"
	    } else {
		if (or(lt(cur_year,minyear),gt(cur_year,maxyear))) {
		    /* blank out year */
		    "<td bgcolor=" lo_bg_color
		    " align=center><font size=4></font></td>" nl()
		} else {
		    /* print year without a link to year page */
		    "<td bgcolor=" lo_bg_color
                    " align=center><font size=4>" d(cur_year) "</font></td>" nl()
		}
	    }
	    incr(cur_year)
	    if (eq(mod(cur_year,10),0)) {
		"</tr>" nl()
		incr(decade)
	    }
	}
    }

    "</table>\n"
    call write_tail()
}

/* openfile(filename, title_to_use)
 * open output file and write out header information
 */
proc openfile(name,title) {
  set(filename, concat(base_filename,name,".html"))
  print("Writing ", filename, "\n")
  newfile(filename, 0)

  "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
  "<!DOCTYPE html public \"-//W3C//DTD HTML 4.01 Transitional//EN\" >\n"
  "<html>\n<head>\n"
  "<title> " title " </title>\n"
  "<style type=\"text/css\">\n"
  "p.hindent { margin-top: 0.2em; margin-bottom:0em;\n"
  "            text-indent: -2em; padding-left: 2em;}\n"
  "p.indent { margin-top: 0.2em; margin-bottom:0em;\n"
  "            text-indent: 0em; padding-left: 2em;}\n"
  "</style>\n"
  "</head>\n"
  if (eqstr(background,"")) {
      "<body bgcolor=" lo_bg_color ">\n"
  } else {
      "<body bgcolor=" lo_bg_color " background=\"" background "\">\n"
  }
  "<center><h1>" title "</h1></center>\n<hr>\n"
}

/* write_tail()
 * write out common footer information for file.
 */
proc write_tail() {
  "<br><hr><address>\n"
  monthformat(6)
  "This page was created " stddate(gettoday())
  "<br>\n"
  "Database maintained by "
  "<a href=\"" owner_email "\">\n"
  db_owner
  "</a></address>\n"
  "<!- generated by lifelines (http://lifelines.sourceforge.net/) with a" nl()
  "script written by Stephen Dum -->" nl()

  "</body></html>\n"
}

/* add_indi(individual)
 * check a given individual and see if there are any events to add
 * at the moment we do birth, death and marriage events.
 * Additional events can be added here
 */
proc add_indi(indi) {
    if (e,birth(indi)) {
	call add_event(e,concat(name(indi)," born"))
    } else {
	if (e, baptism(indi)) {
	    call add_event(e,concat(name(indi)," baptized"))
	}
    }
    if (e,death(indi)) {
	call add_event(e,concat(name(indi)," died"))
    } else {
       if (e, burial(indi)) {
	   call add_event(e,concat(name(indi)," buried"))
       }
    } 
    families(indi,famly, spouse, cnt) {
	/* to avoid duplication, only enter data 
	 * if indi is male, or there is no spouse
	 */
	if (or(male(indi),not(spouse))) {
	    fornodes(fnode(famly), node) {
		if(eqstr(tag(node),"MARR")) {
		    if (spouse) {
			set(spo,concat(" and ",name(spouse)," married"))
		    } else {
			set(spo," married")
		    }
		    call add_event(node,concat(name(indi),spo))
		}
	    }
	}
    }
}

/* add another event to the appropriate lists
 * events  - contains the description of the event
 * dates   - contains the date the event occurred the date is stored
 *           as a decimal number for sorting
 *           (year * 100 + month) * 100 + day
 */
proc add_event(node,description) {
    extractdate(node,day,mon,yr)
    if (yr) {
	set(dt,add(mul(add(mul(yr,100),mon),100),day))
	enqueue(events,description)
	enqueue(dates,dt)
    }
}
