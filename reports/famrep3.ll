/*
 * @progname       famrep3
 * @version        3.0
 * @author         Jones
 * @category       
 * @output         nroff
 * @description    
 *
 *   famrep3
 *
 *   Code by James P. Jones, jjones@nas.nasa.gov
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version two of this report was written by James P. Jones, 1 Nov 1992
 *   version three of  this report was written by James P. Jones, 28 Mar 1993
 *
 *   Family Group Sheet report program, with in-line source references.
 *
 *   I use:     tbl fam.out | xroff -me -tstdout | ipr -Pim7 -D"jobheader off"
 *   Can use:   tbl fam.out | xroff -me -PprinterName
 *              tbl fam.out | groff -me | your_postscript_printer
 */

global(sourcelist)                      /* list of all sources used */
global(sourcestr)

proc main ()
{
    monthformat(4)
    dateformat(0)
    set(TRUE, 1)
    set(FALSE, 0)
    list(sourcelist)
    getindi(indi)                       /* select individual for report */
    set(i, nspouses(indi))
    spouses(indi, svar, fvar, no) {     /* display spouses */
        if (gt(i, 1)) {
            if (gt(no, 4)) {            /* leave space for prompt */
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
    if (gt(i, 1)) {                     /* select a spouse */
        getintmsg(num, "Choose which spouse for Family Report: ")
    }
    else {
        set(num, 1)
    }
    spouses(indi, svar, fvar, no) {
        if (eq(no, num)) {
            if (eq(strcmp(sex(indi), "F"), 0)) {
                set(tmp, indi)          /* Check sex of individual,*/
                set(indi, svar)         /* if Female, replace with */
                set(svar, tmp)          /* information on husband. */
                set(i, nspouses(indi))  /* Easier if assume head-  */
                set(num, 1)             /* of-household is male... */
                if (gt(i, 1)) {
                    spouses(indi, tmps, tmpf, no) {
                        if (eq(name(svar), name(tmps))) {
                            set(num, no)
                        }
                    }
                }
            }
            call doform(indi, svar, fvar, i, num)
        }
    }
    if (eq(i, 0)) {
        print("This person has no spouse in database...")
        print(nl())
        print("Report NOT created, ")
        call doform(indi, 0, 0, 0, 0)

    }
    else {
        call printsources(sourcelist)
        print("Report Done, ")
    }
}

/* Produce the Family Group Sheet form.
 */
proc doform(indi, svar, fvar, numsp, cursp)
{
    ".po 0.8i" nl()
    ".ll 6.8i" nl()
    ".pl +1.5i" nl()
    ".nf" nl()
    ".ps 16" nl()
    ".(b C" nl()
    if (e, surname(indi)) { upper(surname(indi)) }
    "\\0FAMILY\\0GROUP\\0SHEET" nl()
    ".ps 10" nl()
    "Compiled by: \\fIJames\\0Patton\\0Jones\\fR\\0on\\0\\fI"
    stddate(gettoday())
    "\\fR" nl()
    ".vs 10" nl()
    "\\fI619\\0West\\0Remington\\0Drive,\\0Sunnyvale,\\0CA\\094087\\fR" nl()
    "\\fIPhone:\\0408.739.4556\\0\\0\\0E-mail:\\0jjones@nas.nasa.gov\\fR" nl()
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
    "=" nl()
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
    ".T&" nl()
    "l s s s." nl()
    " " nl()
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
    "=" nl()
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
            "l | l | l | l | l." nl()
            "Children's Full Names+Sex+Children's Data+Day Month Year+"
            "City, Town or Place County or Province State or Country" nl()
            "_" nl()
            set(haschild, 2)
        }

        if (or(eq(no, 4), eq(no, 12))) {        /* If 4th or 12th kid, start  */
            ".TE" nl()                          /* a new page. There was an   */
            ".bp" nl()                          /* old woman, who lived in a  */
            ".TS" nl()                       /* shoe, she had so many kids... */
            "tab(+) expand box;" nl()
            "l | l | l | l | l." nl()
            "Children's Full Names+Sex+Children's Data+Day Month Year+"
            "City, Town or Place County or Province State or Country" nl()
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
        "_" nl()

        families(cvar, cfvar, csvar, no) {              /* spouses */
            "\\^+\\^+Marriage+\\fI"
            set(aday, marriage(cfvar))
            if (e, stddate(aday)) { stddate(aday) }
            "+"
            if (e, name(csvar)) { name(csvar) }
            if (aday) { call dosource(aday) }
            "\\fR" nl()
            "_" nl()
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
        if (eq(strcmp(item, sourcestr), 0)) {   /* if source in list */
            " \\s7(" d(i) ")\\s8"                 /* print out source index */
            set(found, 1)
        }
    }
    if (not(eq(found, 1))) {
        push(sourcelist, sourcestr)             /* otherwise add it to list */
        " \\s7(" d(add(i,1)) ")\\s8"            /* and print source index */
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
        nl()
        forlist(slist, item, i) {
            "(" d(i) ")  " item nl()
        }
    }
}

/* End of Report */
