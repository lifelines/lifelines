/*
 * @progname    count_dup.ll
 * @version     1.0
 * @author      anon
 * @category
 * @output      Text
 * @description
 *              Count dups among ancestors?
 */

global(cnttab)
global(indtab)
global(undone)
global(allind)
global(maxcount)
global(maxindi)

proc main() {

    list(undone)
    list(allind)
    table(cnttab)
    table(indtab)

    getindi(person)
    set(maxcount,0)
    set(maxindi,person)
    call addaperson(person)
    set(c,0)

    while (person,dequeue(undone)) {
        incr(c)
        /* print(d(c)," ",key(person),"\n") */
        if(eq(mod(c,1000), 0)) {
         print(d(c)," ",d(maxcount)," ",key(maxindi)," ",name(maxindi),"\n")
        }
        if (p,father(person)) { call addaperson(p) }
        if (p,mother(person)) { call addaperson(p) }
    }

    while(p,dequeue(allind)) {
        set(count,lookup(cnttab,key(p)))
        d(count) " " key(p) " " name(p) " " title(p) "\n"
    }
}

proc addaperson(p)
{
        enqueue(undone,p)
        set(count,lookup(cnttab,key(p)))
        if(ne(count,0)) {
          set(count, add(count,1))
          if(gt(count, maxcount)) {
                set(maxcount, count)
                set(maxindi, p)
          }
        } else    {
          set(count,1)
          insert(indtab, key(p), p)
          enqueue(allind,p)
        }
        insert(cnttab, key(p), count)
}
