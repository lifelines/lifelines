/*
 * @progname    fami-grps.ll
 * @version     1993-01-12
 * @author      Stephen Woodbridge (woodbri@swoodbridge.com)
 * @category
 * @output      Text, 80 cols
 * @description
 *
 *    Program walks thru one's families and dumps information
 *    about each family. It prunes the tree so an individual is
 *    only output once. The program lists all children of the
 *    families as it walks the tree. The "*" marker on a child
 *    signifies the line of descent/ascent.
 *
 *    Output assumes 132 characters wide and 80 lines per page.
 *
 *    Issues:
 *
 *      o only one child is marked in line of descent regardless
 *        of the actual number of children one may descend from
 *      o notes or family group records grater than LPP are NOT
 *        paginated correctly
 *      o program does not walk thru descendants yet
 *      o does not output baptism or burial records
 *      o does not list other spouses of HUSBAND or WIFE
 *
 *    Copyright 1993 Stephen Woodbridge
 */
global(UNKNOWN)
global(DONE)
global(ILIST)
global(NLIST)
global(RVAL)
global(nl)
global(ff)
global(PAGED)
global(PAGENO)
global(INDEXT)
global(INDEXS)
global(LPP)
global(LC)
global(NLF)
global(NLH)
global(NLW)
global(ONCE)

proc main()
{
    table(DONE)
    table(INDEXT)
    indiset(INDEXS)
    list(ILIST)
    list(NLIST)
    list(RVAL)
    set(nl, "\n")
    set(ff, "\f")
    set(PAGED, 1)
    set(PAGENO, 0)
    set(LPP, 80)
    set(LC, 0)
    set(NLF, 0)
    set(NLH, 0)
    set(NLW, 0)
    set(ONCE, 1)

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
    if (PAGED) { call print_index() }
}

