/*
 * @progname    all_anc_lines.ll
 * @version     2
 * @author      Tom Wetmore
 * @category
 * @output      Text
 * @description
 *
 * report all ancestral lines in a Register-like format
 *

   all_anc_lines  -- Shows all ancestral lines of a specified person using
   a pseudo-Register format.  The paternal line of the person is shown
   first; then the paternal line of his/her mother; then the paternal line
   of his/her paternal grandmother; and so on, in a depth-first manner.

   A new feature was added to follow maternal lines also.

   Future option -- breadth first versus depth first coverage -- easy to
   implement by changing the algorithm that builds dlist from a stack to a
   queue.

   by Tom Wetmore, ttw@beltway.att.com
   version 1, 14 Nov 1995
   version 2, 23 Nov 1995
*/

global(mlist)   /* list of pending key persons */
global(glist)   /* generations of pending key persons */
global(stable)  /* table of seen key persons */
global(dlist)   /* list of final key persons */
global(hlist)   /* list of final generations */
global(ilist)   /* list of isolated persons */
global(pat)
global(depth)
global(ftable)  /* list of shown families */
global(ptable)  /* table of printed persons */

proc main ()
{
        getindi(i, "Enter person whose full registry ancestry is wanted.")
        if (i) {
                list(menu)
                enqueue(menu, "Follow paternal lines; or")
                enqueue(menu, "Follow maternal lines.")
                set(m, menuchoose(menu, "Select whether to:"))
                if (eq(1, m)) { set(pat, 1) }
                else          { set(pat, 0) }
                list(menu)
/*
                enqueue(menu, "Output lines depth-first; or")
                enqueue(menu, "Output lines breadth-first.")
                set(m, menuchoose(menu, "Select whether to:"))
                if (eq(1, m)) { set(depth, 1) }
                else          { set(depth, 0) }
*/
                list(mlist)
                list(glist)
                table(stable)
                list(dlist)
                list(hlist)
                list(ilist)
                table(ftable)
                table(ptable)
                call doit(i)
        } else {
                print("Program not run.")
        }
}

proc doit (i)
{
        call makedlist(i)
        call genreport()
}

proc makedlist (i)
{
        enqueue(mlist, i)
        enqueue(glist, 1)
        while (p, dequeue(mlist)) {
                set(g, dequeue(glist))
                enqueue(dlist, p)
                enqueue(hlist, g)
                while (p) {
                        set(g, add(g, 1))
                        if (pat) {
                                if (m, mother(p)) {
                                        if (not(lookup(stable, key(m)))) {
                                                insert(stable, save(key(m)), 1)
                                                enqueue(mlist, m)
                                                enqueue(glist, g)
                                        }
                                }
                                set(p, father(p))
                        } else {
                                if (f, father(p)) {
                                        if (not(lookup(stable, key(f)))) {
                                                insert(stable, save(key(f)), 1)
                                                enqueue(mlist, f)
                                                enqueue(glist, g)
                                        }
                                }
                                set(p, mother(p))
                        }
                }
        }
}

proc genreport ()
{
        call nroffhead()
        forlist (dlist, p, n) {
                set(g, dequeue(hlist))
                if (not(lookup(ptable, key(p)))) {
                        if (pat) { set(q, father(p)) }
                        else     { set(q, mother(p)) }
                        if (q) {
                                call showline(p, g)
                        } else {
                                insert (ptable, save(key(p)), 1)
                                enqueue(ilist, p)
                        }
                }
        }
        forlist (ilist, p, n) {
                "ISOLATED PERSON " name(p) "\n"
        }
}

