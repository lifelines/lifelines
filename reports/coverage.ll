/*
 * @progname       coverage.ll
 * @version        4
 * @author         Wetmore, Woodbridge, Eggert
 * @category       
 * @output         Text
 * @description
 *
 * Display percentage of ancestors of each generation discovered

   coverage -- Displays "ancestor coverage," that is, what percentage of
   ancestors have been discovered for each generation back in time.

   First version by T. Wetmore, 21 February 1994
   2nd   version by S. Woodbridge, 6 March 1994
   3rd   version by J. Eggert, 7 March 1994
   4th   version by J. Eggert, 9 November 1998
*/

proc main ()
{
    getindi(person0, "Enter person to compute ancestor coverage for.")
    print("Collecting data .... \n")

    "Ancestor Coverage Table for " name(person0) "\n\n"
    col(1) "Gen" col(9) "Total" col(19) "Found"
    col(30) "(Diff)" col(38) "Percentage\n\n"

    list(ilist)
    list(glist)
    table(dtable)
    enqueue(ilist, person0)
    enqueue(glist, 1)
    set(g,0) set(d,0) set(gsum,0) set(dsum,0) set(totpos,1)
    set(oldgen,1)
    while(person, dequeue(ilist)) {
        set(gen, dequeue(glist))
        if (ne(gen,oldgen)) {
            call printgen(oldgen,g,d,totpos)
            set(gsum,add(gsum,g))
            set(dsum,add(dsum,d))
            set(g,0)
            set(d,0)
            set(totpos,mul(totpos,2))
            set(oldgen,gen)
        }
        incr(g)
        if (not(lookup(dtable, key(person)))) {
            insert(dtable, key(person), gen)
            incr(d)
        }
/*      print(name(person), "\n")       */
        incr(gen)
        if (par,father(person)) {
            enqueue(ilist, par)
            enqueue(glist, gen)
        }
        if (par,mother(person)) {
            enqueue(ilist, par)
            enqueue(glist, gen)
        }
    }
    set(gsum,add(gsum,g))
    set(dsum,add(dsum,d))
    call printgen(oldgen,g,d,totpos)
    "\n"
    call printgen(0,gsum,dsum,0)
}

proc printgen(gen,g,d,tot) {
    if (tot) {
        col(1) rjustify(d(sub(gen,1)),3)
        col(6) if (lt(gen,31)) { rjustify(d(tot),8) }
    }
    else { col(1) "all" }
    col(16) rjustify(d(g),8)
    if (ne(g,d)) { col(26) rjustify(concat("(",d(d),")"),10) }
    if (and(tot,lt(gen,31))) { col(38)
        set(u, mul(g, 100))
        set(q, div(u, tot))
        set(m, mod(u, tot))
        set(m, mul(m, 100))
        set(m, div(m, tot))
        rjustify(d(q),3) "." if (lt(m, 10)) {"0"} d(m) " %"
    }
    "\n"
}