proc do_me(me, depth, max)
{
    call fam_group(parents(me), 1, me, depth)
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

proc fam_group(fam, notes, mchild, depth)
{
    if (fam)
    {
        call count_fgrp(fam, notes)
        call fg_hdr(fam, depth)
        call pparent(husband(fam), "HUSBAND:")
        col(6) "M: " long(marriage(fam)) nl
        call pparent(wife(fam), "   WIFE:")
        "CHILDREN:" nl
        children(fam, ch, nc)
        {
            insert(DONE, save(key(ch)), 1)
            call pchild(nc, ch, mchild)
        }
        if (notes)
        {
            call print_notes(husband(fam), "\nHusband: ", NLH)
            call print_notes(wife(fam), "\n   Wife: ", NLW)
        }
    }
    else
    {
        if (mchild)
        {
            call fg_hdr(fam, depth)
            call pparent(0, "HUSBAND:")
            col(6) "M:" nl
            call pparent(0, "   WIFE:")
            "CHILDREN:" nl
            insert(DONE, save(key(mchild)), 1)
            call pchild(1, mchild, mchild)
            if (notes)
            {
                call print_notes(mchild, "\n   Child: ", 0)
            }
        }
    }
}

proc addtoindex(me)
{
    addtoset(INDEXS, me, 1)
    if (l, lookup(INDEXT, key(me)))
    {
        enqueue(l, PAGENO)
        insert(INDEXT, save(key(me)), l)
    }
    else
    {
        list(l)
        enqueue(l, PAGENO)
        insert(INDEXT, save(key(me)), l)
    }
}

proc print_index()
{
    "\f------------------------ INDEX -----------------------------\n"
    nl
    namesort(INDEXS)
    forindiset(INDEXS, me, v, n)
    {
        call print_name(me, 1)
        pop(RVAL) col(50)
        set(first, 1)
        set(last, 0)
        forlist(lookup(INDEXT, key(me)), pg, n)
        {
            if (ne(last, pg))
            {
                if(first) { set(first, 0) }
                else { "," }
                d(pg)
                set(last, pg)
            }
        }
        nl
    }
}

proc fg_hdr(fam, depth)
{
    set(dash, " --------------------------- ")
    if (PAGED)
    {
        if (and(gt(NLF, LC), lt(NLF, LPP)))
        {
            set(PAGENO, add(PAGENO, 1))
            if (ONCE) { set(ONCE, 0) } else { ff }
            dash d(depth) dash col(80) "Page: " d(PAGENO) nl
            set(LC, sub(LPP, NLF))
        }
        else
        {
            dash d(depth) dash nl
            set(LC, sub(LC, NLF))
        }
    }
    else
    {
        dash d(depth) dash nl
    }
}

proc count_fgrp(fam, notes)
{
    set(cnt, 13)
    children(fam, ch, nc)
    {
        set(cnt, add(cnt, 3))
        set(cnt, add(cnt, nspouses(ch)))
    }
    set(NLF, cnt)

    call cnt_notes(husband(fam), notes)
    set(NLH, pop(RVAL))

    call cnt_notes(wife(fam), notes)
    set(NLW, pop(RVAL))
}

proc cnt_notes(me, notes)
{
    set(c, 0)
    if (and(me, notes))
    {
        fornodes(inode(me), node)
        {
            if (not(strcmp("NOTE", tag(node))))
            {
                set(c, add(c, 1))
                fornodes(node, next)
                {
                    set(c, add(c, 1))
                }
            }
        }
    }
    if (c) { set(c, add(c, 2)) }
    push(RVAL, c)
}

proc pparent(me, hdr)
{
    if(me)
    {
        call get_refn(me)
        call print_name(me, 1)
        hdr col(10) pop(RVAL) col(55) "[" key(me) "]" col(62) pop(RVAL) nl
        col(6) "B:" col(10) long(birth(me)) nl
        col(6) "D:" col(10) long(death(me)) nl
        call addtoindex(me)
        if (fam, parents(me))
        {
            if (i, husband(fam))
            {
                call get_sdates(i)
                call print_name(i, 1)
                col(10) "FA:" col(15) pop(RVAL) col(60) pop(RVAL) nl
                call addtoindex(i)
            }
            if (i, wife(fam))
            {
                call get_sdates(i)
                call print_name(i, 1)
                col(10) "MO:" col(15) pop(RVAL) col(60) pop(RVAL) nl
                call addtoindex(i)
            }
        }
    }
    else
    {
        hdr nl col(6) "B:" nl col(6) "D:" nl
    }
}

proc pchild(num, me, markme)
{
    if (eq(me, markme)) { set(m, "*") } else { set(m, " ") }
    call print_name(me, 1)
    call rjt(num, 2)
    pop(RVAL) m sex(me) col(8) pop(RVAL) col(55) "[" key(me) "]" nl
    col(6) "B:" col(10) long(birth(me)) nl
    call addtoindex(me)
    spouses(me, sp, fam, nf)
    {
        call print_name(sp, 0)
        call addtoindex(sp)
        col(6) "M:" d(nf) col(10) long(marriage(fam))
            " TO " pop(RVAL) " [" key(sp) "]" nl
    }
    col(6) "D:" col(10) long(death(me)) nl
}

proc print_notes(me, string, nlines)
{
    if (me)
    {
        call paginate_notes(nlines)
        call addtoindex(me)
        set(hdr, 1)
        fornodes( inode(me), node)
        {
            if (not(strcmp("NOTE", tag(node))))
            {
                if (hdr)
                {
                    call print_name(me, 1)
                    string pop(RVAL) " [" key(me) "]" nl
                    set(hdr, 0)
                }
                col(8) value(node) nl
                fornodes(node, next)
                {
                    col(8) value(next) nl
                }
            }
        }
    }
}

proc paginate_notes(nlines)
{
    if (PAGED)
    {
        if (and(gt(nlines, LC), lt(nlines, LPP)))
        {
            set(PAGENO, add(PAGENO, 1))
            ff col(80) "Page: " d(PAGENO) nl
            set(LC, sub(LPP, add(nlines, 1)))
        }
        else
        {
            set(LC, sub(LC, nlines))
        }
    }
}


proc print_name (me, last)
{
    call get_title(me)
    push(RVAL, save(concat(fullname(me, 1, not(last), 45), pop(RVAL))))
}

proc get_refn (me)
{
    fornodes( inode(me), node)
    {
        if (not(strcmp("REFN", tag(node))))
        {
        set(refn, node)
        }
    }
    if (refn) { push(RVAL, save(value(refn))) }
    else { push(RVAL, "") }
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

proc rjt(n, w)
{
    if (lt(n, 10)) { set(d, 1) }
    elsif (lt(n, 100)) { set(d, 2) }
    elsif (lt(n, 1000)) { set(d, 3) }
    elsif (lt(n, 10000)) { set(d, 4) }
    else  { set(d, 5) }
    if (lt(d, w))
        { set(pad, save( trim("      ", sub(w, d)))) }
    else
        { set(pad, "") }
    push(RVAL, save( concat(pad, save(d(n)))))
}