proc showline (p, g)
{
        if (pat) {
                call showsurnames(p)
                /*".NL\nPATERNAL LINE OF " upper(name(p)) "\n\n"*/
                print(surname(p), "  ")
        } else {
                ".NL\nMATERNAL LINE OF " upper(name(p)) "\n\n"
        }
        list(alist)
        if (pat) {
                while (f, father(p)) {
                        push(alist, p)
                        set(p, f)
                        set(g, add(g, 1))
                }
        } else {
                while (m, mother(p)) {
                        push(alist, p)
                        set(p, m)
                        set(g, add(g, 1))
                }
        }
        push(alist, p)
        set(a, pop(alist))
        while (a) {
                set(b, pop(alist))
                call dotwo(a, b, g)
                set(a, b)
                set(g, sub(g, 1))
        }
}

proc dotwo (a, b, g)
{
        /*".GN\nGENERATION " d(g) "\n\n"*/
        ".IN\n" d(g) ". "
        call longvitals(a)      /* show main line person */
        insert(ptable, save(key(a)), 1)

        if (pat) { set(c, mother(b)) }
        else     { set(c, father(b)) }
        if (pat) { set(d, father(c)) }
        else     { set(d, mother(c)) }

        if (and(c, not(d))) {
                call gammavitals(c, a)
                insert(ptable, save(key(c)), 1)
        }

        call dochildren(a, b)
        if (and(c, not(d))) {
                call gammachildren(c)
        }
}

proc nroffhead ()
{
    ".de hd\n'sp .8i\n..\n"
    ".de fo\n'bp\n..\n"
    ".wh 0 hd\n.wh -.8i fo\n"
    ".de CH\n"
    ".sp\n"
    ".in 11n\n"
    ".ti 0\n"
    "\\h'3n'\\h'-\\w'\\\\$1'u'\\\\$1\\h'6n'\\h'-\\w'\\\\$2'u'\\\\$2\\h'1n'\n"
    "..\n"

    ".de IN\n.sp\n.in 0\n..\n"
    ".de NL\n.br\n.ne 2i\n.sp 2\n.in 0\n.ce\n..\n"
    ".de GN\n.br\n.ne 2i\n.sp 2\n.in 0\n.ce\n..\n"
    ".de P\n.sp\n.in 0\n.ti 5\n..\n"
    ".po 5\n"
    ".ll 7i\n"
    ".ls 1\n"
    ".na\n"
}

proc dochildren (i, c)
{
        if (c) { set(ckey, save(key(c))) }
        else   { set(ckey, "JUNK") }
        families (i, f, s, n) {
            ".P\n"
            if (s) { set(sname, save(name(s))) }
            else        { set(sname, "(_____)") }
            if (eq(0, nchildren(f))) {
                name(i) " and " sname
                " had no children.\n"
            } elsif (lookup(ftable, key(f))) {
                "Children of " name(i) " and " sname
                " listed under " sname ".\n"
/*
                children(f, k, m) {
                   if (not(strcmp(key(k), ckey))) {
                        ".CH (+) " roman(m) "\n"
                        call shortvitals(k)
                    } else {
                        ".CH \"\" " roman(m) "\n"
                        call shortvitals(k)
                    }

                }
*/
            } else {
                "Children of " name(i) " and " sname ":\n"
                children(f, k, m) {
                   if (not(strcmp(key(k), ckey))) {
                        /*print(name(k), "\n")*/
                        ".CH (+) " roman(m) "\n"
                        call shortvitals(k)
                    } else {
                        ".CH \"\" " roman(m) "\n"
                        call middlevitals(k)
                    }
                }
                insert(ftable, save(key(f)), 1)
            }
        }
}

proc shortvitals (i)
{
        name(i)
        set(b, birth(i))
        set(d, death(i))
        if (and(b, short(b))) { ", b. " short(b) }
        if (and(d, short(d))) { ", d. " short(d) }
        ".\n"
}

proc middlevitals (i)
{
        name(i) ".\n"
        set(e, birth(i))
        if(and(e,long(e))) { "Born " long(e) ".\n" }
        if (eq(1, nspouses(i))) {
                spouses(i, s, f, n) {
                        "Married"
                        call spousevitals(s, f)
                }
        } else {
                spouses(i, s, f, n) {
                        "Married " ord(n) ","
                        call spousevitals(s, f)
                }
        }
        set(e, death(i))
        if(and(e, long(e))) { "Died " long(e) ".\n" }
        set(p, 0)
}

