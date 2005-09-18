/*
 * @progname    dump-ances.ll
 * @version     1992-11
 * @author      Stephen Woodbridge
 * @category
 * @output      Text, 80 cols
 * @description
 *
 *      Program walks thru one's ancestors and dumps information
 *      about each family. It prunes the tree so an individual is
 *      only output once. It is a simple program that is easy to
 *      make changes to, if you want more or less info printed. I
 *      have included three date routines get_dates(), get_sdates(),
 *      and get_ldates for variations in the amount of event info that
 *      gets output to the file. The program lists all children of the
 *      families as it walks the tree. The ">>>>" marker on a child
 *      signifies the line of descent.
 *
 *      Writen by Stephen Woodbridge, Nov 1992
 */
global(UNKNOWN)
global(DONE)
global(ILIST)
global(NLIST)
global(RVAL)

proc main()
{
        table(DONE)
        list(ILIST)
        list(NLIST)
        list(RVAL)
        set(UNKNOWN, "____?____")

        getindi(me)
        getintmsg(max, " Maximum Depth :")
        enqueue(ILIST, me)
        enqueue(NLIST, 1)
        set(i, 1)
        while (me, dequeue(ILIST))
        {
                set(depth, dequeue(NLIST))
                if (not(lookup(DONE, key(me))))
                {
                        call do_me(me, depth, max)
                }
        }
}

proc do_me(me, depth, max)
{
        call out_me(me, depth)
        insert(DONE, save(key(me)), 1)
        if (le(add(depth, 1), max))
        {
                if (dad, father(me))
                {
                        enqueue(ILIST, dad)
                        enqueue(NLIST, add(depth, 1))
                }
                if (mom, mother(me))
                {
                        enqueue(ILIST, mom)
                        enqueue(NLIST, add(depth, 1))
                }
        }
}

proc out_me(me, depth)
{
        "-------------------- " d(depth) " --------------------\n"
        if (dad, father(me))
        {
                call get_sdates(dad)
                call print_name(dad, 1)
                pop(RVAL) col(45) pop(RVAL) "\n"
        }
        else { UNKNOWN "\n"}

        if (mom, mother(me))
        {
                call get_sdates(mom)
                call print_name(mom, 1)
                pop(RVAL) col(45) pop(RVAL) "\n"
        }
        else { UNKNOWN "\n"}

        if (fam, parents(me))
        {
                "  m. " long(marriage(fam)) "\n"

                children( fam, child, nchild)
                {
                        if (eq(me, child)) { ">>>> " } else { "     " }
                        call get_sdates(child)
                        call print_name(child, 1)
                        pop(RVAL) col(50) pop(RVAL) "\n"
                }
        }
        else
        {
                " m.\n"
                ">>>> "
                call get_sdates(me)
                call print_name(me, 1)
                pop(RVAL) col(50) pop(RVAL) "\n"
        }
}

proc print_name (me, last)
{
    call get_title(me)
    push(RVAL, save(concat(fullname(me, 1, not(last), 45), pop(RVAL))))
}

proc get_title (me)
{
    fornodes(inode(me), node)
    {
        if (not(strcmp("TITL", tag(node)))) { set(n, node) }
    }
    if (n) { push(RVAL, save(concat(" ", value(n)))) }
        else { push(RVAL, "") }
}

proc get_sdates (me)
{
    if (e, birth(me)) { set(b, save(concat("( ", short(e)))) }
        else { set(b, "( ") }
    if (e, death(me)) { set(d, save(concat(" - " , short(e)))) }
        else { set(d, " - ") }
    push(RVAL, save(concat(b, concat(d, " )"))))
}

proc get_ldates (me)
{
    if (e, birth(me)) { set(b, save(concat("( ", long(e)))) }
        else { set(b, "( ") }
    if (e, death(me)) { set(d, save(concat(" - " , long(e)))) }
        else { set(d, " - ") }
    push(RVAL, save(concat(b, concat(d, " )"))))
}

proc get_dates (me)
{
    if (e, birth(me)) { set(b, save(concat("( ", date(e)))) }
        else { set(b, "( ") }
    if (e, death(me)) { set(d, save(concat(" - " , date(e)))) }
        else { set(d, " - ") }
    push(RVAL, save(concat(b, concat(d, " )"))))
}

