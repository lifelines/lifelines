/*
 * @progname       desc-henry.ll
 * @version        8
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program prints out a descendants report, assigning a d'Aboville,
Henry, modified Henry, or modern Henry code to the individuals.  The
chosen ancestor, and all of his/her spouses, descendants, and
descendants' spouses are included in the report.

desc-henry - a LifeLines descendants listing program using Henry codes
    by Jim Eggert (eggertj@atc.ll.mit.edu)
    Versions 1-3 1992
    Version 4,  7 Jan 1993 (added generation limit)
    Version 5, 22 Dec 1993 (added header, trailer, and optional keys)
    Version 6, ???????????
    Version 7, 17 Mar 1995 (added grouped code option)
    Version 8,  6 Jun 1995 (added numbering options)

Some sample codes are:
            d'Aboville     Henry     modified Henry  modern Henry
root        1              1         1               1
child  1    1.1            11        11              11
child 10    1.10           1X        1(10)           1A
child 11    1.11           1A        1(11)           1B
child 20    1.20           1J        1(20)           1K
g-child     1.20.1         1J1       1(20)1          1K1
gg-child    1.20.1.4       1J14      1(20)14         1K14
ggg-child   1.20.1.4.15    1J14E     1(20)14(15)     1K14F
gggg-child  1.20.1.4.15.3  1J14E3    1(20)14(15)3    1K14F3

Spouses codes, if requested, are indicated by appending .sn, where n
indicates which spouse is meant, and is omitted if there is only one
spouse.  The root code is user selectable so that you can have
arbitrary code prefixes.

I use the latter feature when my database indicates that person X was
not a descendant of Y, but I want to rig up a report which indicates X
is to be included in Y's descendancy.  I make two reports, one of Y's
real descendancy, and the second of X's giving X the number he would
have in Y's descendancy.  Then I need merely edit the two files to
achieve the desired result.

The program can also generate grouped codes, where the generation
separator (if any) is replace by a comma every three generations.  The
choice of arbitrary roots indicates that an additional parameter, the
initial comma location, be selectable.  The grouped format is
sometimes used in published genealogies, using a single capital letter
for the root symbol.

The user can elect to include only male descendance lines.  This is
useful for single-name studies.  In this case, spouses are not printed
as separate entries, but are indicated with the descendant.  For
female descendants, an indication of the number of children is also
printed.

The user can select whether no dates, simple dates (birth - death), or
dates and places (birth, baptism, death, burial, one per line) are to
be printed.  Also top-level notes can be optionally printed.  The
program only understands PAF-like events and notes.  Printing simple
dates and no notes gives a useful one-line-per-person outline.

The user can also elect to limit the number of generations to be printed
out.  Selecting 0 means all generations will be printed out.

The user can also elect to include keys for each individual in the report.

The user can also elect to exclude, with annotation, repeated individuals.

The report will include a header and a trailer.  You may easily modify the
do_header() and do_trailer() procedures to alter or eliminate these if
you wish.

*/

global(do_notes)
global(do_dates)
global(do_keys)
global(generations)
global(written_people)
global(this_indi_already_done)
global(notation)
global(grouped)
global(code_sep)
global(group_sep)
global(comma_separation)
global(first_comma)
global(malesonly)

proc main ()
{
    table(written_people)
    dayformat(1)
    monthformat(4)
    getindimsg(indi_root,
      "Enter root individual for report generation")
    getstrmsg(root,
      "Enter Henry code string for root individual (usually 1)")
    list(henry_list)
    push(henry_list,save(root))

    list(choices)
    enqueue(choices,save(concat("d'Aboville      ",root,".5.12.10")))
    enqueue(choices,save(concat("Henry           ",root,"5BX")))
    enqueue(choices,save(concat("modified Henry  ",root,"5(12)(10)")))
    enqueue(choices,save(concat("modern Henry    ",root,"5CA")))
    set(notation,menuchoose(choices,"Select notation:"))

    if (eq(notation,1)) { set(code_sep,".") } else { set(code_sep,"") }
    getintmsg(grouped,
      "Enter 0 for ungrouped notation, 1 for grouped notation")
    if (grouped) {
        getintmsg(comma_separation,
            "Enter comma separation, usually 3")
        getintmsg(first_comma,
            "Enter comma offset (0-2, default=0)")
        set(group_sep,",")
    } else {
        set(group_sep,"")
        set(comma_separation,999)
        set(first_comma,0)
    }
    getintmsg(do_dates,
      "Enter 0 for no dates, 1 for dates, 2 for dates+places")
    getintmsg(do_notes,"Enter 0 for no notes, 1 for notes")
    getintmsg(do_keys,"Enter 0 for no keys, 1 for keys")
    getintmsg(malesonly,
      "Enter 0 for all descendants, 1 for male lines only")
    getintmsg(generations,"Enter number of generations (0=all)")
    call do_header(indi_root)
    call desc_sub(indi_root,henry_list)
    call do_trailer(indi_root)
}

