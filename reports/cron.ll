/*
 * @progname       cron.ll
 * @version        2.0
 * @author         Stephen Dum
 * @category       
 * @output         HTML
 * @description    

Generate calendar of birth, death, marriage events arranged by the year, month 
and day that they occurred.  Generates a top level index by year, with actual
events stored in a separate html file for each decade.
Warning, this report requires lifelines version 3.0.27 or later.
Some properties must be set in your lifelines configuration file for this
report to run, see comments at beginning of the report for details.

         by Stephen Dum (stephen.dum@verizon.net)
         Version 1   March    2003  
         Version 2   November 2005

This program was inspired by similar efforts by Mitch Blank (mitch@ctrpnt.com)
but without ever seeing the code he used to do a similar thing.

Before using, there are a few properties that need to be customized for your
own environment so add them to your .linesrc ( or for windows lines.cfg) file.
The properties that are looked up are:
   user.fullname -- name of the database owner
   user.email -- email address of the db owner
   cron.htmldir -- path to the directory to store results in
                     e.g. /home/joe/genealogy/html
   cron.backgroundimage -- path to the background image, no image if not defined.
                 e.g. ../../image/crink.jpg
		 this places image at the same level as /home/joe/genealogy/html
*/

/* customization globals */
char_encoding("ASCII")
option("explicitvars")

global(base_filename)	/* where to store the results */
global(background)	/* path of background image relative to final html
			 * location, or "" */
global(hi_bg_color)	/* highlighted year background color */
global(lo_bg_color)	/* non-highlighted year background color */

global(db_owner)        /* name of database owner - from config file */
global(owner_email)     /* email of database owner - from config file */
global(privatize)       /* should we privatize the data */
global(withkey)         /* should we include key's in the output */
global(cutoff_year)     /* 100 years before today */
                        /* birth >= cutoff_year  is about 101 years,
			 * and we consider person living */

global(month_name)      /* names of the months */
global(events)          /* list of events to print */
global(dates)           /* list of dates of the events */

proc main ()
{
    /* initialization of globals */

    set(hi_bg_color,"\"#ddb99f\"")
    set(lo_bg_color,"\"#e5d3c5\"")

    set(db_owner, getproperty("user.fullname"))
    set(owner_email, concat("mailto:",getproperty("user.email")))
    set(background,getproperty("anniver.backgroundimage")) 
    set(base_filename,concat(getproperty("anniver.htmldir"),"/",database(),"/"))  
    if (not(test("d",base_filename))) {
        print("Error, property anniver.htmldir=",base_filename,
	      ", is not a directory,aborting\n")
	print("Please read comments at beginning of report about setting properties\n")
        return()
    }

    /* other globals*/
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

    extractdate(gettoday(),day,mon,cutoff_year)
    decr(cutoff_year,100)

    /* end of initialization of globals */

    getint(privatize,"Enter 1 to privitize data, 0 otherwise")
    getint(withkey,"Enter 1 to include keys, 0 otherwise")
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
	print("Computing descendants ")
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
    set(birth_type,0)
    if (birth,birth(indi)) {
	set(birth,get_date(birth))
	set(birth_type," born")
    } elsif (birth, baptism(indi)) {
	set(birth,get_date(birth))
	set(birth_type," baptized")
    }
    set(death_type,0)
    if (death,death(indi)) {
	set(death,get_date(death))
	set(death_type," died")
    } elsif (death, burial(indi)) {
	set(death,get_date(death))
        set(death_type," buried")
    }
    if (privatize) {
	/* skip confidential or living people */
	if (confidential(indi)) { return() }
	/* living - birth, no death, and birth < 101 years ago */
	if (and(birth,not(death))) {
	    if (ge(div(birth,10000),cutoff_year)) { return()}
	}
    }
    if (birth) {
	if (withkey) {
	    enqueue(events,concat(name(indi),"(",key(indi),")",birth_type))
	} else {
	    enqueue(events,concat(name(indi),birth_type))
	}
	enqueue(dates,birth)
    }
    if (death) {
	if (withkey) {
	    enqueue(events,concat(name(indi),"(",key(indi),")",death_type))
	} else {
	    enqueue(events,concat(name(indi),death_type))
	}
	enqueue(dates,death)
    }

    families(indi,famly, spouse, cnt) {
	if (privatize) {
	    /* skip confidential families */
	    if (confidential(famly)) { return() }
	}
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
		    set(marr,get_date(node))
		    if (marr) {
			if (withkey) {
			    enqueue(events,concat(name(indi),spo,
				    "(",key(indi),",",key(spouse),")"))
			} else {
			    enqueue(events,concat(name(indi),spo))
			}
			enqueue(dates,marr)
		    }
		}
	    }
	}
    }
}

/* get_date(node)
 * if event node has a date associated with it return it encoded as
 *           (year * 100 + month) * 100 + day
 *           These values facilitate sorting.
 */
func get_date(node)
{
    extractdate(node,day,mon,yr)
    if (yr) {
	return(add(mul(add(mul(yr,100),mon),100),day))
    }
    return(0)
}

func confidential(n)
{
    fornodes(n,node) {
        if (eqstr(tag(node),"RESN")) {
	    if (eqstr(value(node),"confidential")) {
	        return(1)
	    }
	}
    }
    return(0)
}
