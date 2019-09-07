/*
 * @progname       anniver.ll
 * @version        4.0
 * @author         Stephen Dum
 * @category       
 * @output         HTML
 * @description    

Generate calendar of birth, death, marriage events arranged by the month 
and day that they occurred.  Generates a top level index calendar, with actual
events stored in a separate html file for each month.
Some properties must be set in your lifelines configuration file for this
report to run, see comments at beginning of the report for details.

Warning, this report requires lifelines version 3.0.50 or later.

         by Stephen Dum (stephen.dum@verizon.net)
         Version 1   March    2003  
         Version 2   November 2005 Support privitizing data
         Version 3   December 2005 Do html char set encoding
         Version 4   June 2006 incorporated mods by Dave Eaton (dwe@arde.com) May 2006

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

Before using, there are a few properties that need to be customized for your
own environment so add them to your .linesrc ( or for windows lines.cfg) file.
You can also set them on the command line (like -Ianniver.htmldir=/tmp/foo)
The properties that are looked up are:
   user.fullname -- name of the database owner
   user.email -- email address of the db owner
   anniver.htmldir -- path to the directory to store results in
                     e.g. /home/joe/genealogy/html
                     (program expects a subdir in this directory with the name
                     of the database in it.)
   anniver.backgroundimage -- path to the background image,
                 no image if not defined.
                 e.g. ../../image/crink.jpg
                 this places image at the same level as /home/joe/genealogy/html
   privatization:  This report respects 2 levels of privatization
       1. if a record "RESN confidential" exists on an individual they are
          skipped (as this report is designed to be shared, this seems
          like a reasonable default)
       2. skip anyone estimated to be living

   History.
      Version 2 Add code to allow respecting privatized data.
      Version 3 switch from baptism() to get_baptism() for wider coverage
                use translation tables to convert data to properly 
                escaped html.  This is very codeset dependent.
     Version 4   added changes by Dave Eaton (dwe@arde.com)
         Added "firstyear" that events may be on the calendar
         Added "includedeath" check to drop deaths if those are not desired
         Added ability to generate report for descendants of more than one
             individual
         Added ability to generate report only for living people 
             (omitting confidential if desired)
*/

/* customization globals */
char_encoding("ASCII")
option("explicitvars")

global(base_filename)   /* where to store the results */
global(background)      /* path of background image relative to final html
                         * location, or "" */
global(hi_bg_color)     /* highlighted year background color */
global(lo_bg_color)     /* non-highlighted year background color */

global(db_owner)        /* name of database owner - from config file */
global(owner_email)     /* email of database owner - from config file */
global(justliving)      /* should we generate a report only for living people? */
global(privatize)       /* should we privatize the data 
                         * 0 = display all data
                         * 1 = skip confidential records
                         * 2 = skip confidential and living
                         */
global(withkey)         /* should we include key's in the output */
global(cutoff_year)     /* 100 years before today */
                        /* birth >= cutoff_year  is about 101 years,
                         * and we consider person living */

global(firstyear)       /* earliest year for which entries should be included */
global(includedeath)    /* if set, then include the death events on the calendar */