proc do_header(indi_root)
{
    "desc-henry:  Descendant report for " fullname(indi_root,0,1,80)
    if (do_keys) { " (" key(indi_root) ")" }
    "\nGenerated by the LifeLines Genealogical System on "
    stddate(gettoday()) ".\n\n"
}

proc do_trailer(indi_root)
{
    "\nEnd of Report\n"
}

proc do_name(person,henry_list,marr)
{
    set(h,"")
    set(c,sub(first_comma,1))
        /* less one for the root symbol */
    forlist(henry_list,l,li) {
        if (not(strcmp(trim(l,1),"s"))) {
            set(h,save(concat(h,".",l)))
        }
        else {
            if (eq(li,1)) { set(h,concat(h,l)) }
            elsif (and(grouped,eq(c,0))) { set(h,concat(h,group_sep,l)) }
            else { set(h,concat(h,code_sep,l)) }
            incr(c)
            set(c,mod(c,comma_separation))
        }
    }
    h ". "
    if (person) { fullname(person,0,1,80) } else { "<SPOUSE>" }
    if (and(person,eq(do_keys,1))) { " (" key(person) ")" }
    if (l,lookup(written_people,key(person))) {
        " appears above as " l "\n"
    }
    else {
        if (person) { insert(written_people,save(key(person)),h) }
        if (and(person,eq(do_dates,1))) {
            " ("
            set(e,birth(person))
            if (and(e,date(e))) { date(e) }
            else {
                set(e,baptism(person))
                if (and(e,date(e))) { "bap." date(e) }
            }
            " - "
            set(e,death(person))
            if (and(e,date(e))) { date(e) }
            else {
                set(e,burial(person))
                if (and(e,date(e))) { "bur." date(e) }
            }
            ")"
        }
        "\n"
        if (eq(do_dates,2)) {
            if (person) {
                if (e,birth(person))   { "     b: " long(e) "\n" }
                if (e,baptism(person)) { "   bap: " long(e) "\n" }
            }
            if (marr)                  { "     m: " long(marr) "\n"}
            if (malesonly) {
                set(nfam,nfamilies(person))
                families(person,fam,sp,spi) {
                    if (gt(nfam,1))    { "    m" d(spi) }
                    else               { "     m" }
                    ": " long(marriage(fam))
                    " to " if (sp) { fullname(sp,0,1,80) } else { "<SPOUSE>" }
                    if (female(person)) {
                        ", "
                        set(nc,nchildren(fam))
                        if (not(nc)) { "no children" }
                        else {
                            card(nc) " child" if (gt(nc,1)) { "ren" }
                        }
                    }
                    "\n"
                }
            }
            if (person) {
                if (e,death(person))   { "     d: " long(e) "\n" }
                if (e,burial(person))  { "   bur: " long(e) "\n" }
            }
        }
        if (and(person,eq(do_notes,1))) {
            fornodes(inode(person), node) {
                if (eq(0,strcmp("FILE", tag(node)))) {
                    copyfile(value(node)) }
                elsif (eq(0,strcmp("NOTE", tag(node)))) {
                    "     " value(node) "\n"
                    fornodes(node, subnode) {
                    if (eq(0,strcmp("CONT", tag(subnode)))) {
                    "     " value(subnode) "\n" }
                        }
                    }
                }
            fornodes(inode(person), node) {
                if (eq(0,strcmp("REFN", tag(node)))) {
                    "     SOURCE: " value(node) "\n"
                }
            }
        }
        if (or(eq(do_dates,2),eq(do_notes,1))) { "\n" }
    }
}

func desc_code(number)
{
    if (eq(notation,1)) { return(d(number)) }
    if (eq(notation,2)) {
        if (lt(number,10)) { return(d(number)) }
        if (eq(number,10)) { return("X") }
        return(upper(alpha(sub(number,10))))
    }
    if (eq(notation,3)) {
        if (lt(number,10)) { return(d(number)) }
        return(concat("(",d(number),")"))
    }
    if (eq(notation,4)) {
        if (lt(number,10)) { return(d(number)) }
        return(upper(alpha(sub(number,9))))
    }
    return("?")
}


proc desc_sub(person,henry_list)
{
    call do_name(person,henry_list,0)
    set(nfam,nfamilies(person))
    set(chi,0)
    families(person,fam,sp,spi) {
        if (not(malesonly)) {
            if (gt(nfam,1)) { push(henry_list,save(concat("s",d(spi)))) }
            else { push(henry_list,"s") }
            call do_name(sp,henry_list,marriage(fam))
            set(junk,pop(henry_list))
        }
        if (or(eq(generations,0),
               lt(length(henry_list),generations))) {
            if (or(not(malesonly),male(person))) {
                children (fam,ch,famchi) {
                    incr(chi)
                    push(henry_list,save(desc_code(chi)))
                    call desc_sub(ch,henry_list)
                    set(junk,pop(henry_list))
                }
            }
        }
    }
}
