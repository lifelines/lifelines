/*
 * @progname       dates.ll
 * @version        4
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

Prints out the value of all the lines in your database with the DATE
tag, along with enough information so you can find the line easily.
The purpose of this report is so you can sort all the dates in the
database, so you can look for illegal dates, make birthday lists, etc.
The dates are printed in the order that they appear in the database,
Output can then be sorted if this is more useful than the native form.

dates - a LifeLines dates extraction program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  3 December 1992
        Version 2,  8 December 1992 (minor bug fix, report in columns)
        Version 3,  5 February 1993 (speedup of tag handling)
        Version 4,  1 September 1993 (bug fix for families with no parents)

This code borrows heavily from the excellent program places, written
by David Olsen (dko@cs.wisc.edu), and contains improvements by Steve
Woodbridge (sew@pcbu.prime.com).

Prints out the value of all the lines in your database with the DATE
tag, along with enough information so you can find the line easily.
The purpose of this report is so you can sort all the dates in the
database, so you can look for illegal dates, make birthday lists, etc.
The dates are printed in the order that they appear in the database,
so the report is not very useful in its native form.  To make it more
useful, run the output file through the program 'sort', and perhaps
'grep' if you want to get only birthdates etc.  To make sorting
easier, each date is preceded by a eight-digit number of the form
yyyymmdd.  Then a simple ASCII line-by-line sort puts the dates in the
right order.  Days and months are checked for validity (thirty days
hath September and all that) and futurity.  It asks you if you want to
use the Gregorian calendar or the Julian calendar.  This only affects
the validity of Feb 29ths, so don't fret too much.  Any invalid or
future date is marked with a * as the first character of the line.
These will appear first in the sorted output.

If the date is part of an individual record, it is followed by the key
and name of the individual and by the hierarchy of tags between it and
the INDI tag.  (This is usually just a single tag, such as BIRT, CHR,
or DEAT.)  If the date is part of a family record, it is followed by
the key and name of the husband (or the wife is there is no husband,
or first child if there is no parent), the relationship in the family
of that person, and by the hierarchy of tags between it and the FAM
tag.  (This is usually just the single tag MARR.)  Some sample output
(selected lines that have been sorted):

15800000 ABT    1580          | BIRT | I4403 Valentin BISMARCK
15810000 1581                 | DEAT | I41   Catherina
15821221 21 DEC 1582          | BIRT | I4404 Berta ASSEBURG
15850000 ABT    1585          | BIRT | I2595 Henrich SPANUTH
15850529 29 MAY 1585          | DEAT | I4418 Brigitte BISMARCK
15860000 1586                 | BIRT | I2596 Johan SPANUTH
15860301 1 MAR 1586           | BIRT | I2675 Catharine SPANUTH
15870000 1587/1589            | BIRT | I2597 Caspar SPANUTH
15880201 1 FEB 1588           | BIRT | I2676 Johan SPANUTH
15890226 26 FEB 1589          | BIRT | I2677 Johan SPANUTH
15960000 1596                 | MARR | F883  I2679 Arend KOLLE, husb
16421009 9 OCT 1642           | BURI | I4404 Berta ASSEBURG
19800108 8 JAN 1980           | ENDL | I3635 Maria Catharina KINDLER
19800124 24 JAN 1980          | CHIL SLGC | F948  I2336 Anselm KINDLER, husb

*/

global(today)
global(tomonth)
global(toyear)
global(julian)

proc do_date(datenode)
{
    extractdate(datenode,day,month,year)
    if (or(le(month,0),gt(month,12))) { set(daysinmonth,0) }
    elsif (or(or(eq(month,9),eq(month,4)),
              or(eq(month,6),eq(month,11)))) { set(daysinmonth,30) }
    elsif (eq(month,2)) {
        if (and(eq(mod(year,4),0),
                or(julian,or(ne(mod(year,100),0),eq(mod(year,400),0))))) {
            set(daysinmonth,29) }
        else { set(daysinmonth,28) }
    }
    else { set(daysinmonth,31) }
    set(future,0)
    if (gt(year,toyear)) { set(future,1) }
    elsif (eq(year,toyear)) {
        if (gt(month,tomonth)) { set(future,1) }
        elsif (and(eq(month,tomonth),gt(day,today))) { set(future,1) }
    }
    if (or(gt(day,daysinmonth),future)) { "*" }
    if (lt(year,0)) { d(year) }
    else {
        if (lt(year,10)) { "0" }
        if (lt(year,100)) { "0" }
        if (lt(year,1000)) { "0" }
        d(year)
    }
    if (lt(month,10)) { "0" }
    d(month)
    if (lt(day,10)) { "0" }
    d(day) " "
}


proc main()
{
    getintmsg(julian,
        "Enter 0 for Gregorian (normal) or 1 for Julian (old) calendar")
    extractdate(gettoday(),today,tomonth,toyear)

    list(tag_stack)

    print("Printing all dates.\n")
    print("Be patient.  This may take a while.\n")

    forindi (person, pnum) {

        traverse (inode(person), node, level) {

            setel(tag_stack, add(level, 1), tag(node))

            if (eq(strcmp(tag(node), "DATE"), 0)) {
                call do_date(node)
                value(node) col(31) "| "
                set(tlength,0)
                set(tcount,0)
                forlist (tag_stack, a_tag, tnum) {
                    if (and(gt(tnum, 1), le(tnum, level))) {
                        a_tag " "
                        set(tlength,add(tlength,strlen(a_tag)))
                        set(tcount,add(tcount,1))
                    }
                }
                set(tlength,add(tlength,tcount))
                if (lt(tlength,5)) { col(38) }
                "| " key(person)
                col(add(41,mul(5,tcount))) name(person) "\n"
            }
        }
    }
    forfam (fam, fnum) {
        traverse (fnode(fam), node, level) {

            setel(tag_stack, add(level, 1), tag(node))

            if (eq(strcmp(tag(node), "DATE"), 0)) {
                call do_date(node)
                value(node) col(31) "| "
                set(tlength,0)
                set(tcount,0)
                forlist (tag_stack, a_tag, tnum) {
                    if (and(gt(tnum, 1), le(tnum, level))) {
                        a_tag " "
                        set(tlength,add(tlength,strlen(a_tag)))
                        set(tcount,add(tcount,1))
                    }
                }
                set(tlength,add(tlength,tcount))
                if (lt(tlength,5)) { col(38) }
                "| " key(fam)
                if (person,husband(fam)) { set(relation,", husb") }
                elsif (person,wife(fam)) { set(relation,", wife") }
                else {
                    children(fam,child,cnum) {
                        if (eq(cnum,1)) {
                            set(person,child)
                            set(relation,", chil")
                        }
                    }
                }
                if (person) {
                    col(add(41,mul(5,tcount))) key(person) " "
                    col(add(47,mul(5,tcount))) name(person) relation
                }
                "\n"
            }
        }
    }
}
