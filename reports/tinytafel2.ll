/*
 * @progname       tinytafel2
 * @version        2.0
 * @author         Wetmore, Eggert
 * @category       
 * @output         TinyTafel
 * @description    


tinytafel2

Based on tinytafel1 by Tom Wetmore, ttw@cbnewsl.att.com

This report works only with the LifeLines Genealogy program

Version 1,        1991, by Tom Wetmore.
Version 2, 11 Jan 1993, by Jim Eggert, eggertj@ll.mit.edu,
                        added header, trailer, sorting, date fixing,
                        and default moderate interest.  Modified
                        empty surname recognition.

This report will produce a tinytafel report on a person.

Output is an ASCII file.  It should be edited to translate any
non-ASCII characters, to shorten long place names (to 14-16
characters), and to indicate interest level after each year:
   [space] No interest (level 0)
   .       Low interest (level 1)
   :       Moderate interest (level 2) (default)
   *       Highest interest (level 3)

You will want to modify the write_tafel_header() procedure to
include your name, address, etc.

Empty surnames or those starting with "_" or " " will not
be written to the report.  This report doesn't do birthyear
estimation; it uses other events for the year if birthyear
is not available.

See the end of this report for an example of a tinytafel report.
*/

global(tafelset)
global(fdatelist)
global(ldatelist)
global(fplacelist)
global(lplacelist)
global(line_count)

proc write_tafel_header() {
    "N John Q. Public\n"           /* your name, mandatory */
    "A 1234 North Maple\n"         /* address, 0-5 lines */
    "A Homesville, OX 12345-6789\n"
    "A USA\n"
    "T 1 (101) 555-1212\n"         /* telephone number */
    "C 19.2 Baud, Unix System\n"   /* communications */
    "C Send any Email to:  jqpublic@my.node.address\n"
    "B SoftRoots/1-101-555-3434\n" /* BBS system/phone number */
    "D Unix Operating System\n"    /* diskette formats */
    "F LifeLines Genealogy Program for Unix\n"  /* file format */
    "R This is a default header, please ignore.\n"  /* comments */
    "Z " d(line_count) "\n"
}

proc main ()
{
    list(plist)
    indiset(tafelset)
    list(fdatelist)
    list(ldatelist)
    list(fplacelist)
    list(lplacelist)
    set(line_count,0)

    getindi(person)
    enqueue(plist, person)
    while (person, dequeue(plist)) {
        call process_line(person, plist)
    }
    namesort(tafelset)
    call write_tafel_header()
    call write_tafelset()
    call write_tafel_trailer()
}

global(fdate)
global(ldate)
global(pdate)
global(fplace)
global(lplace)
global(pplace)
global(sname)

proc write_tafelset() {
    forindiset(tafelset,person,index,snum) {
        soundex(person) " "
        getel(ldatelist,index) ":" /* moderate interest by default */
        getel(fdatelist,index) ":"
        surname(person)
        if (lplace,getel(lplacelist,index)) { "\\" lplace }
        if (fplace,getel(fplacelist,index)) { "/" fplace }
        "\n"
    }
}

proc write_tafel_trailer() {
    "W " date(gettoday()) "\n"
}

proc process_line (person, plist)
{
    call first_in_line(person)
    set(initial,trim(sname,1))
    if (and(and(strcmp(initial, "_"),
                strcmp(initial, " ")),
            strcmp(sname,""))) {
        set(last, 0)
        while (person) {
            print(".")
            if (moth, mother(person)) {
                enqueue(plist, moth)
            }
            set(last, person)
            set(person, father(person))
            if (strcmp(sname, surname(person))) {
                call last_in_line(last)
                call first_in_line(person)
            }
        }
    }
}

proc first_in_line (person)
{
    call set_year_place(person)
    set(fdate, pdate)
    set(fplace, pplace)
    set(sname,save(surname(person)))
}

proc last_in_line (person)
{
    call set_year_place(person)
    set(ldate, pdate)
    set(lplace, pplace)
    set(line_count,add(line_count,1))
    addtoset(tafelset,person,line_count)
    if (and(strcmp(ldate,"????"),
            gt(strcmp(ldate,fdate),0))) {
        /* reverse order ldate and fdate */
        enqueue(ldatelist,save(fdate))
        enqueue(fdatelist,save(ldate))
    }
    else {
        /* normal order ldate and fdate */
        enqueue(ldatelist,save(ldate))
        enqueue(fdatelist,save(fdate))
    }
    enqueue(lplacelist,save(lplace))
    enqueue(fplacelist,save(fplace))
}

proc set_year_place (person)
{
    set (yr, year(birth(person)))
    if (eq(yr, 0)) {
        set (yr, year(baptism(person)))
    }
    if (eq(yr, 0)) {
        set (yr, year(death(person)))
    }
    if (eq(yr, 0)) {
        set (yr, year(burial(person)))
    }
    if (eq(yr, 0)) {
        set (yr, "????")
    }
    set(pdate, save(yr))
    set(pl, place(birth(person)))
    if (eq(pl, 0)) {
        set(pl, place(baptism(person)))
    }
    if (eq(pl, 0)) {
        set(pl, place(death(person)))
    }
    if (eq(pl, 0)) {
        set(pl, place(burial(person)))
    }
    set(pplace, save(pl))
}


/*

Here is an example of a tiny tafel by Cliff Manis.

Note that the "Z" line is the number of actual data lines.

N Alda Clifford Manis
A P. O. Box 33937
A San Antonio
A Texas
A 78265-3937
T 1 (512) 654-9912
C 19.2 Baud, Unix System
C Send any Email to:  cmanis@csoftec.csf.com
D Unix Operating System
F LifeLines Genealogy Program for Unix
Z 16
M520 1939 1939 Manis\Knoxville, Knox Co, TN/Knoxville, Knox Co, TN
M520 1780 1902 Manes\Sevier Co, TN ?/Union Valley, Sevier Co, TN
M520 1770 1770 Maness\Sevier Co, Tennessee ?/Sevier Co, Tennessee ?
M520 1805 1914 Manis\North Carolina ?/Dandridge, Jefferson Co, TN
C536 1820 1869 Canter\VA/Jonesboro, Washington Co, TN
B620 1765 1829 Bowers/TN
N550 1730 1881 Newman\Monroe Co., WV/Jefferson Co, TN
B630 1760 1845 Bird\Frederick Co, VA/Sevier Co, TN
B630 1730 1730 Barth\Germany/Germany
F652 1745 1810 Francis\Augusta Co, VA ?/Rutherford Co, NC
W365 1860 1846 Whitehorn\VA/Washington Co, TN ?
C500 1700 1808 Cowan/TN
C613 1720 1843 Corbett\Scotch-Irish Dec/Jefferson Co, TN
R525 1750 1806 Rankin\Scotland/Jefferson Co., TN
S636 1776 1799 Shrader\Virginia/Sevier Co, TN ?
B300 1772 1772 Boyd\Boyd's Creek, Sevier Co, TN/Boyd's Creek, Sevier Co, TN
W 24 September 1992

*/

/* End of Report */
