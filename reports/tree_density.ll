/*
 * @progname       tree_density.ll
 * @version        1
 * @author         Jim Eggert (eggertj@atc.ll.mit.edu)
 * @category       
 * @output         Text
 * @description

This program calculates the weight of each node in an ancestral tree.
The weight of a node is given by the number of people in the database
that are most closely related to that node.  The degree of relation is
measured in hops, where a hop is a step to a parent, spouse, sibling,
or child.  Thus this program allows you to get an idea of where most
of the research has been done, where large numbers of cousins hook in
to the database, and where more work may be needed.

The output of the program is a simple ahnentafel with the ahnentafel
number, node weight, and key and name of the ancestors.

tree_density - a LifeLines program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  15 February 1996
*/

global(plist)
global(mark)
global(anccounts)

proc include(person,anckey)
{
    if (person) {
        set(pkey,key(person))
        if (not(lookup(mark,pkey))) {
            set(skey,save(pkey))
            set(sanckey,save(anckey))
            insert(mark,skey,sanckey)
            set(count,lookup(anccounts,sanckey))
            insert(anccounts,sanckey,add(count,1))
            enqueue(plist,skey)
        }
    }
}

proc main ()
{
    table(mark)
    table(anccounts)
    list(plist)
    list(alist)

    getindimsg(from_person,
        "Enter person to compute relation from:")
    set(from_key,save(key(from_person)))
    call include(from_person,from_key)

    set(counter,0)
    set(nextcount,0)

    while (pkey,dequeue(plist)) {
        incr(counter)
        if (ge(counter,nextcount)) {
            print(d(counter)," ")
            set(nextcount,add(nextcount,100))
        }

        set(person,indi(pkey))
        set(anckey,save(lookup(mark,pkey)))
        if (not(strcmp(pkey,anckey))) {
            call include(father(person),save(key(father(person))))
            call include(mother(person),save(key(mother(person))))
        }
        else {
            call include(father(person),anckey)
            call include(mother(person),anckey)
        }
        children(parents(person),child,cnum) {
            call include(child,anckey)
        }
        families(person,fam,spouse,pnum) {
            call include(spouse,anckey)
            children(fam,child,cnum) {
                call include(child,anckey)
            }
        }
    }

    "Ahnentafel" col(10) "  weight" col(20) "key" col(28) "name"
    enqueue(plist,from_key)
    enqueue(alist,1)
    while (pkey,dequeue(plist)) {
        set(a,dequeue(alist))
        set(p,indi(pkey))
        if (f,father(p)) {
            enqueue(plist,save(key(f)))
            enqueue(alist,add(a,a))
        }
        if (m,mother(p)) {
            enqueue(plist,save(key(m)))
            enqueue(alist,add(a,a,1))
        }
        d(a) col(10) rjustify(d(lookup(anccounts,pkey)),8)
        col(20) key(p) col(28) name(p) "\n"
    }
}
