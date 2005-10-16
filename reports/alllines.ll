/*
 * @progname       allines.sgml.ll
 * @version        1.1
 * @author         Wetmore, Nozell
 * @category       
 * @output         SGML, NROFF
 * @description    
 *
 * This program shows all ancestral lines of a specified person
 * using a pseudo-Register format.
 *
 * Output is in nroff or sgml format.  This may change to something
 * more generic.
 *
 * Tom Wetmore, ttw@shore.net
 * beta version, 27 February 1997
 *
 * Marc Nozell, nozell@rootsweb.com
 * Added sgmldoc (formerly known as linuxdoc), 3 March 1997
 */

global(format_type)     /* what format? nroff or sgml? */
global(CurID)           /* ID values assigned to ancestors */
global(BOLK)            /* list of keys of persons who begin lines */
global(BOLG)            /* generations of begin line persons */
global(BOLR)            /* relationships of begin line persons */
global(CurK)            /* current line being processed */
global(CurG)            /* generations in current line */
global(CurR)            /* relations in current line */
global(AncT)            /* table of all ancestors */
global(AncL)            /* list of all ancestors */
global(KeyT)            /* table of all saved keys */
global(TOLT)            /* table of top of line persons */
global(TOLL)            /* list of top of line persons */
global(FamT)            /* NEED COMMENT TO DESCRIBE THIS!! */

/* User Options */

global(OPat)            /* follow paternal lines */
global(ORel)            /* show relationships */

/* LineParent -- Return parent in line direction. */

func LineParent (p)
{
        if (OPat) { return(father(p)) }
        else      { return(mother(p)) }
}

/* OthrParent -- Return parent in non-line direction. */

func OthrParent (p)
{
        if (OPat) { return(mother(p)) }
        else      { return(father(p)) }
}

/*
 * main - This is the main routine; it asks the user to identify a person
 * and then calls the DoIt routine.
 */

proc main ()
{
        getindi(i, "Enter person whose full registry ancestry is wanted.")
        if (i) { call DoIt(i) }
        else   { print("Program not run.") }
}

/*
 * DoIt - This is the top routine of the program; it calls routines to
 * perform the main algorithmic jobs and then calls a routine to write the
 * report.
 */

proc DoIt (i)
{
        set(CurID, 1)
        table(KeyT)
        call GetUserOptions()

/*
 * The first step in this program is to compute the list of "bottom of
 * line" persons.  These persons are those that on first sight seem to
 * require an ancestral line generated in the program's output.  Because
 * multiple bottom of line persons may have the same top of line ancestor
 * (due to pedigree collapse) it may turn out that there is not a separate
 * line computed for each bottom of line person.  This complication is
 * dealt with later.  The first bottom of line person is always the
 * starting person, and the first ancestral line shown in the output will
 * be the parental line of this person.  Normally this parental line will
 * be the paternal line.
 */
        print("Finding all bottom of line persons.\n")
        call BFirstCreateBOLLists(i)/**/
        /* call ShowBOLLists() /*DEBUG*/

/*
 * The second step is to build an ancestor table that contains all the
 * information about the ancestors of the key person that is needed in
 * generating the program's output.  The table accumulates the information
 * needed to deal with pedigree collapse.
 */

        print("Creating table of all ancestors.\n")
        call CreateAncStructures() /* call ShowAncTable() /**/

/*
 * The third step is to number the ancestors in the ancestor table in such
 * a way that on output each numbered ancestor magically has the right
 * sequential number.
 */

        print("Numbering all ancestors in table.\n")
        call NumberAncestors() /* call ShowAncTable() /**/

/*
 * The fourth step is to compute the list of top of line ancestors.  Due
 * to pedigree collapse there may be fewer top of line ancestors than
 * there are bottom of line persons.  Whenever this is the case, there
 * will be an ancestor somewhere in the line who has more than one child
 * who are also ancestors (the essence of pedigree collapse).  This program
 * collapses all lines that begin with the same person but lead to
 * different descendants (who are still all ancestors of the starting
 * person)
 */

        print("Computing top of line ancestors.\n")
        call CreateTOLList() /* call ShowTOLList() /**/

/*
 * The last step is to write the report.
 */

        print("Printing final report.\n")
        call WriteReport()
}

/*
 * GetUserOptions - As you can see, users can't actually select them yet!
 */

proc GetUserOptions ()
{
        getintmsg(format_type, "Enter 0 for nroff, 1 for sgml")

        set(OPat, 1)    /* this version only follows paternal lines */
        set(ORel, 1)    /* this version shows relationships */
}

