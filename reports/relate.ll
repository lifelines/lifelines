/*
 * @progname       relate.ll
 * @version        1.0
 * @author         Wetmore
 * @category       
 * @output         Text
 * @description    
 *
 *  Finds a shortest path between two persons in a LifeLines database.
 *  Inspiration from Jim Eggert's relation program.


relate - Finds a shortest path between two persons in a LifeLines
        database.
        by Tom Wetmore (ttw@petrel.att.com)
        Inspiration from Jim Eggert's relation program
        Version 1, 07 September 1993
*/

proc main ()
{
        getindimsg(from, "Please identify starting person.")
        getindimsg(to, "Please identify ending person.")
        if (and(from, to)) {
                print("Computing relationship between:\n  ")
                print(name(from)) print(" and ")
                print(name(to)) print(".\n\nThis may take awhile -- ")
                print("each dot is a person.\n")

                set(fkey, save(key(from)))
                set(tkey, save(key(to)))
                call relate(tkey, fkey)
        } else {
                print("We're ready when you are.")
        }
}

global(links)
global(rels)
global(klist)

proc relate (fkey, tkey)
{
        table(links)    /* table of links back one person */
        table(rels)     /* table of relationships back one person */
        list(klist)     /* list of persons not linked back to */

        insert(links, fkey, fkey)
        insert(rels, fkey, ".")
        enqueue(klist, fkey)
        set(again, 1)

        while (and(again, not(empty(klist)))) {
                set(key, dequeue(klist))
                set(indi, indi(key))
                call include(key, father(indi), ", father of")
                call include(key, mother(indi), ", mother of")
                families(indi, fam, spouse, num1) {
                        children(fam, child, num2) {
                                call include(key, child, ", child of")
                        }
                        if (spouse) {
                                call include(key, spouse, ", spouse of")
                        }
                }
                if (fam, parents(indi)) {
                        children(fam, child, num2) {
                                call include(key, child, ", sibling of")
                        }
                }
                if (key, lookup(links, tkey)) {
                        call foundpath(tkey)
                        set(again, 0)
                }
        }
        if (again) {
                print("They are not related to one another.")
        }
}

proc include (key, indi, tag)
{
        if (and(indi, not(lookup(links, key(indi))))) {
                print(".")
                set(new, save(key(indi)))
                insert(links, new, key)
                insert(rels, new, tag)
                enqueue(klist, new)
        }
}

proc foundpath (key)
{
        print("\n\nA relationship between them was found:\n\n")
        set(again, 1)
        while (again) {
                print("  ")
                print(name(indi(key)))
                print(lookup(rels, key))
                print("\n")
                set(new, lookup(links, key))
                if (eq(0, strcmp(key, new))) {
                        set(again, 0)
                } else {
                        set(key, new)
                }
        }
}
