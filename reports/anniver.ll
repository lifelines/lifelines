/*
 * @progname       anniver
 * @version        1.0
 * @author         Stephen Dum
 * @category       
 * @output         html
 * @description    

Generate calendar of birth, death, marriage events arranged by the month 
and day that they occured.  Generates a top level index calendar, with actual
events stored in a separate html file for each month.
Warning, this report requires lifelines version 3.0.27 or later.

         by Stephen Dum (steve_dum@mentorg.com)
         Version 1,  March 2003  

This program was inspired by similar efforts by Mitch Blank (mitch@ctrpnt.com)
but without ever seeing the code he used to do a similar thing.

Originally this program used getel and setel to access the dates and events
lists and to sort them.  It ran about 400 seconds on 11850 element lists.
Conversations between myself and Perry Rapp about sorting the large lists
created by this program led to the sort and rsort functions being added to
the report language.  This program uses them.  Also care was taken to avoid
using getel or setel functions on the dates and events lists as random access
to very large lists is very slow.  With these changes run time dropped to 10
seconds. 

Before using, there are a few variables that need to be customized for your
own environment.  the changes need to be made in set_static_html_globals() 
below. Note: name and email address are obtained with getproperty() so add
them to your .linesrc ( or for windows lines.cfg) file.

By default, program tries to write data files in the directory 
./html/<name of database> -- you must create this directory before running
the program.
*/

/* customization globals - modify values in set_static_html_globals below */
char_encoding("ASCII")
options(explicitvars)

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
    set(db_owner, getproperty("user.name"))
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
    sort(events,dates)
    
    /* Now print out all the data for each month
     */
    print(nl())

    list(daymask)
    set(dm_day,1)	/* last day dealt with */
    set(mask,1)         /* mask for this day */
    set(lastday,-1)
    set(lastmonth,-1)
    set(in_day,0)
    while(length(dates)) {
        set(val,pop(dates))
	set(event,pop(events))
	set(month,div(val,1000000))
	set(year,mod(val,1000000))
	set(day,div(year,10000))
	set(year,mod(val,10000))

	if (ne(lastmonth,month)) {
	    if (ne(lastmonth,-1)) {
		if (in_day) {
		    "</table>\n"
		}
		call write_tail()
                setel(daymask,lastmonth,dm)
	    }
	    set(lastday,-1)
	    set(dm,0)
	    set(dm_day,1)
	    set(mask,1)
	    set(m_name, getel(month_name,month)) 
	    call openfile(lower(m_name),concat(m_name," Anniversary Dates"))
	    set(lastmonth,month)
	}
	if (ne(lastday,day)) {
	    if (ne(lastday,-1)) {
		if (in_day) {
		    "</table>\n"
		}
	    }
	    if (lastday,day) {
		while(lt(dm_day,day)) {
		    incr(dm_day)
		    set(mask,add(mask,mask))
		}
		set(dm,add(dm,mask))
		"<p><a name=" d(day) "><B>" m_name " " d(day) "</b></a>\n"
		"<table>\n"
	    } else {
		/* don't know day, so just generic month */
		"<p><B>" m_name "</b>\n"
		"<table>\n"
	    }
	    set(in_day,1)
	}
	"<tr>\n<td width=\"70\" valign=top align=center><font size=4><b>"
	if (year) {
	    d(year)
	} else {
	    "&nbsp;"
	}
	"</b></font>" nl()
	"<td><font size=4>" event "</font></td>" nl()
    }
    if (ne(lastmonth,-1)) {
	if (in_day) {
	    "</table>\n"
	}
	call write_tail()
	setel(daymask,lastmonth,dm)
    }

    /* Now print out the calendar page indexing the individual month files */

    /* debug print out month masks
    set(i,1)
    while(le(i,12)) {
        set(dm,getel(daymask,i))
	print( "Month ",d(i),"  ")
	while(dm) {
	    print( d(mod(dm,2)))
	    set(dm,div(dm,2))
	}
	print(nl())
	incr(i)
    }
    */

    call openfile("annver","Calendar of Anniversary Dates")
    "This calendar of anniversary dates lists events arranged by the" nl()
    "month and day that they occured." nl()
    "<P>Click on the month name or any highlighed day to see the events" nl()
    "for that time." nl()
    "<hr>" nl()
    "<table border=4 width=\"99%\">" nl()

    /* The calendar is arranged with 4 months across
     * so we need to process 4 months at a time */
    list(month_len)
    enqueue(month_len,31)
    enqueue(month_len,29)
    enqueue(month_len,31)
    enqueue(month_len,30)
    enqueue(month_len,31)
    enqueue(month_len,30)
    enqueue(month_len,31)
    enqueue(month_len,31)
    enqueue(month_len,30)
    enqueue(month_len,31)
    enqueue(month_len,30)
    enqueue(month_len,31)
    list(inds)
    set(i,0)   /* i iterates over 3 chunks of 4 months */
    while(le(i,2)) {
	/* generate the headings */
	"<tr>" nl()
	set(j,1)
	while(le(j,4)) {
	    "<td bgcolor=" hi_bg_color 
	    " colspan=7 align=center><font size=5>" nl()
	    set(m_name,getel(month_name,add(mul(i,4),j)))
	    "<a href=\"" lower(m_name) ".html\">" m_name  "</a>" nl()
	    "</font></td>" nl()
	    "<td></td>" nl()
	    incr(j)
	}
	"</tr>" nl()

	/* now compute the starting indexes for each month */

	set(wk,0)
	while(le(wk,4)) { /* for each of the 5 weeks in the months */
	
	    "<tr>" nl()	/* start a row in the table */
	    set(k,0)
	    while(le(k,3)) { /* for each of the 4 months in this line */
		set(mon,add(mul(i,4),k,1))
		set(m_name,getel(month_name,mon))
		set(m_len,getel(month_len,mon))
		set(ind,getel(daymask,mon))
		set(day,add(mul(wk,7),1))

		set(l,1)
		while(le(l,7)) { /* for each of the 7 days in a week  */

		    /* do a day */
		    if (gt(day,m_len)) {
			/* empty square */
			"<td></td>" nl()
		    } elsif(mod(ind,2)) {
			/* linked square */
			"<td bgcolor=" hi_bg_color 
			" align=center><font size=4><a href=\""
			lower(m_name) ".html#" d(day) "\">"
			d(day) "</a>" nl()
			"</font></td>" nl()
		    } else {
			/* output transparent number */
			"<td bgcolor=" lo_bg_color 
			" align=center><font size=4>"
			d(day) "</font></td>" nl()
		    }
		    incr(day)
		    incr(l)
		    set(ind,div(ind,2))
		}
		if (ne(k,3)) {  /* add separater between months */
		    "<td></td>" nl()
		}
		setel(daymask,mon,ind) /* save away latest day mask */
		incr(k)
	    }
	    "</tr>" nl()
	    incr(wk)
	}
	"<tr><td colspan=31></td></tr>" nl()
	incr(i)
    }
    "</table>" nl()
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
 * dates   - contains the date the event occured the date is stored
 *           as a decimal number for sorting
 *           (mon * 100 + day) * 10000 + yr
 */
proc add_event(node,description) {
    extractdate(node,day,mon,yr)
    if (mon) {
	set(dt,add(mul(add(mul(mon,100),day),10000),yr))
	enqueue(events,description)
	enqueue(dates,dt)
    }
}