proc longvitals (i)
{
        name(i) ".\n"
        set(e, birth(i))
        if(and(e,long(e))) { "Born " long(e) ".\n" }
        if (eq(1, nspouses(i))) {
                spouses(i, s, f, n) {
                        "Married"
                        call spousevitals(s, f)
                }
        } else {
                spouses(i, s, f, n) {
                        "Married " ord(n) ","
                        call spousevitals(s, f)
                }
        }
        set(e, death(i))
        if(and(e, long(e))) { "Died " long(e) ".\n" }
        set(p, 0)
        fornotes(inode(i), n) {
                if (not(p)) { ".P\n" set(p, 1) }
                n "\n"
        }
}

proc spousevitals (s, f)
{
        set(e, marriage(f))
        if (and(e, long(e))) { "\n" long(e) "," }
        "\n" name(s)
        set(e, birth(s))
        if (and(e, long(e)))  { ",\nborn " long(e) }
        set(e, death(s))
        if (and(e, long(e)))  { ",\ndied " long(e) }
        set(d, father(s))
        set(m, mother(s))
        if (or(d, m)) {
                ",\n"
                if (male(s))      { "son of " }
                elsif (female(s)) { "daughter of " }
                else              { "child of " }
        }
        if (d)         { name(d) }
        if (and(d, m)) { "\nand " }
        if (m)         { name(m) }
        ".\n"
}

proc gammavitals(a, c)
{
        set(n, nfamilies(a))
        set(m, mother(a))
        set(d, father(a))
        if (or(gt(n, 1), or(m, d))) {
                ".P\n" name(a) ", "
                if (or(d, m)) {
                        if (male(a))      { "son of " }
                        elsif (female(a)) { "daughter of " }
                        else              { "child of " }
                }
                if (d)         { name(d) }
                if (and(d, m)) { "\nand " }
                if (m)         { name(m) }
                if (or(d, m)) { ",\n" }
                if (gt(n, 1)) {
                        if (eq(1, nspouses(a))) {
                                spouses(a, s, f, n) {
                                        "Married "
                                        if (eqstr(key(c), key(s))) {
                                                name(s) ".\n"
                                        } else {
                                                call spousevitals(s, f)
                                        }
                                }
                        } else {
                                spouses(a, s, f, n) {
                                        "Married " ord(n) ","
                                        if (eqstr(key(c), key(s))) {
                                                name(s) ".\n"
                                        } else {
                                                call spousevitals(s, f)
                                        }
                                }
                        }
                ".\n"
                }
        }
}
proc gammachildren (p)
{
        families (p, f, s, n) {
                if (not(lookup(ftable, key(f)))) {
                        ".P\n"
                        if (s) { set(sname, save(name(s))) }
                        else   { set(sname, "(_____)") }
                        if (eq(0, nchildren(f))) {
                                name(p) " and " sname " had no children.\n"
                        } else {
                                "Children of " name(p) " and " sname ":\n"
                                children(f, k, m) {
                                        ".CH \"\" " roman(m) "\n"
                                        call middlevitals(k)
                                }
                        }
                }
        }
}

proc showsurnames(p)
{
        /*".NL\nPATERNAL LINE OF " upper(name(p)) "\n\n"*/
        ".NL\n"
        list(snames)
        table(stable)
        while (p) {
                if (not(lookup(stable, surname(p)))) {
                        enqueue(snames, save(surname(p)))
                        insert(stable, save(surname(p)), 1)
                }
                set(p, father(p))
        }
        set(c, "")
        forlist (snames, s, n) {
                c upper(s)
                set(c, ", ")
        }
        "\n"
}
