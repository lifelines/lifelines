/*
 * @progname       search_source.ll
 * @version        1.0
 * @author         Stephen Dum
 * @category
 * @output         text
 * @description

Search source records for a particular string.
Program prompts for the type of sub record to search
and then for string to search for.  If no sub record type is
entered, all records are searched. Case is ignored in searches.

         by Stephen Dum (stephen.dum@verizon.net)
         Version 1   July     2006
*/

option("explicitvars")

proc main()
{
    getstr(search,"Enter type of SOUR sub records searched(e.g. TITL, AUTH) <return> for all")
    set(search,upper(search))
    getstr(match,"Enter string to search for")
    set(match,lower(match))

    forsour(n,i){
	if (strlen(search)) {
	    /* only search children of this source where tag is search */
	    fornodes(n,ch) {
		if (eqstr(tag(ch),search)) {
		    set(v,value(ch))
		    if (i,index(lower(v),match,1)) {
			/*
			print ("Found in ",xref(n)," ",v,"\n")
			*/
			print ("Found in ",xref(n),": ")
			print (d(level(ch))," ")
			if (strlen(xref(ch))) {
			    print (xref(ch)," ")
			}
			print(tag(ch)," ",lower(v),"\n")
		    }
		}
	    }
	} else {
	    traverse(n,ch,i) {
		set(v,value(ch))
		if (i,index(lower(v),match,1)) {
		    /*
		    print ("Found in ",xref(n)," ",v,"\n")
		    */
		    print ("Found in ",xref(n),": ")
		    print (d(level(ch))," ")
		    if (strlen(xref(ch))) {
			print (xref(ch)," ")
		    }
		    print(tag(ch)," ",lower(v),"\n")
		}
	    }
	}
    }
}
