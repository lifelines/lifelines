/*
 * @progname       timeline2.ll
 * @version        2.0
 * @author         Jones
 * @category       
 * @output         Text, 80/132 cols
 * @description    
 *
 *   This report creates one of the following timeline charts:
 *      1. Ascii timeline graph showing birth, marriage, and death events of
 *         selected individuals; shows which individuals were contemporaies.
 *      2. Ascii timeline chart data with the above information for use with
 *         the my timeline generation program or the today program.
 *
 *   timeline2 - create a timeline of select individuals
 *
 *   version one:  4 Sept 1993
 *   version two:  9 Sept 1993
 *
 *   Code by James P. Jones, jjones@nas.nasa.gov
 *
 *       Contains code from:
 *       "famrep4"  - By yours truly, jjones@nas.nasa.gov
 *       "tree1"    - By yours truly, jjones@nas.nasa.gov
 *
 *   This report works only with the LifeLines Genealogy program.
 *
 *   This report creates one of the following timeline charts:
 *
 *      1. Ascii timeline graph showing birth, marriage, and death events of
 *         selected individuals; shows which individuals were contemporaies.
 *
 *      2. Ascii timeline chart data with the above information for use with
 *         the my timeline generation program or the today program.
 *
 *      3. IN PROGRESS (postscript version of #1 above)
 *
 *   User selects individual to base the chart on; then selects from
 *   the following sets:
 *
 *              parents,
 *              children,
 *              spouses,
 *              ancestors,
 *              descendents,
 *              everyone
 *
 *   User selects start date (e.g. 1852) and end date for graph; graph size
 *   (80 or 132 col); and various options regarding who to display.
 *
 *   Note: If an indi is "alive" for more than MAXAGE years, this is flagged as
 *   "uncertain" with a question mark in graph after MAXAGE years from birth.
 *   One should check these individuals to determine where they are really
 *   that old, or if one can determine an approxmate date of death, etc.
 *
 *   Additional functionality will be added as soon as LL version 2.3.5
 *   is released. See comments below for details.
 *
 *   Sample output (#1 above) follows (start=1800; end=2000; 80 col; sort by
 *   name; show people with dates only):

         Name           1800 1820 1840 1860 1880 1900 1920 1940 1960 1980 2000
________________________|____|____|____|____|____|____|____|____|____|____|
AUSTIN, George W        |                    B****M************D
AUSTIN, Velma Cleo      |                              B***M*M**M**M***M*
BLAKE, Nancy Elizabeth  |              B****M***********D
HEFLIN, Wyatt           |**************D
HUNTER, Rebecca A.      |     B****M************D
JONES, Arvel Fred Jr.   |                                  B******M******
JONES, Arvel Fred Sr.   |                          B****M*************D
JONES, Charles Columbus |                  B*******************D
JONES, Sarah Frances    |                  B*****D
JONES, Wesley           |            B************************?
JORDAN, Mary Cardine    |             B****D
PHIPPEN, Rose Marie     |                                    B****M***M**
WILDE, Charles          |      B************************?
____, Sarah A           |********************?

Scale: 1 point = 4 years
Key: B=birthdate, M=marriage, D=deathdate, *=living, ?=uncertainity

 *
 *   Output from #1 can be sorted using the sort(1) command, for example, the
 *   following command will sort the above output by birthdate:
 *
 *              sort -t\| +1 filename
 *
 *   where FILENAME is a file contain the above sample data, would produce:

PHIPPEN, Rose Marie     |                                    B****M***M**
JONES, Arvel Fred Jr.   |                                  B******M******
AUSTIN, Velma Cleo      |                              B***M*M**M**M***M*
JONES, Arvel Fred Sr.   |                          B****M*************D
AUSTIN, George W        |                    B****M************D
JONES, Charles Columbus |                  B*******************D
JONES, Sarah Frances    |                  B*****D
BLAKE, Nancy Elizabeth  |              B****M***********D
JORDAN, Mary Cardine    |             B****D
JONES, Wesley           |            B************************?
WILDE, Charles          |      B************************?
HUNTER, Rebecca A.      |     B****M************D
____, Sarah A           |********************?
HEFLIN, Wyatt           |**************D

 *
 *
 */

/*
 * timeline
 */

global(startdate)
global(enddate)
global(curyear)
global(linnum)
global(linpos)
global(offset)
global(years)
global(scale)
global(count)
global(indnum)
global(showall)
global(MAXYEAR)
global(MAXAGE)

