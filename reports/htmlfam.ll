/*
 * @progname       htmlfam.ll
 * @version        3
 * @author         Tom Wetmore (ttw@shore.net)
 * @category       
 * @output         HTML
 * @description
 *
 * output family group summaries in HTML format
 */

/* third draft -- 12/27/95 -- Tom Wetmore -- ttw@shore.net */

global(pert)    /* person table */
global(showf)   /* families that have been shown */

proc main ()
{
        getindi(per0, "Who do you want to start with?")
        set(fam0, parents(per0))
        list(perq)
        list(famq)
        table(pert)
        list(lst)
        insert(pert, save(key(per0)), lst)
        table(showf)

        enqueue(perq, per0)
        while (per, dequeue(perq)) {
                if (fam, parents(per)) {
                        if (per, husband(fam)) {
                                call makelink(per, fam)
                                enqueue(perq, per)
                        }
                        if (per, wife(fam)) {
                                call makelink(per, fam)
                                enqueue(perq, per)
                        }
                }
        }
        call showhead()
        call showper(per0)
        enqueue(famq, fam0)
        while (fam, dequeue(famq)) {
                if (not(lookup(showf, key(fam)))) {
                        call showfam(fam)
                        insert(showf, save(key(fam)), 1)
                }
                set(husb, husband(fam))
                set(wife, wife(fam))
                if (fam, parents(husb)) { enqueue(famq, fam) }
                if (fam, parents(wife)) { enqueue(famq, fam) }
        }
        call showtail()
}

proc makelink (per, fam)
{
        if (lst, lookup(pert, key(per))) {
                call enqueueifnew(lst, key(fam))
        } else {
                list(lst)
                enqueue(lst, save(key(fam)))
                insert(pert, save(key(per)), lst)
        }
}

proc enqueueifnew (lst, key)
{
        forlist (lst, el, num) {
                if (eqstr(key, el)) { return() }
        }
        enqueue(lst, save(key))
}

proc showper (per)
{
        call showone(per)
        families(per, fam, sp, num) {
                call showone(sp)
                call showmarr(fam)
                call showchildren(fam)
        }
        "<HR>\n"
}

proc showfam (fam)
{
        "<A NAME=\"" key(fam) "\"></A>\n"
        call showone(husband(fam))
        call showone(wife(fam))
        call showmarr(fam)
        call showchildren(fam)
        "<HR>\n"
}

proc showone (per)
{
        if (not(per)) { return() }
        "<P><STRONG>"name(per, 0)"</STRONG>\n"
        if (evt, birth(per)) { "<BR>born "long(evt)"\n" }
        if (evt, death(per)) { "<BR>died "long(evt)"\n" }
        set(fam, parents(per))
        if (par, father(per)) {
                "<BR>father " call showlink(par, key(fam)) "\n"
        }
        if (par, mother(per)) {
                "<BR>mother " call showlink(par, key(fam)) "\n"
        }
}

proc showmarr (fam)
{
        if (evt, marriage(fam)) { "<BR>married "long(evt)"\n" }
}

proc showchildren (fam)
{
        if (eq(0, nchildren(fam))) { return() }
        "<P><STRONG>Children</STRONG>\n"
        children (fam, per, num) {
                "<BR>" d(num) " " call showchild(per) "\n"
        }
}

proc showlink (per, key) {
        set(lst, lookup(pert, key(per)))
        if (lst) { "<A HREF=\"#" key "\">" }
        name(per, 0)
        if (lst) { "</A>" }
        call showevents(per)
}

proc showchild (per) {
        if (lst, lookup(pert, key(per))) {
                call showlinks(per, lst)
        } else {
                name(per, 0)
                call showevents(per)
        }
}

proc showlinks (per, lst) /* LOOSEEND -- THIS ROUTINE NEEDS MORE */
{
        if (eq(0, length(lst))) {
                call showlink(per, "start")
        } else {
                call showlink(per, getel(lst, 1))
        }
}

proc showevents (per)
{
        set(evt, birth(per))
        if (and(evt, year(evt))) { ", b " year(evt) }
        set(evt, death(per))
        if (and(evt, year(evt))) { ", d " year(evt) }
}

proc showhead () {
        "<HTML><HEAD><TITLE>Genealogy Page</TITLE></HEAD>\n<BODY>\n"
        "<A NAME=\"start\"></A>\n"
}

proc showtail () {
        "</BODY></HTML>\n"
}
