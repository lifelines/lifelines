/*
 * @progname       famrep.ll
 * @version        6.3
 * @author         James P. Jones (jjones@nas.nasa.gov)
 * @category       
 * @output         nroff
 * @description    
 *
 *   This report program produces a Family Group Sheet for the selected
 *   individual, with options for generating sheets for married children
 *   of the individual, and their children, etc.
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version two:    1 Nov 1992
 *   version three: 28 Mar 1993 bug fixes
 *   version four:  25 Apr 1993 added sources
 *   version five:  26 Sep 1993 added multiple indi's, bug fix
 *   version six:    3 Oct 1993 bug fixes
 *
 *   This report program produces a Family Group Sheet for the selected
 *   individual. User is given the choice of having sheets generated for
 *   married children of individual, and the children of the children, etc.
 *   Sources of information are indicated with end-note style
 *   references. The report produces 'roff output, which I suggest you
 *   convert to postscript for the highest quality report. Following are
 *   several examples how to process and print the report (assuming the
 *   output file name is "fam.out":
 *
 *        tbl fam.out | xroff -me -tstdout | ipr -Pim7 -D"jobheader off"
 *        tbl fam.out | xroff -me -PprinterName
 *        tbl fam.out | groff -me | your_postscript_printer
 *        tbl fam.out | troff -me | dpost | lp -dps
 *
 *   The data in "compiler" table in main() is initialized with property's
 *   obtained from the lifelines config file (~/.linesrc on unix else 
 *   lines.cfg) with values from
 *   user.fullname
 *   user.email 
 *   user.address
 *   user.phone 
 */

global(sourcelist)                      /* list of all sources used */
global(sourcestr)
global(compiler)
global(TRUE)
global(FALSE)
global(ONCE)

proc main ()
{
    monthformat(4)
    dateformat(0)
    set(TRUE, 1)
    set(FALSE, 0)
    set(ONCE, TRUE)
    list(sourcelist)

    table(compiler)
    insert(compiler, "name", getproperty("user.fullname"))
    insert(compiler, "addr", getproperty("user.address"))
    insert(compiler, "phone", getproperty("user.phone"))
    insert(compiler, "email", getproperty("user.email"))

    set(indi, NULL)
    while (eq(strcmp(name(indi), NULL), 0)) {
        getindi(indi)                   /* select individual for report */
        if (eq(strcmp(name(indi), NULL), 0)) {
            print("Individual not found in database.")
            print(nl())
        }
    }
    while (or(lt(ionly,1), gt(ionly,2))) {
      getintmsg(ionly,"Choose (1) Individual only, (2) + Married Descendents: ")
    }
    if (eq(ionly, 2)) {
        set(sonly,0)
        while (or(lt(sonly,1), gt(sonly,2))) {
            getintmsg(sonly,"Choose (1) Select Spouse, (2) All Spouses: ")
        }
    }
    else {
        set(sonly, 1)
    }
    call FGsheet(indi, ionly, sonly)
    print("Report Done, ")
}

/* Select the individual's spouse for the Family Group Sheet.
 */
proc FGsheet(indi, ionly, sonly)
{
    if (eq(sonly, 1)) {
        set(i, nspouses(indi))
        spouses(indi, svar, fvar, no) { /* display spouses */
            if (gt(i, 1)) {
                if (gt(no, 7)) {                /* leave space for prompt */
                    print(nl())
                    print(nl())
                    print(nl())
                    print(nl())
                }
                print(d(no))
                print(". ")
                print(fullname(svar,TRUE,FALSE,50))
                print(nl())
            }
        }
        if (gt(i, 1)) {                 /* select a spouse */
            getintmsg(num, "Choose which spouse for Family Report: ")
        }
        else {
            set(num, 1)
        }
        if (lt(i, 1)) {
            print(name(indi))
            print(" has no spouse in database...")
            print(nl())
        }
        else {
            if (eq(ONCE, TRUE)) {
                ".po 0.8i" nl()
                ".ll 6.8i" nl()
                ".pl +1.5i" nl()
                ".nf" nl()

                set(ONCE, FALSE)
            }
        }
    }
    spouses(indi, svar, fvar, no) {
        if (or(and(eq(sonly,1), eq(no, num)), eq(sonly,2))) {
            if (eq(strcmp(sex(indi), "F"), 0)) {
                set(tmp, indi)          /* Check sex of individual,*/
                set(tindi, svar)        /* if Female, replace with */
                set(tsvar, tmp)         /* information on husband. */
                set(i, nspouses(tindi)) /* Easier if assume head-  */
                set(num, 1)             /* of-household is male... */
                if (gt(i, 1)) {
                    spouses(tindi, tmps, tmpf, no) {
                        if (eq(name(tsvar), name(tmps))) {
                            set(num, no)
                        }
                    }
                }
                call doform(tindi, tsvar, fvar, i, num)
            }
            else {
                call doform(indi, svar, fvar, i, num)
            }
            call printsources(sourcelist)
            while (not(empty(sourcelist))) {    /* NULL out sources each time */
                set(nil, dequeue(sourcelist))
                set(sourcestr, NULL)
            }
            ".bp" nl()

            if (eq(ionly, 2)) {
                children(fvar, kid, j) {
                    if(or(ge(nspouses(kid),1), ge(nfamilies(kid),1))) {
                        call FGsheet(kid, ionly, 2)
                    }
                }
            }
        }
    }
}