proc main ()
{
        set(MAXYEAR, 2020)   /* the distant future */
        set(MAXAGE, 90)      /* if birth/death dates unknown, guess age */
        set(indnum,0)
        set(startdate, 0)
        set(enddate, 0)
        set(linnum, 1)
        set(linpos, 1)
        list(plist)
        while (eq(indi, NULL)) {
            getindi(indi)
        }
        while (and(ne(gra,1), ne(gra,2))) {
                getintmsg(gra,"Select timeline (1) graph, (2) chart:")
        }
        set(valid,0)
        while (eq(valid,0)) {
            print("Graph (1) parents, (2) children, (3) spouses")
            print(nl())
            print("      (4) ancestors, (5) descendents, (6) everyone")
            print(nl())
            getintmsg(ltype,"Choose subset of individuals: ")
            if (and(ge(ltype,1), le(ltype,6))) {
                set (valid, 1)
            }
        }
        while (and(ne(showall,1), ne(showall,2))) {
            getintmsg(showall,"Include people without dates (1) no, (2) yes: ")
        }
        if (eq(gra,1)) {
            pagemode(1, 200)
            set(startdate,0)
            set(enddate,0)
            while(le(enddate, startdate)) {
                set(startdate,0)
                set(enddate,0)
                while (or(le(startdate,0),gt(startdate,MAXYEAR))) {
                  getintmsg(startdate,"Enter start date for graph, e.g. 1800: ")
                }
                while (or(le(enddate,0),gt(startdate,MAXYEAR))) {
                    getintmsg(enddate, "Enter end date for graph, e.g. 1950: ")
                }
                if (le(enddate, startdate)) {
                        print("End date exceeds start date. Re-enter dates.")
                        print(nl())
                }
            }
            while (and(ne(size,1), ne(size,2))) {
                getintmsg(size,"Select graph size (1) 80 col, (2) 132 col: ")
            }
            while (and(ne(order,1), ne(order,2))) {
                getintmsg(order,"Order by (1) family group, (2) last name: ")
            }
            if (eq(size, 1)) {
                set(offset, sub(80, 26))
            }
            else {
                set(offset, sub(130, 26))
            }
            set(years, sub(enddate, startdate))

            set(scale, div(years, offset))
            if (gt(mod(years, offset), 0)) {
                set(scale, add(scale, 1))
            }
            if (le(scale, 0)) {
                set(scale, 1)
            }

            print("Scale: 1 point = ")
            print(d(scale))
            print(" years")

            call datelin()
            pageout()

            call header()
            pageout()
        }

        indiset(idx)
        addtoset(idx,indi,n)

        if (eq(ltype, 1)) {
            set(idx, parentset(idx))
            addtoset(idx,indi,n)
        }
        if (eq(ltype, 2)) {
            set(idx, childset(idx))
            addtoset(idx,indi,n)
        }
        if (eq(ltype, 3)) {
            set(idx, spouseset(idx))
            addtoset(idx,indi,n)
        }
        /* this will work in LL version 2.3.5 ...
        if (eq(ltype, 4)) {
            set(idx, siblingset(idx))
            addtoset(idx,indi,n)
        }*/
        if (eq(ltype, 4)) {
            set(idx, ancestorset(idx))
            addtoset(idx,indi,n)
        }
        if (eq(ltype, 5)) {
            set(idx, descendentset(idx))
            addtoset(idx,indi,n)
        }
        if (eq(ltype, 6)) {
            forindi(indiv,n) {
                    addtoset(idx,indiv,n)
            }
        }
        if (eq(gra,1)) {
            /* this will work in LL version 2.3.5 ...
            if (eq(lengthset(idx), 0)) {
                print("This set contains no individuals, please try again.")
                print(" ")
            }
            else {
            */
                if (eq(order, 2)) {
                        namesort(idx)
                }
                forindiset(idx,indiv,v,n) {
                        set(indnum, add(indnum,1))
                        call graph(indiv) /* outputs a 1 line "page" for each */
                }
                linemode()
                call printkey()

            /*}*/
        }
        else {
                linemode()
                forindiset(idx,indiv,v,n) {
                        call timeline(indiv)
                }
        }
}

proc datelin()
{
        set(linpos, 10)
        pos(linnum, linpos)
        "Name"
        set(linpos, 25)
        set(count, mul(scale, 5))
        set(curyear, sub(startdate, mod(startdate, count)))
        while (le(curyear, enddate)) {
                pos(linnum, linpos)
                d(curyear)
                set(curyear, add(curyear,count))
                set(linpos, add(linpos, 5))
        }
        set(curyear, sub(curyear,count))
}

proc header()
{
        set(tmpyear, sub(startdate, mod(startdate, count)))
        pos(linnum, 1)
        "________________________"
        set(linpos, 25)
        set(i, 25)
        while (le(tmpyear, curyear)) {
                set(j, 0)
                while (lt(j, count)) {
                        pos(linnum, linpos)
                        if (or(eq(i, 25), eq(mod(i, 5),0))) { "|" }
                        else {
                                if (lt(tmpyear, curyear)) { "_" }
                        }
                        set(j, add(j,scale))
                        set(linpos, add(linpos, 1))
                        set(i, add(i,scale))
                }
                set(tmpyear, add(tmpyear,count))
        }
}