/*
 * BFirstCreateBOLLists - This routine creates the beginning of lines lists.
 * This is the breadth first version of this routine.  Following is the
 * moving front version.  I don't know which order is the best.  Try them
 * both and see which you prefer.
 */

proc BFirstCreateBOLLists (i)
{
        list(BOLK) list(BOLG) list(BOLR)
        list(TmpK) list(TmpG) list(TmpR)
        enqueue(TmpK, savekey(key(i)))
        enqueue(TmpG, 1) enqueue(TmpR, 1)

        while (k, dequeue(TmpK)) {
                set(p, indi(k))
                set(g, dequeue(TmpG)) set(r, dequeue(TmpR))
                if (eq(1, mod(r, 2))) {
                        enqueue(BOLK, k) enqueue(BOLG, g) enqueue(BOLR, r)
                }
                set(g, add(1, g)) set(r, mul(2, r))
                if (f, LineParent(p)) {
                        enqueue(TmpK, savekey(key(f)))
                        enqueue(TmpG, g) enqueue(TmpR, r)
                }
                set(r, add(1, r))
                if (m, OthrParent(p)) {
                        enqueue(TmpK, savekey(key(m)))
                        enqueue(TmpG, g) enqueue(TmpR, r)
                }
        }
}

/*
 * MFrontCreateBOLLists - This routine also creates the beginning of line
 * lists.  This is the moving front version, and is not used in this beta
 * version.
 */

proc MFrontCreateBOLLists (i)
{
        list(BOLK) list(BOLG) list(BOLR)
        list(TmpK) list(TmpG) list(TmpR)
        enqueue(TmpK, savekey(key(i)))
        enqueue(TmpG, 1) enqueue(TmpR, 1)

        while (k, dequeue(TmpK)) {
                set(g, dequeue(TmpG)) set(r, dequeue(TmpR))
                set(p, indi(k))
                enqueue(BOLK, k) enqueue(BOLG, g) enqueue(BOLR, r)
                while (p) {
                        set(g, add(g, 1)) set(r, mul(r, 2))
                        if (m, OthrParent(p)) {
                                enqueue(TmpK, savekey(key(m)))
                                enqueue(TmpG, g) enqueue(TmpR, add(r, 1))
                        }
                        set(p, LineParent(p))
                }
        }
}

/*
 * CreateAncStructures - This routine creates the AncT table and AncL list.
 *  These are data structures that hold information about all ancestors of
 *  the starting person.  This routine operates by considering each bottom
 *  of line person in turn.  For each bottom of line person his or her
 *  ancestral line is computed and then the ProcessCurLine routine is
 *  called.  It is the ProcessCurLine routine that actually updates the
 *  data structures.
 *
 *  Note that the only use of the AncL list is in the debugging routine
 *  ShowAncTable.
 */

proc CreateAncStructures ()
{
        table(AncT) list(AncL)

        forlist(BOLK, k, n) {  /* for each bottom of line person ... */
                set(g, getel(BOLG, n)) set(r, getel(BOLR, n))
                set(p, indi(k))

                list(CurK) list(CurG) list(CurR) /* make them empty */
                while (p) { /* start with BOL person and follow line back */
                        push(CurK, savekey(key(p)))
                        push(CurG, g) push(CurR, r)
                        set(g, add(1, g))
                        set(r, mul(2, r))
                        set(p, LineParent(p))
                }
                call ProcessCurLine()
        }
}

/*
 * ProcessCurLine - This routine updates the ancestor table and list based
 * on an ancestral line just computed for a bottom of line person by the
 * CreateAncStructures routine.  This line is stored in the three global
 * lists CurK, CurG, and CurR, which form the interface between this
 * routine and CreateAncStructures.  This routine processes the line from
 * the last line ancestor of the bottom of line person to the bottom of
 * line person.
 */

proc ProcessCurLine ()
{
        set(f, 0)  /* f holds the line parent of the current person */
        set(k, pop(CurK))
        while (k) {
                set(p, indi(k))
                set(g, pop(CurG))
                set(r, pop(CurR))
                call AddToAncTable(k, g, r, f)
                /*name(p) " (" d(g) ", " d(r) ") "/*DEBUG*/
                set(f, k)
                set(k, pop(CurK))
        }
}

/*
 * AddToAncTable - This routine adds information to the ancestor table.
 * Each table entry is a list with six elements:
 * 1 Key of person
 * 2 ID of person
 * 3 Number of appearances in pedigree
 * 4 List of generations relative to key person by appearance
 * 5 List of relationships to key person by appearance
 * 6 List of children of this person who are also ancestors of key person
 */

