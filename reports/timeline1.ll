/*
 *   timeline1
 *
 *   version one:  4 Sept 1993
 *
 *   Code by James P. Jones, jjones@nas.nasa.gov
 *
 *       Contains code from:
 *       "famrep4"  - By yours truely, jjones@nas.nasa.gov
 *       "tree1"    - By yours truely, jjones@nas.nasa.gov
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   This report creates one of the following timeline charts:
 *
 *      1. Ascii timeline showing birth, marriage, and death events of
 *         selected individuals; shows which individuals were contemporaies.
 *
 *      2. IN PROGRESS
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
 *   User selects start date (e.g. 1852) and end date for graph; as well as
 *   graph size (80 or 132 col).
 *
 *   Note: If an indi is "alive" for more than 100 years, this is flagged as
 *   "uncertain", with a question mark in the graph after 100 years from birth.
 *   One should check these individuals to determine where they are really
 *   that old, or if one can determine an approxmate date of death, etc.
 *
 *   Additional functionality will be added as soon as LL version 2.3.5
 *   is released.
 *
 *   Sample output follows (start=1800; end=2000; 80 columns):

         Name           1800 1820 1840 1860 1880 1900 1920 1940 1960 1980 2000
________________________|____|____|____|____|____|____|____|____|____|____|
AUSTIN, George W                             B****M************D
AUSTIN, Velma Cleo                                     B***M*M**M**M***M*
BLAKE, Nancy Elizabeth                 B****M***********D
HEFLIN, Wyatt           ***************D
HUNTER, Rebecca A.            B****M************D
JONES, Arvel Fred Jr.                                      B******M******
JONES, Arvel Fred Sr.                              B****M*************D
JONES, Charles Columbus                    B*******************D
JONES, George
JONES, Sarah Frances                       B*****D
JONES, Wesley                        B************************???????????
JORDAN, Mary Cardine                  B****D
PHIPPEN, Rose Marie                                          B****M***M**
WILDE, Charles                 B************************?????????????????
____, Sarah A           *********************????????????????????????????

Scale: 1 point = 4 years
Key: B=birthdate, M=marriage, D=deathdate, *=living, ?=uncertainity

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

proc main ()
{
        set(startdate, 0)
        set(enddate, 0)
        set(linnum, 1)
        set(linpos, 1)
        list(plist)
        pagemode(1, 200)
        getindi(indi)
        while (eq(indi, NULL)) {
            getindi(indi)
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
        while (le(startdate,0)) {
                getintmsg(startdate, "Enter start date for graph, e.g. 1852: ")
        }
        while (le(enddate,0)) {
                getintmsg(enddate, "Enter end date for graph, e.g. 1852: ")
        }
        while (and(ne(size,1), ne(size,2))) {
                getintmsg(size,"Select graph size (1) 80 col, (2) 132 col: ")
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

                                        /*      put at bottom, + key*/
        print("Scale: 1 point = ")
        print(d(scale))
        print(" years")

        call datelin()
        pageout()

        call header()
        pageout()

        indiset(idx)

        if (eq(ltype, 1)) {
                addtoset(idx,indi,n)
                set(idx, parentset(idx))
        }
        if (eq(ltype, 2)) {
                addtoset(idx,indi,n)
                set(idx, childset(idx))
        }
        if (eq(ltype, 3)) {
                addtoset(idx,indi,n)
                set(idx, spouseset(idx))
        }
        /*if (eq(ltype, 4)) {
                addtoset(idx,indi,n)
                set(idx, siblingset(idx))
        }*/
        if (eq(ltype, 4)) {
                addtoset(idx,indi,n)
                set(idx, ancestorset(idx))
        }
        if (eq(ltype, 5)) {
                addtoset(idx,indi,n)
                set(idx, descendentset(idx))
        }
        if (eq(ltype, 6)) {
                forindi(indiv,n) {
                        addtoset(idx,indiv,n)
                }
        }
        /* lengthset() is not in this version of LL; wait until Ver. 2.3.5
        if (eq(lengthset(idx), 0)) {
                print("This set contains no individuals, please try again.")
                print(" ")
        }
        else {
        */
                namesort(idx)
                forindiset(idx,indiv,v,n) {
                        call graph(indiv) /* outputs a 1 line "page" for each */
                        pageout()         /* entry on graph */
                }
                linemode()
                call printkey()
        /*}*/
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
        set(linnum, 1)
        set(linpos, 1)
        pos(linnum, linpos)
        fullname(indi, 1, 0, 24)
        set(linpos, 25)
        print(".")

        /* birth event */
        set(start, strtoint(year(birth(indi))))
        if (eq(start, 0)) {
                set(start, strtoint(year(baptism(indi))))
        }
        if (eq(start, 0)) {
                set(unknown, 1)
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
        if (eq(end, 0)) {
                if (not(unknown)) {
                        set(end, add(enddate, 1))  /* assume he is alive */
                        /*set(start, sub()*/
                }
        }

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
                        "B"
                        set(last, 1)
                }
                if (eq(year, end)) {
                        "D"
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
                    if (eq(last,0)) {
                        if (ge(sub(end, start), 100)) {
                            if (le(end,stopdate)) { "*" }
                            else {
                                if (ge(year,add(start,100))) { "?" }
                                else { "*" }
                            }
                        }
                        else { "*" }
                    }
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
