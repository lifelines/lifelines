/*
 * @progname lldb.ll
 * @version  1.02
 * @author   Marc Nozell <marc@nozell.com>
 * @category palmpilot
 * @output   pdb import files
 * @description
 *
 *   This program produces a report of all INDI's in the database, with
 *   sorted names as output for inport into Tom Dyas' Open Source DB
 *   PalmOS app.  
 *
 *   lldb.ll V1.02
 *
 *   Marc Nozell <marc@nozell.com>
 *
 *   This report generator works only with the LifeLines Genealogy program
 *
 *   It will produce a report of all INDI's in the database, with
 *   sorted names as output for inport into Tom Dyas' Open Source DB
 *   PalmOS app.  
 * 
 * 0) Obtain Tom Wetmore's LifeLines genealogy program for Unix.
 *    See https://lifelines.github.io/lifelines/
 * 
 * 1) Obtain Kenneth Albanowski's <kjahds@kjahds.com> pilot-link
 *    package.  The Microsoft Windows-based Palm desktop should also
 *    work.  Most Linux distributions include pilot-link (check
 *    http://rpmfind.net) and should build on most UNIXes.
 *
 * 2) Obtain Tom Dyas' <tdyas@vger.rutgers.edu> "DB: Open Source
 *    Database Program for PalmOS" and supporting tools from
 *    http://pilot-db.sourceforge.net/
 *
 * 3) Run this lifelines report.  It will generate two files, lldb.info
 *    and lldb.csv.
 *    
 * 5) Run the CSV to PDB conversion tool like this:
 *            csv2pdb --info=lldb.info lldb.csv lldb.pdb
 *    
 * 5) Install the converted info to the Palm device like this:
 *            pilot-link -i ll.pdb
 * 
 * 
 *   V1.00					  11-Sep-1999
 *   Initial Version
 *
 *   V1.01					  26-Oct-1999
 * 
 *   Cleaned up output files
 *   Updated to new version Dyas' conversion tool
 *	(pre-palm-db-tools-0.2.0.tar.gz)
 *
 *   V1.02					  10-Nov-2000
 *   Updated URLs
 * 
 *  Revision 1.7  2004/07/19 05:54:55  dr_doom
 *  Merge Vincent Broman Changes to reports
 *
 *  Revision 1.6  2003/01/19 02:50:23  dr_doom
 *  move 1 paragraph description to immediately before @description  for index.html
 *
 *  Revision 1.5  2000/11/28 21:39:45  nozell
 *  Add keyword tags to all reports
 *  Extend the report script menu to display script output format
 *
 *  Revision 1.4  2000/11/27 20:48:15  nozell
 *  Header is to verbose, use just Log
 *
 *  Revision 1.3  2000/11/27 20:46:26  nozell
 *  Typo in CVS header
 *
 *  Revision 1.2  2000/11/27 20:45:43  nozell
 *  Add CVS keywords
 *
 * 
 */

proc main ()
{
/*   newfile (concat (database (), ".info"), 0) */

   newfile (concat ("lldb.info"), 0)
	
   "title \"Genealogy\"\n"
   "backup off\n"
   "find on\n"
   "extended off\n"
   "field \"ID\" string 25\n"
   "field \"Name\" string 80\n"
   "field \"Birth\" string 80\n"
   "field \"Death\" string 80\n"
   "field \"SpouseID\" string 80\n"
   "field \"ChildrenID\" string 80\n"
   "field \"FatherID\" string 80\n"
   "field \"MotherID\" string 80\n"

/*   newfile (concat (database (), ".csv"), 0) */

  newfile (concat ("lldb.csv"), 0)

  indiset(idx)

    /*	monthformat(4) */

    /* Grab them all */
    print("Please wait...")
    forindi(indi,n) {  
        addtoset(idx,indi,n)
    }
    print(nl()) print("Found ") print(d(n)) print(" people.")
    print(nl())
    print("begin sorting") print(nl())
    namesort(idx)
    print("done sorting") print(nl())

/*    col(1) "ID,Name,Birth,Death,SpouseID,ChildrenID,FatherID,MotherID" nl() */

    forindiset(idx,indi,v,n) {
        col(1) "\"" key(indi) "\""
	","
	"\""fullname(indi,1,0,30) "\""
	","

        call showvitals(indi)
	call showspouse(indi)
	call showkids(indi)
	call showparents(indi)
        print("+") 
	}

	nl()
	print(nl())
}

/************************************************************************/
proc showvitals (i)
{
    set(b, birth(i))
    set(d, death(i))
    if (and(b, short(b))) {
        "\"" long(b) "\""
    }
    else {
        "\" \"" 
    }

    ","

    if (and(d, short(d))) {
        "\"" long(d) "\"" 
    }
    else {
        "\" \"" 
    }
}

proc showparents (i)
{
    ",\""
    if(fath,father(i)) {
/*        "(" key(fath) ") " */
        key(fath)
    }
    else {
        "-unknown-"
    }

    "\""

    ",\""

    if(moth,mother(i)) {
/*         "(" key(moth) ") " */
        key(moth)
    }
    else {
        "-unknown-"
    }
    "\""
}
/************************************************************************/
proc showspouse (i) {
    ",\""
    if (eq(1, nspouses(i))) {
        spouses(i, s, f, n) {
            name(s) "(" key(s) ") "
        }
    }
    else {
        spouses(i, s, f, n) {
        ord(n) /* First, Second ... */
        " " name(s) "(" key(s) ") "
        }
    }
    "\""
}

/************************************************************************/
proc showkids (i) {
    ",\""
    set(j, 0)
    families(i, f, s, n) {
        set(j, add(j, nchildren(f)))
    }

    if (eq(0, j)) {
        " "
    }
    else {
        if (eq(1, j)) {
            "Child: "  
        }
        else {
            d(j) " Children:"  
        }
        set(j, 1)
        families(i, f, s, n) {
            children(f, c, m) {
               " (" key(c) ")"
                set(j, add(j,1))
            }
        }
    }
    "\""
}