/* Produce the Family Group Sheet form.
 */
proc doform(indi, svar, fvar, numsp, cursp)
{
    ".ps 16" nl()
    ".(b C" nl()
    if (e, surname(indi)) { upper(surname(indi)) }
    "\\0FAMILY\\0GROUP\\0SHEET" nl()
    ".ps 10" nl()
    "Compiled by: \\fI" lookup(compiler, "name") "\\fR\\0on\\0\\fI"
    stddate(gettoday())
    "\\fR" nl()
    ".vs 10" nl()
    "\\fI" lookup(compiler, "addr") "\\fR" nl()
    "\\fIPhone:\\0" lookup(compiler, "phone") "\\0\\0\\0E-mail:\\0"
    lookup(compiler, "email") "\\fR" nl()
    ".)b" nl()
    ".ps 8" nl()
    ".TS" nl()
    "tab(+) expand box;" nl()
    "l s s." nl()
    "Husband's Full Name:\\0\\fI"
    if (e, name(indi)) { name(indi) "\\fR" nl() }
    else { "\\fR" }
    "_" nl()
    ".T&" nl()
    "l | l | l." nl()
    "Husband's Data+Day Month Year+City,\\0\\0Town or Place\\0\\0County or Province\\0\\0State or Country" nl()
    "_" nl()
    "\\0Birth+\\fI"
    set(aday, birth(indi))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()      /* note: first call to source */
    "_" nl()
    "\\0Christened+\\fI"
    set(aday, baptism(indi))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    "\\0Married+\\fI"
    set(aday, marriage(fvar))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    "\\0Death+\\fI"
    set(aday, death(indi))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    "\\0Burial+\\fI"
    set(aday, burial(indi))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    ".T&" nl()
    "l | l s." nl()
    "\\0Father's Name:+\\fI"
    if (e, name(father(indi))) { name(father(indi)) "\\fR" nl() }
    else { "\\fR" nl() }
    "_" nl()
    "\\0Mother's Maiden Name:+\\fI"
    if (e, name(mother(indi))) { name(mother(indi)) "\\fR" nl() }
    else { "\\fR" nl() }
    "_" nl()
    "\\0Other Wives:\\fI"
    set(f, 0)
    set(spstr, save(name(wife(fvar))))
    spouses(indi, wifenm, tmpfvar, no) {
        set(wstr, save(name(wifenm)))
        if (ne(strcmp(spstr, wstr), 0)) {
            "\\fI+"
            name(wifenm)
            "\\fR" nl()
            set(f,1)
        }
    }
    if (eq(f, 0)) { "\\fR" nl() }
    "_" nl()
    ".TE" nl()
    ".TS" nl()
    "tab(+) expand box;" nl()
    "l s s." nl()
    "Wife's Full Maiden Name:\\0\\fI"
    if (e, name(svar)) { name(svar) }
    "\\fR" nl()
    "_" nl()
    ".T&" nl()
    "l | l | l." nl()
    "Wife's Data   +Day Month Year+City,\\0\\0Town or Place\\0\\0County or Province\\0\\0State or Country" nl()
    "_" nl()
    "\\0Birth+\\fI"
    set(aday, birth(svar))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    "\\0Christened+\\fI"
    set(aday, baptism(svar))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    "\\0Death+\\fI"
    set(aday, death(svar))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    "\\0Burial+\\fI"
    set(aday, burial(svar))
    if (e, stddate(aday)) { stddate(aday) }
    "+"
    if (e, place(aday)) { place(aday) } "\\fR"
    if (aday) { call dosource(aday) } nl()
    "_" nl()
    ".T&" nl()
    "l | l s." nl()
    "\\0Father's Name:+\\fI"
    if (e, name(father(svar))) { name(father(svar)) "\\fR" nl() }
    else { "\\fR" nl() }
    "_" nl()
    "\\0Mother's Maiden Name:+\\fI"
    if (e, name(mother(svar))) { name(mother(svar)) "\\fR" nl() }
    else { "\\fR" nl() }
    "_" nl()
    "\\0Other Husbands:\\fI"
    set(f, 0)
    set(spstr, save(name(indi)))
    spouses(svar, hubby, tmpfvar, no) {
        set(hstr, save(name(hubby)))
        if (ne(strcmp(spstr, hstr), 0)) {
            "\\fI+"
            name(hubby)
            "\\fR" nl()
            set(f,1)
        }
    }
    if (eq(f, 0)) { "\\fR" nl() }
    "_" nl()
    ".TE" nl()
                                                /* now for the children... */
    set(haschild, 1)
    children(fvar, cvar, no) {
        if (eq(haschild, 1)) {
            ".TS" nl()
            "tab(+) expand box;" nl()
            "l |l| l | l | l." nl()
            "Complete Names of All Children+Sex+Event+Date+"
            "City, Town, County, State or Country" nl()
            "_" nl()
            set(haschild, 2)
        }

        if (or(eq(no, 4), eq(no, 12))) {        /* If 4th or 12th kid, start  */
            ".TE" nl()                          /* a new page. There was an   */
            ".bp" nl()                          /* old woman, who lived in a  */
            ".TS" nl()                       /* shoe, she had so many kids... */
            "tab(+) expand box;" nl()
            "l |l| l | l | l." nl()
            "Complete Names of All Children+Sex+Event+Date+"
            "City, Town, County/Province, State, Country" nl()
            "_" nl()
        }
        "T{" nl()
        "\\fI("
        d(no)
        ") "
        if (e, name(cvar)) { name(cvar) }
        "\\fR" nl()
        "T}+\\fI"
        sex(cvar)
        "\\fR+Birth+\\fI"
        set(aday, birth(cvar))
        if (e, stddate(aday)) { stddate(aday) }
        "+"
        if (e, place(aday)) { place(aday) } "\\fR"
        if (aday) { call dosource(aday) } nl()
        "_" nl()
        "\\^+\\^+Death+\\fI"
        set(aday, death(cvar))
        if (e, stddate(aday)) { stddate(aday) }
        "+"
        if (e, place(aday)) { place(aday) } "\\fR"
        if (aday) { call dosource(aday) } nl()
        "_" nl()
        "\\^+\\^+Burial+\\fI"
        set(aday, burial(cvar))
        if (e, stddate(aday)) { stddate(aday) }
        "+"
        if (e, place(aday)) { place(aday) } "\\fR"
        if (aday) { call dosource(aday) } nl()

        families(cvar, cfvar, csvar, no) {              /* spouses */
            "_" nl()
            "\\^+\\^+Marriage+\\fI"
            set(aday, marriage(cfvar))
            if (e, stddate(aday)) { stddate(aday) }
            "+"
            if (e, name(csvar)) { name(csvar) }
            if (aday) { call dosource(aday) }
            "\\fR" nl()
        }
        "=" nl()
    }
    if (eq(haschild, 2)) {
        ".TE" nl()
    }
}


