/*
 * @progname       addsour
 * @version        1.0
 * @author         Stephen Dum
 * @category       
 * @output         Modifies Database
 * @description    

This script prompts for a message and adds the message along with todays date
as a Source record on each individual and Family in the database.  It checks
to see if the source already exists, and allows you to skip or replace an
existing source record.  Warning, this script modifies your database, making a
backup of your data before running this script is advised.
*/

option("explicitvars")

proc main()
{
    print("\n")
    print("This script will add a SOUR record to each indi and fam in your database\n")
    print("The value will be the message you supply with todays date appended.\n")
    print("Warning: This script modifies your database - backup your data\n",
          "before running it -- enter abort to abort\n\n")
    getstr(msg,"Enter Message to add to SOUR")
    if (index(lower(msg),"abort",1)) {
        return()
    }

    /* iterate thru each individual adding sources to end of each */
    forindi(indiv,cnt) {
	set(ok,"ok")
        fornodes(indiv,n) {
	   if (eqstr(tag(n),"SOUR")) {
	        if (index(value(n),msg,1)) {
		    print("Warning, ",key(indiv),": SOUR ",value(n),nl())
		    print("Message already exists in level 1 SOUR record",nl())
		    getstr(ok,"Press return to skip add, rep to replace,  else ok<cr>")
		    if (index(lower(ok),"rep",1)) {
			/* replace node */
			detachnode(n)
			set(n,nn)
		    }
		}
	   }
	   set(nn,n)
	}
	if (strlen(ok)) {
	    print("adding msg for ",key(indiv),nl())
	    set(s,createnode("SOUR",concat(msg," ",date(gettoday()))))
	    addnode(s,indiv,nn)
	    writeindi(indiv)
	}
    }
    forfam(fam,cnt) {
        fornodes(fam,n) {
	   if (eqstr(tag(n),"SOUR")) {
	        if (index(value(n),msg,1)) {
		    print("Warning, ",key(fam),": SOUR ",value(n),nl())
		    print("Message already exists in level 1 SOUR record",nl())
		    getstr(ok,"Press return to skip add, rep to replace,  else ok<cr>")
		    if (index(lower(ok),"rep",1)) {
			/* replace node */
			detachnode(n)
			set(n,nn)
		    }
		}
	   }
	   set(nn,n)
	}
	if (strlen(ok)) {
	    print("adding msg for ",key(fam),nl())
	    set(s,createnode("SOUR",concat(msg," ",date(gettoday()))))
	    addnode(s,fam,nn)
	    writefam(fam)
	}
    }
}
