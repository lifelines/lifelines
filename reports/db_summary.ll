/*
 * @progname    db_summary.ll
 * @version     1
 * @author      Eggert
 * @category
 * @output      Text
 * @description

This program gives you summary statistics on your database.  It
calculates the number of birth, baptism, marriage, death, and burial
events, and gives the distribution over centuries of birth/baptisms,
death/burials, and marriages.  It tells you how many different names
(given names and surnames separately) there are in the database, and
how many persons have no surname in the database.

db_summary - a LifeLines database summary program
        by Jim Eggert (eggertj@ll.mit.edu)
        Version 1, 29 March    1995  Initial release
*/

proc main() {
    table(surnames)
    table(givens)
    list(bcents)
    list(dcents)
    list(mcents)

    list(namelist)

    set(nsurnames,0)
    set(ngivens,0)
    set(nnosurnames,0)
    set(nnogivens,0)
    set(nemptysurnames,0)
    set(nbirths,0)
    set(nbaptisms,0)
    set(nmarrs,0)
    set(ndeaths,0)
    set(nburials,0)

    print("Collecting individual statistics...")
    forindi(person,pnum) {
/* Do individual event statistics */
        set(by,0)
        if (b,birth(person)) {
            incr(nbirths)
            extractdate(b,bd,bm,by)
        }
        if (b,baptism(person)) {
            incr(nbaptisms)
            if (not(by)) { extractdate(b,bd,bm,by) }
        }
        call increment_century(bcents,by)

        set(dy,0)
        if (d,death(person)) {
            incr(ndeaths)
            extractdate(d,dd,dm,dy)
        }
        if (d,burial(person)) {
            incr(nburials)
            if (not(dy)) { extractdate(d,dd,dm,dy) }
        }
        call increment_century(dcents,dy)

/* Do name statistics */
        extractnames(inode(person),namelist,nnames,isurname)
        if (not(isurname)) { incr(nnosurnames) }
        forlist(namelist,name,nnum) {
            if (eq(nnum,isurname)) {
                if (not(lookup(surnames,name))) {
                    incr(nsurnames)
                    insert(surnames,save(name),save(key(person)))
                }
                if (not(strcmp(name,""))) {
                    incr(nemptysurnames)
                }
                if (not(name)) { incr(nnosurnames) }
            }
            else {
                if (not(lookup(givens,name))) {
                    incr(ngivens)
                    insert(givens,save(name),save(key(person)))
                }
            }
        }
    }

    print("done.\nCollecting family statistics...")
    forfam(family,fnum) {
        set(by,0)
        if (m,marriage(family)) {
            incr(nmarrs)
            extractdate(m,md,mm,my)
            call increment_century(mcents,my)
        }
    }

    print("done.\nGenerating report...")
    "The database " database() " contains:\n"
    d(pnum) " individuals\n"
    d(nsurnames) " unique surnames\n"
    d(ngivens) " unique given names\n"
    d(nemptysurnames) " individuals with empty surnames\n"
    d(nnosurnames) " individuals with no surname\n"
    d(nbirths) " birth events\n"
    d(nbaptisms) " baptism events\n"
    "Birth/baptism events distributed by century as\n"
    call list_centuries(bcents)
    d(ndeaths) " death events\n"
    d(nburials) " burial events\n"
    "Death/burial events distributed by century as\n"
    call list_centuries(dcents)
    "\n"
    d(fnum) " families\n"
    d(nmarrs) " marriage events distributed by century as\n"
    call list_centuries(mcents)
    print("done.\n")
}

proc increment_century(centuries,year) {
    if (year) {
        set(century,div(year,100))
        if (not(length(centuries))) {
            enqueue(centuries,century)
            enqueue(centuries,1)
        }
        else {
            set(first_century,dequeue(centuries))
            while (lt(century,first_century)) {
                requeue(centuries,0)
                set(first_century,sub(first_century,1))
            }
            set(index,add(1,sub(century,first_century)))
            setel(centuries,index,add(getel(centuries,index),1))
            requeue(centuries,first_century)
        }
    }
}

proc list_centuries(centuries) {
    set(century,dequeue(centuries))
    while (count,dequeue(centuries)) {
        "    " d(century) "00s  " d(count) "\n"
        incr(century)
    }
}