/* Short macro procedure to combine SOURCE and SOURCENUM calls, to shorten
 * above report code.
 */
proc dosource(eventnode)
{
    call source(eventnode)              /* get source of data */
    if (sourcestr) {                    /* if source not NULL */
        call sourcenum()                /* print source number */
    }
}

/* Retrieve source from a given event (EVENTNODE), and save it in the global
 * string SOURCESTR.
 */
proc source(eventnode)
{
    set(sourcestr, NULL)
    traverse(eventnode, node, lev) {
       if (eq(strcmp(tag(node), "SOUR"), 0)) {
           set(sourcestr, value(node))
       }
    }
}

/* Create a "List of Sources" table for the report; in the report itself,
 * print only a footnote number, and later the list these number refer to
 * can be printed (via PRINTSOURCES).
 */
proc sourcenum()
{
    set(found,0)
    forlist(sourcelist, item, i) {
	set(numsources,i)
        if (eq(strcmp(item, sourcestr), 0)) {   /* if source in list */
            " \\s7(" d(i) ")\\s8"               /* print out source index */
            set(found, 1)
        }
    }
    if (not(eq(found, 1))) {
        push(sourcelist, sourcestr)             /* otherwise add it to list */
        " \\s7(" d(add(numsources,i,1)) ")\\s8" /* and print source index */
    }
}

/* Print a list of all the sources refered to in the document. The numbers
 * preceeding each source entry are what the in-line references refer to.
 */
proc printsources(slist)
{
    if (not(empty(slist))) {
        ".(b C" nl()
        "LIST OF SOURCES REFERENCED IN THIS REPORT" nl()
        ".)b" nl()
        forlist(slist, item, i) {
            "(" d(i) ")  " item nl()
        }
    }
}

/* End of Report
 */