global(month_name)      /* names of the months */
global(events)          /* list of events to print */
global(dates)           /* list of dates of the events */
global(keynames)        /* name(s) of the key individuals for this report */

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
    set(cs,getproperty("codeset"))
    if (eqstr(cs,"UTF-8")) {
        set(srccs,"UTF-8")
        set(dstcs,"UTF-8//html")
    } elsif (eqstr(cs,"ISO-8859-15")) {
        set(srccs,"ISO-8859-15//html")
        set(dstcs,"UTF-8")
    } else {
        print("\nDatabase codeset ",cs," not supported, exiting\n")
    }

    /* end of initialization of globals */

    getint(justliving,"Enter 1 to include only living people, 0 otherwise")
    if (justliving) {
        /* Default the choices which conflict with "justliving" */
        set(includedeath,0)
        /* We want living people, so see if we also want confidental */
        getint(noconfidential,"Enter 1 to omit confidential living people, 0 otherwise")
        if (noconfidential) {
            set(privatize,1)
        } else {
            set(privatize,0)
        }
    } else {
        getint(privatize,"\nPrivatization: 0 print all data; 1 skip confidential records; 2 skip confidential and living")
        getint(includedeath,"Enter 1 to include deaths on calendar, 0 otherwise")
    }
    getint(withkey,"Enter 1 to include keys, 0 otherwise")
    getint(firstyear,"Enter oldest year to be on calendar, 0 for no limit")
    getindi(person,"Enter person for whom to find descendants (return for all)")
    indiset(thisgen)
    indiset(allgen)
    list(events)
    list(dates)
    list(keynames)
    set(firstpass,1)
    /* if a person is entered, the generated list of people include
     * person and spouse, and all the children of either
     * and then recursively the people, their spouses and all the children
     * thereof
    */
    if (person) {
        while (person) {
            addtoset(thisgen, person, 0)
            addtoset(allgen, person, 0)
            print("Computing descendants of ", name(person), " ")
            enqueue(keynames,concat(name(person)))
            set(thisgensize,1)
            set(gen,neg(1))
            while(thisgensize) {
                set(gen,add(gen,1))
                print("adding ",d(thisgensize)," individuals for generation ",d(gen),"\n")
                indiset(spouse)
                set(spouse,spouseset(thisgen))
                set(thisgen,childset(union(thisgen,spouse)))
                set(allgen,union(allgen,spouse))
                set(allgen,union(allgen,thisgen))
                set(thisgensize,length(thisgen))
                /* the following check prevents looping if the
                 * database has been corrupted and a parent is listed
                 * as a child of that parent, and diagnoses the fault
                 */
                if (eq(length(intersect(allgen,thisgen)),thisgensize)) {
                    set(thisgensize,0)
                    print("Warning child is listed as its own parent\n")
                    forindiset(thisgen,indi,val,i) {
                       print (name(indi)," ")
                    }
                    print("\n")
                }
            }
            if (firstpass) {
                print ("Total of ")
                set(firstpass,0)
            } else {
                print ("New total of ")
            }
            print (d(length(allgen))," individuals",nl())
            getindi(person,"Enter next person for whom to find descendants")
        }
        /* now generate list of events */
        forindiset(allgen,indi,val,i) {
            if (not(mod(i,100))) {
                print(".")
            }
            call add_indi(indi)
        }
        print("\n")
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
    
    /* Now print out all the data for each month
     */
    print(nl())

    list(daymask)
    set(dm_day,1)       /* last day dealt with */
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
        "<td><font size=4>"
        if (srccs) {
            convertcode(event,srccs,dstcs)
        } else {
            event
        }
        "</font></td>" nl()
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
    "<p>This calendar of anniversary dates lists events" nl()
    if (firstyear) {
        "since " d(firstyear) nl()
    }
    if (justliving) {
        "of living people" nl()
    }
    "arranged by the" nl()
    "month and day that they occurred." nl()
    if (not(includedeath)) {
        "<!-- Diagnostic: death events not included -->" nl()
    }
    if (length(keynames)) {
        "<br>Events listed are for descendants of:" nl()
        while(length(keynames)) {
                set(nxtname,pop(keynames))
                set(nameout,length(keynames))
                nxtname
                if (nameout) {
                        ","
                } else {
                        "."
                }
                nl()
        }
    }
    "</p>" nl()
    "<p>Click on the month name or any highlighted day to see the events" nl()
    "for that time.</p>" nl()
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
        
            "<tr>" nl() /* start a row in the table */
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
                if (ne(k,3)) {  /* add separator between months */
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

  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE html public \"-//W3C//DTD HTML 4.01 Transitional//EN\" >\n"
  "<html>\n<head>\n"
   "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n"
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
  "<!- generated by lifelines (https://lifelines.github.io/lifelines/) with a" nl()
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
    } elsif (birth, get_baptism(indi)) {
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
    /* skip confidential records and living people */
    if (privatize) {
        if (confidential(indi)) { return() }

        /* living - birth, no death, and birth < 101 years ago */
        if (and(ge(privatize,2),birth,not(death))) {
            if (ge(mod(birth,10000),cutoff_year)) { return()}
        }
    }
    if (birth) {
        /* Make certain that if we only want living people that this is 
           (or at least may be) */
        if (not(or(and(justliving,death),and(justliving,lt(mod(birth,10000),cutoff_year))))) {
                if (withkey) {
                    enqueue(events,concat(name(indi),"(",key(indi),")",birth_type))
                } else {
                    enqueue(events,concat(name(indi),birth_type))
                }
                enqueue(dates,birth)
        }
    }
    if (and(includedeath,death)) {
        if (withkey) {
                enqueue(events,concat(name(indi),"(",key(indi),")",death_type))
        } else {
                enqueue(events,concat(name(indi),death_type))
        }
        enqueue(dates,death)
    }

    families(indi,famly, spouse, cnt) {
        /* skip confidential families */
        if (confidential(famly)) { continue() }
        if (and(privatize,spouse)) {
            if (confidential(spouse)) { continue() }
        }
        if (justliving) {
            /* make sure the person is living: no death, birth and 
               birth < 101 years ago */
            /* Nope, we know they have died */
            if (death) { return() }
            if (birth) {
                /* Nope, estimated they would be too old now */
                if (lt(mod(birth,10000),cutoff_year)) { return() }
            }
        }
        /* living - birth, no death, and birth < 101 years ago */
        if (ge(privatize,2)) {
            if (and(birth(spouse),not(death(spouse)))) {
                if (ge(mod(get_date(birth(spouse)),10000),cutoff_year)) { continue()}
            }
        }
        /* to avoid duplication, only enter data 
         * if indi is male, or there is no spouse
         */
        if (or(male(indi),not(spouse))) {
            fornodes(fnode(famly), node) {
                if(eqstr(tag(node),"MARR")) {
                    if (spouse) {
                        set(names,concat(name(indi)," and ",name(spouse)))
                        set(keys,concat("(",key(indi),",",key(spouse),")"))
                    } else {
                        set(names,name(indi))
                        set(keys,concat("(",key(indi),")"))
                    }
                    set(marr,get_date(node))
                    if (marr) {
                        /* Make sure date is plausible for living or that we don't care */
                        if (or(not(justliving),ge(mod(marr,10000),cutoff_year))) {
                            if (withkey) {
                                enqueue(events,concat(names,keys," married"))
                            } else {
                                enqueue(events,concat(names," married"))
                            }
                            enqueue(dates,marr)
                        }
                    }
                }
            }
        }
    }
}

/* get_date(node)
 * if event node has a date associated with it return it encoded as
 *           (mon * 100 + day) * 10000 + yr
 *           These values facilitate sorting.
 */
func get_date(node)
{
    extractdate(node,day,mon,yr)
    if (mon) {
        if (ge(yr,firstyear)) {
                return(add(mul(add(mul(mon,100),day),10000),yr))
        } else {
        /* Nope, this one should not be on the calendar */
                return(0)
        }
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
func get_baptism(ind)
{
    fornodes(ind,node) {
        if (index(" BAPM BAPL CHR CHRA ",concat(" ",upper(tag(node))," "),1)) {
            return(node)
        }
    }
    return(0)
}
