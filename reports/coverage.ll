/*
 * @progname       coverage
 * @version        3.0
 * @author         Wetmore, Woodbridge, Eggert
 * @category       
 * @output         Text
 * @description    
 *

   Displays "ancestor coverage," that is, what percentage of
   ancestors have been discovered for each generation back in time.

   First version by T. Wetmore, 21 February 1994
   2nd   version by S. Woodbridge, 6 March 1994
   3rd   version by J. Eggert, 7 March 1994
*/

proc main ()
{
        getindi(indi0, "Enter person to compute ancestor coverage for.")
        print("Collecting data .... \n")
        list(ilist)
        list(glist)
        list(garray)
        table(dtable)
        list(darray)
        enqueue(ilist, indi0)
        enqueue(glist, 1)
        while(indi, dequeue(ilist)) {
                set(gen, dequeue(glist))
                set(i, getel(garray, gen))
                set(i, add(i, 1))
                setel(garray, gen, i)
                if (not(lookup(dtable, key(indi)))) {
                        insert(dtable, key(indi), gen)
                        set(i, getel(darray, gen))
                        set(i, add(i, 1))
                        setel(darray, gen, i)
                }
/*                print(name(indi), "\n")       */
                if (par,father(indi)) {
                        enqueue(ilist, par)
                        enqueue(glist, add(1, gen))
                }
                if (par,mother(indi)) {
                        enqueue(ilist, par)
                        enqueue(glist, add(1, gen))
                }
        }
        set(i, 1)
        set(tot, 1)
        set(num, getel(garray, i))
        set(dnum, getel(darray, i))
        set(numsum, num)
        set(dnumsum, dnum)
        "Ancestor Coverage Table for " name(indi0) "\n\n"
        col(1) "Gen" col(6) "Total" col(16) "Found"
        col(26) "(Diff)" col(38) "Percentage\n\n"
        while (num) {
                col(1) d(sub(i, 1))
                col(6) if (lt(i,31)) { d(tot) }
                col(16) d(num)
                if (ne(num, dnum)) { col(26) "(" d(dnum) ")" }
                if (lt(i,31)) { col(38)
                        set(u, mul(num, 100))
                        set(q, div(u, tot))
                        set(m, mod(u, tot))
                        set(m, mul(m, 100))
                        set(m, div(m, tot))
                        d(q) "." if (lt(m, 10)) {"0"} d(m) " %"
                        set(tot, mul(tot, 2))
                }
                set(i, add(i, 1))
                set(num, getel(garray, i))
                set(dnum, getel(darray, i))
                set(numsum, add(numsum, num))
                set(dnumsum, add(dnumsum, dnum))
                print(d(i), "  ", d(num), "\n")
        }
        "\n\n"
        col(1) "all" col(16) d(numsum)
        if (ne(numsum, dnumsum)) { col(26) "(" d(dnumsum) ")" }
        "\n"
}