proc AddToAncTable (k, g, r, f)
{
        if (e, lookup(AncT, k)) {  /* if person is already in table ... */

                setel(e, 3, add(1, getel(e, 3)))  /* incr num of appearances */
                set(l, getel(e, 4))
                enqueue(l, g)  /* update list of generations */
                set(l, getel(e, 5))
                enqueue(l, r)  /* update list of relationships */

        } else {  /* this is the first time this ancestor has been seen */

                list(e)  /* create new, empty table entry for person */
                enqueue(e, k)   /* add person's key */
                enqueue(e, 0)   /* init id to zero */
                enqueue(e, 1)   /* init num of appearences to one */
                list(l)         /* create sub-list to hold generations */
                enqueue(l, g)   /* init sub-list to current generation */
                enqueue(e, l)   /* add sub-list to table entry */
                list(l)         /* create sub-list to hold relationships */
                enqueue(l, r)   /* init sub-list to current relationship */
                enqueue(e, l)   /* add sub-list to table entry */
                list(l)         /* create sub-list to hold line descendants */
                enqueue(e, l)   /* add (empty) sub-list to table entry */
                insert(AncT, k, e)  /* add new entry to ancestor table */
                enqueue(AncL, k)  /* add key of person to ancestor list */
        }
        if (f) {  /* if not top of line make a child of line parent */
                set(d, lookup(AncT, f))
                set(l, getel(d, 6))
                if (not(inlist(l, k))) {
                        enqueue (l, k)
                }
        }
}

/*
 * NumberAncestors - This routine numbers the ancestors in the ancestor
 * table.
 */

proc NumberAncestors ()
{
        forlist(BOLK, k, n) {
                set(p, indi(k))
                while (f, LineParent(p)) { set(p, f) }
                call NumberLine(key(p))
        }
}

proc NumberLine (k)
{
        set(e, lookup(AncT, k))
        if (ne(0, getel(e, 2))) { return() }
        list(TmpQ)
        enqueue(TmpQ, k)
        while (k, dequeue(TmpQ)) {
                set(p, indi(k))
                set(e, lookup(AncT, k))
                setel(e, 2, CurID)
                set(CurID, add(1, CurID))
                set(cl, getel(e, 6))
                families (p, f, s, n) {
                        children (f, o, m) {
                                if (inlist(cl, key(o))) {
                                        enqueue(TmpQ, savekey(key(o)))
                                }
                        }
                }
        }
}

proc CreateTOLList ()
{
        table(TOLT) list(TOLL)
        forlist (BOLK, k, n) {
                set(p, indi(k))
                while (f, LineParent(p)) { set(p, f) }
                set(s, savekey(key(p)))
                if (and(nestr(k, s), not(lookup(TOLT, s)))) {
                        enqueue(TOLL, s)
                        insert(TOLT, s, s)
                }
        }
}

proc ShowTOLList ()
{
        "START OF LINE LIST --\n"
        forlist (TOLL, k, n) {
                name(indi(k)) "\n"
        }
}

/*
 * WriteReport - This routine controls writing a report.  Right now this
 * program has built in knowledge that the report is being generated in
 * nroff format.  This should be changed so that only generic routines
 * are called out of this routine, making substitution for different report
 * formats (e.g., LaTeX, HTML) easier in the future.
 */

proc WriteReport ()
{
        call WriteHeading()
        table(FamT)
        forlist (TOLL, k, n) {
                call WriteLine(k)
        }
        call WriteTail()
}

/*
 * WriteLine - This routine is responsible writing a single line to the
 * report file.
 */

proc WriteLine (k)      /* k -- key of a line's top of line person */
{
        call LineTitle(k)
        set(e, lookup(AncT, k))
        list(TmpQ)
        enqueue(TmpQ, k)
        while (k, dequeue(TmpQ)) {
                set(e, lookup(AncT, k))
                call WriteLinePerson(e)
                call WriteChildren(e)
                forlist(getel(e, 6), c, n) {
                        enqueue(TmpQ, c)
                }
        }
}

proc EmitPara () {
        if (eq(format_type, 0)) { call nroffPara() }
        else { call sgmlPara() }
}

proc EmitLeftSquareBracket () {
        if (eq(format_type, 0)) { call nroffLeftSquareBracket() }
        else { call sgmlLeftSquareBracket() }
}

proc EmitRightSquareBracket () {
        if (eq(format_type, 0)) { call nroffRightSquareBracket() }
        else { call sgmlRightSquareBracket() }
}

proc EmitStartList () {
        if (eq(format_type, 0)) { call nroffStartList() }
        else { call sgmlStartList() }
}