proc graph(indi)
{
        set(NOINFO, 0)
        set(showit, 0)
        set(linnum, 1)
        set(linpos, 1)
        pos(linnum, linpos)

        if (eq(mod(indnum,10),0)) { print(".") }

        /* birth event */
        set(start, strtoint(year(birth(indi))))
        if (eq(start, 0)) {
                set(start, strtoint(year(baptism(indi))))
        }

        /* marriage event(s) */
        list(mlist)
        spouses(indi, svar, fvar, no) {
                set(tdate, strtoint(year(marriage(fvar))))
                if (ne(tdate,0)) {
                        enqueue(mlist, tdate)
                }
        }
        set(myear, dequeue(mlist))

        /* death event */
        set(end, strtoint(year(death(indi))))
        if (eq(end, 0)) {
                set(end, strtoint(year(burial(indi))))
        }


        /* do we have enough info to continue? */
        set(Bunknown, 0)
        set(Dunknown, 0)
        if (and(eq(start, 0),eq(end, 0))) { set(NOINFO,1) }
        else {
                if (eq(start, 0)) {
                        set(start,sub(end, MAXAGE))
                        set(Bunknown, 1)
                }
                if (eq(end, 0)) {
                        set(end, add(start, MAXAGE))
                        set(Dunknown, 1)
                }
        }

        if (or(eq(showall,2),eq(NOINFO,0))) {
            fullname(indi, 1, 0, 24)
            set(linpos, 25)
        }
        if (eq(NOINFO, 0)) {

            set(year, startdate)
            set(loop, 1)
            set(thisyear, strtoint(year(gettoday())))
            if (le(thisyear, enddate)) {
                set(stopdate, thisyear)
            }
            else { set(stopdate, enddate) }
            set(last, 0)
            while (le(year, stopdate)) {
                pos(linnum, linpos)
                if (lt(year, start)) {
                        if (eq(last,0)) { " " }
                }
                if (gt(year, end)) {
                        if (eq(last,0)) { " " }
                }
                if (eq(year, start)) {
                        if (eq(Bunknown,1)) { "?" }
                        else { "B" }
                        set(last, 1)
                }
                if (eq(year, end)) {
                        if (eq(Dunknown,1)) { "?" }
                        else { "D" }
                        set(last, 1)
                }
                if (and(gt(year, start), le(year, end))) {
                        if (eq(year, myear)) {
                                "M"
                                set(last, 1)
                                set(myear, dequeue(mlist))
                        }
                }
                if (and(gt(year, start), lt(year, end))) {
                    if (eq(last,0)) { "*" }
                }

                set(year, add(year, 1))
                if (eq(loop, scale)) {
                        set(loop, 1)
                        set(last, 0)
                        set(linpos, add(linpos, 1))
                }
                else {
                        set(loop, add(loop, 1))
                }
            }
            set(showit, 1)
        }
        if (or(eq(showall,2),eq(showit,1))) {
            set(linpos, 25)
            pos(linnum,linpos)
            "|"
            pageout()
        }
}

proc printkey()
{
        nl()
        "Scale: 1 point = " d(scale)
        if (eq(scale,1)) { " year" }
        else { " years" }
        nl()
        "Key: B=birthdate, M=marriage, D=deathdate, *=living, ?=uncertainity"
        nl()
}


proc timeline(indi)
{
        dayformat(1)
        monthformat(1)
        dateformat(6)

        set(tdate, date(birth(indi)))
        if (strcmp(tdate,NULL)) {
                "B" stddate(birth(indi)) " " name(indi) nl()
        }
        set(tdate, date(baptism(indi)))
        if (strcmp(tdate,NULL)) {
                "C" stddate(baptism(indi)) " " name(indi) nl()
        }


        spouses(indi, svar, fvar, no) {
                set(tdate, date(marriage(fvar)))
                if (strcmp(tdate,NULL)) {
                        "M" stddate(marriage(fvar)) " "
                        if (eq(strcmp(sex(indi), "M"),0)) {
                                name(indi) " to " name(svar) nl()
                        }
                        else {
                                name(svar) " to " name(indi) nl()
                        }
                }
        }

        set(tdate, date(death(indi)))
        if (strcmp(tdate,NULL)) {
                "D" stddate(death(indi)) " " name(indi) nl()
        }
        set(tdate, date(burial(indi)))
        if (strcmp(tdate,NULL)) {
                "F" stddate(burial(indi)) " " name(indi) nl()
        }
}
