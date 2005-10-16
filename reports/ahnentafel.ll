/* 
 * @progname       ahnentafel.ll
 * @version        1
 * @author         Wetmore
 * @category       
 * @output         Text
 * @description    
 *
 * Generate an ahnentafel chart for the selected person.
 * 
 * ahnentafel -- Generate an ahnentafel chart */
/* Tom Wetmore        */
/* Version 1, 4/27/95 */

proc main ()
{
        getindimsg(indi, "Whose Ahnentafel do you want?")
        if (not(indi)) { return() }
        "Ahnentafel of " name(indi) "\n\n"
        print("Computing ahnentafel of ", name(indi), "\n",
                "  Dots show persons per generation\n\n")
        list(ilist)
        list(alist)
        list(glist)
        table(ktab)
        enqueue(ilist,indi)
        enqueue(alist,1)
        enqueue(glist,1)
        set(gen, 0)
        while(indi,dequeue(ilist)) {
                set(ahnen, dequeue(alist))
                set (newgen, dequeue(glist))
                if (ne(gen, newgen)) {
                        "Generation " upper(roman(newgen)) ".\n\n"
                        print("\n", roman(newgen), " ")
                        set(gen, newgen)
                }
                set(before, lookup(ktab, key(indi)))
                if (before) {
                        d(ahnen) ". Same as " d(before) ".\n"
                } else {
                        print(".")
                        insert(ktab, save(key(indi)), ahnen)
                        d(ahnen) ". " name(indi) "\n"
                        if (e, birth(indi)) { "    b. " long(e) "\n" }
                        if (e, death(indi)) { "    d. " long(e) "\n" }
                }
                "\n"
                if (par,father(indi)) {
                        enqueue(ilist, par)
                        enqueue(alist, mul(2,ahnen))
                        enqueue(glist, add(gen, 1))
                }
                if (par,mother(indi)) {
                        enqueue(ilist, par)
                        enqueue(alist, add(1,mul(2,ahnen)))
                        enqueue(glist, add(gen, 1))
                }
        }
}