proc EmitEndList () {
        if (eq(format_type, 0)) { call nroffEndList() }
        else { call sgmlEndList() }
}

proc EmitChildItem () {
        if (eq(format_type, 0)) { call nroffChildItem() }
        else { call sgmlChildItem() }
}

proc WriteHeading () {
        if (eq(format_type, 0)) { call nroffhead() }
        else { call sgmlhead() }
}

proc WriteTail () {
        if (eq(format_type, 0)) { call nrofftail() }
        else { call sgmltail() }
}

proc LineTitle (k)
{
        if (eq(format_type, 0)) { call nroffLineTitle(k) }
        else { call sgmlLineTitle(k) }
}

proc nroffhead ()
{
    ".de CH\n"
    ".sp\n"
    ".in 11n\n"
    ".ti 1\n"
    "\\h'3n'\\h'-\\w'\\\\$1'u'\\\\$1\\h'5n'\\h'-\\w'\\\\$2'u'\\\\$2\\h'1n'\n"
    "..\n"

    ".de P\n.sp\n.in 0\n..\n"
    /*".po 5\n"*/
    ".ll 72\n"
    ".ls 1\n"
    ".na\n"
}

proc sgmlhead ()
{

    "<!doctype linuxdoc system>" nl()
    "<article>" nl()
    "<title>All Lines</title>" nl()
    "<author>by Marc Nozell</author>"
        "<abstract> " nl()
         "This shows all ancestral lines of a specified person  using a pseudo-Register format."
        "</abstract>" nl()
        "<toc>" nl()
}

proc nrofftail ()
{
        " " nl() /* pretty boring... */
}

proc sgmltail ()
{
        "  </article>" nl()
}

proc nroffLineTitle (k) {
        ".P\n.sp 2\nANCESTRAL LINE FROM " upper(name(indi(k))) "\n"
        ".br\n-----------------------------------------------------\n"
}

proc sgmlLineTitle (k) {
         nl()"<sect>Ancestral line from " upper(name(indi(k))) "\n"
}

proc nroffPara () {
        ".P\n"
}

proc sgmlPara () {
         "<p>\n"
}

proc nroffLeftSquareBracket () {
        "["
}
proc sgmlLeftSquareBracket () {
        "&lsqb;"
}

proc nroffRightSquareBracket () {
        "]"
}
proc sgmlRightSquareBracket () {
        "&rsqb;"
}

proc nroffStartList () {
        "\n"
}

proc sgmlStartList () {
        "<enum>\n"
}

proc nroffEndList () {
        "\n"
}

proc sgmlEndList () {
        "</enum>\n"
}

proc nroffChildItem () {
        " "
}

proc sgmlChildItem () {
        "<item>\n"
}


/*
 * WriteChildren - This routine writes out the children for a person in an
 * ancestral line.
 */

proc WriteChildren (e)
{
        set(p, indi(getel(e, 1)))
        set(cl, getel(e, 6))    /* list of child keys also in this line */
        families (p, f, s, n) {
                if (s) { set(u, save(name(s))) }
                else   { set(u, "(_____)") }
                if (lookup(FamT, key(f))) {
                        call EmitPara()
                        "Children of " name(p) " and " u
                        " listed under " u ".\n"
                } elsif (gt(nchildren(f), 0)) {
                        call EmitPara()
                        "Children of " name(p) " and " u ":\n"
                        call EmitStartList()
                        children(f, c, m) {
                                if (inlist(cl, key(c))) {
                                        set(ce, lookup(AncT, key(c)))
                                        call EmitChildItem()
                                        d(getel(ce, 2)) " "
                                        roman(m) "\n"
                                        call shortvitals(c)
                                } else {
                                        call EmitChildItem()
                                        roman(m) "\n"
                                        call middlevitals(c)
                                }
                        }
                        insert(FamT, savekey(key(f)), 1)
                        call EmitEndList()
                }
        }
}

proc shortvitals (i)
{
        name(i)
        set(b, birth(i)) set(d, death(i))
        if (and(b, short(b))) { ", b. " short(b) }
        if (and(d, short(d))) { ", d. " short(d) }
        ".\n"
        call EmitPara()
}

proc middlevitals (i)
{
        name(i) ".\n"
        set(e, birth(i))
        if(and(e,long(e))) {
                call EmitPara()
                "Born " long(e) ".\n" }
        if (eq(1, nspouses(i))) {
                spouses(i, s, f, n) {
                        call EmitPara()
                        "Married"
                        call spousevitals(s, f)
                }
        } else {
                spouses(i, s, f, n) {
                        call EmitPara()
                        "Married " ord(n) ","
                        call spousevitals(s, f)
                }
        }
        set(e, death(i))
        if(and(e, long(e))) {
                call EmitPara()
                "Died " long(e) ".\n" }
        set(p, 0)
}

