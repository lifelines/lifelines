/*
 * @progname    altern.ll
 * @version     2.0
 * @author      Rafal T. Prinke
 * @category
 * @output      Text
 * @description
 *      find the longest line of alternating male/female links
 *
        v.1.0  Rafal T. Prinke - 14 APR 1997
        v.2.0  Rafal T. Prinke - 16 NOV 1999
*/

global(who)
global(was)
global(final)

proc main()

{
        set(final,0)
        list(who)
        table(was)
        forfam(f, y) {
                if(eq(nchildren,0)) {
                        if(husband(f)) {
                                call line(husband(f))
                                insert(was, key(husband(f)), 1)
                        }
                        if(wife(f))    {
                                call line(wife(f))
                                insert(was, key(wife(f)), 1)
                        }
                }
        }

"The longest alternating ancestral lines are: \n\n"

        while(not(empty(who))) {
                set(n, dequeue(who))
                set(count, 1)
                d(count) ". " name(n, 0) "\n"
                while(parents(n)) {
                        set(count, add(count, 1))
                        if (eqstr(sex(n),"M")) {
                                set(n, mother(n))
                        }
                        else { set(n, father(n)) }
                        d(count) ". " name(n, 0) "\n"
                }
        "\n"
        }
}



proc line (x) {
        if(not(lookup(was,key(x)))) {
                set(p, x)
                set(count,1)
                while(parents(x)) {
                        if (eqstr(sex(x),"M")) {
                                set(x, mother(x))
                        }
                        else { set(x, father(x))
                        }
                        set(count,add(count,1))
                }
                if (eq(count, final)) {
                        enqueue(who, p)
                }
                if (gt(count, final)) {
                        list(who)
                        enqueue(who, p)
                        set(final, count)
                }
        }
}
