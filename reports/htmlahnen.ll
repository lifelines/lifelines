/*
 * @progname       htmlahnen.ll
 * @version        2
 * @author         Tom Wetmore
 * @category       
 * @output         HTML
 * @description
 *
 * Generate an ahnentafel chart in HTML format
 */
/* Version 2, 12/31/95 */

proc main ()
{
        getindi(per, "Whose Ahnentafel do you want?")
        if (not(per)) { return() }

        set(title,  concat("Ahnentafel of ", name(per, 0)))
        call htmlhead(title)
        call htmlheading(3, title)
        print("Ahnentafel of ", name(per), "\n")

        list(ilist)     /* list of persons waiting to be output */
        list(alist)     /* ahnen numbers of those persons */
        list(glist)     /* generations of those persons */
        table(ktab)     /* table of all persons who have been output */
        table(ctab)     /* table of child links */

        enqueue(ilist, per)     /* initialize all structures */
        enqueue(alist, 1)
        enqueue(glist, 1)
        set(cgen, 0)
        call addchild(ctab, 0, per)

        while(per, dequeue(ilist)) {
                set(ahnen, dequeue(alist))
                set (tgen, dequeue(glist))
                if (ne(cgen, tgen)) {
                        "<HR><P>" call htmlstrong("Generation ")
                        call htmlstrong(d(tgen)) "\n"
                        set(cgen, tgen)
                }
                "<P>"
                set(old, lookup(ktab, key(per)))
                if (old) {
                        call htmlstrong(d(ahnen)) " Same as "
                        call htmlstrong(d(old))
                        call htmllink(concat("#", key(per)), " link")
                } else {
                        call htmlname(key(per)) print(".")
                        insert(ktab, save(key(per)), ahnen)
                        call htmlstrong(d(ahnen)) " "
                        call htmlstrong(name(per, 0)) "\n"
                        set(lst, lookup(ctab, key(per)))
                        set(comma, 0)
                        forlist (lst, key, n) {
                                if (comma) { ", " }
                                else { set(comma, 1) }
                                call htmllink(concat("#", key), "chld")
                        }
                        if (par,father(per)) {
                                enqueue(ilist, par)
                                call addchild(ctab, per, par)
                                enqueue(alist, mul(2, ahnen))
                                enqueue(glist, add(cgen, 1))
                                if (comma) { ", " }
                                else { set(comma, 1) }
                                call htmllink(concat("#", key(par)), "fath")
                        }
                        if (par,mother(per)) {
                                enqueue(ilist, par)
                                call addchild(ctab, per, par)
                                enqueue(alist, add(1, mul(2, ahnen)))
                                enqueue(glist, add(cgen, 1))
                                if (comma) { ", " }
                                else { set(comma, 1) }
                                call htmllink(concat("#", key(par)), "moth")
                        }
                        if (e, birth(per)) { "<BR>    b. " long(e) "\n" }
                        if (e, death(per)) { "<BR>    d. " long(e) "\n" }
                }
                "\n"
        }
        call htmltail()
}

proc addchild (ctab, per, par)
{
        set(lst, lookup (ctab, key(par)))
        if (not(lst)) {
                list(lst)
                if (per) {
                        setel(lst, 1, save(key(per)))
                }
                insert(ctab, save(key(par)), lst)
        } else {
                setel(lst, add(1, length(lst)), save(key(per)))
        }
}

proc htmlhead (title)
{
        "<HTML><HEAD><TITLE>" title "</TITLE></HEAD>\n<BODY>\n"
}

proc htmltail ()
{
        "\n</BODY></HTML>\n"
}

proc htmlstrong (str)
{
        "<STRONG>" str "</STRONG>"
}


proc htmllink (href, link)
{
        "<A HREF=\"" href "\">" link "</A>"
}

proc htmlname (name)
{
        "<A NAME=\"" name "\"></A>"
}

proc htmlheading (lev, head)
{
        "<H" d(lev) ">" head "</H" d(lev) ">\n"
}