/*
 * WriteLinePerson - This routine generates the report output for one
 * person in one of the ancestral lines.  This version of the routine
 * generates output in nroff format.  It prints boiler plate vitals
 * information about the person followed by all notes in the person's
 * record in the database.  This routine does not print the person's
 * children (see routine >>>>> for this).
 */

proc WriteLinePerson (e)
{
        set(p, indi(getel(e, 1)))
        call EmitPara()
        d(getel(e, 2)) "  "
        name(p)
        if (ORel) {
                call EmitLeftSquareBracket()
                set(c, "")
                forlist (getel(e, 5), r, n) {
                        c call ShowRel(r) set(c, ", ")
                }
                call EmitRightSquareBracket()
        }
        ".\n"
        call EmitPara()
        set(o, birth(p))
        if(and(o, long(o))) { "Born " long(o) ".\n" }
        if (eq(1, nspouses(p))) {
                spouses(p, s, f, n) {
                        "Married"
                        call spousevitals(s, f)
                }
        } else {
                spouses(p, s, f, n) {
                        "Married " ord(n) ","
                        call spousevitals(s, f)
                }
        }
        set(o, death(p))
        if(and(o, long(o))) { "Died " long(o) ".\n" }
        set(b, 0)
        fornotes(root(p), n) {
                if (not(b)) {
                        call EmitPara()
                        set(b, 1) }
                n "\n"
        }
}

proc spousevitals (s, f)
{
        set(e, marriage(f))
        if (and(e, long(e))) { "\n" long(e) "," }
        "\n" name(s)
        set(e, birth(s))
        if (and(e, long(e)))  { ",\nborn " long(e) }
        set(e, death(s))
        if (and(e, long(e)))  { ",\ndied " long(e) }
        set(d, LineParent(s))
        set(m, OthrParent(s))
        if (or(d, m)) {
                ",\n"
                if (male(s))      { "son of " }
                elsif (female(s)) { "daughter of " }
                else              { "child of " }
        }
        if (d)         { name(d) }
        if (and(d, m)) { "\nand " }
        if (m)         { name(m) }
        ".\n"
}

/*
 * ShowBOLLists - This debug routine shows the bottom of line persons as
 * recorded in the BOLK, BOLG, and BOLR lists
 */

proc ShowBOLLists ()
{
        forlist(BOLK, k, n) {
                set(g, getel(BOLG, n)) set(r, getel(BOLR, n))
                name(indi(k)) " " d(g) " "
                d(r) " (" call ShowRel(r) ")\n"
        }
}

proc ShowCurLine ()
{
        set(k, pop(CurK))
        set(p, indi(k))
        while (p) {
                set(g, pop(CurG)) set(r, pop(CurR))
                name(p) " (" d(g) "," d(r) ") "
                set(k, pop(CurK)) set(p, indi(k))
        }
        "\n"
}

/* ShowAncTable -- Debug routine which shows contents of AncT. */

proc ShowAncTable ()
{
        forlist(AncL, k, n) {
                set(e, lookup(AncT, k))
                set(p, indi(k))
                set(i, getel(e, 2))
                set(g, getel(e, 4))
                set(r, getel(e, 5))
                set(d, getel(e, 6))
                k " " name(p) " " d(i) " "
                forlist (g, j, l) { d(getel(g, l)) " " }
                forlist (r, j, l) { call ShowRel(getel(r, l)) " " }
                forlist (d, c, l) { name(indi(c)) " " }
                "\n"
        }
}

proc ShowRel (r)
{
        if (eq(r, 1)) { "s" }
        if (gt(r, 1)) {
                list(RelStack)
                push(RelStack, neg(1))
                while (gt(r, 1)) {
                        set(m, mod(r, 2))
                        set(r, div(r, 2))
                        push(RelStack, m)
                }
                set(r, pop(RelStack))
                while (ne(r, neg(1))) {
                        if (r) { "m" }
                        else   { "f" }
                        set(r, pop(RelStack))
                }
        }
}

/* inlist -- See if a string is in a list of strings */

func inlist (l, s)
{
        forlist(l, e, n) {
                if (eqstr(e, s)) { return(1) }
        }
        return(0)
}

func savekey (k)
{
        if (e, lookup(KeyT, k)) {  return(e) }
        set(k, save(k))
        insert(KeyT, k, k)
        return(k)
}
