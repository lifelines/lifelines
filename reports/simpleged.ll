/*
 * @progname       simpleged.ll
 * @version        1.0
 * @author         Wetmore
 * @category       
 * @output         GEDCOM
 * @description    

This program generates a simple GEDCOM file from a database.  It can
be modified to convert your own LifeLines database formats to other
GEDCOM formats.

simpleged

Written by Tom Wetmore, July 1993.
*/

proc main ()
{
        "0 HEAD \n"
        "1 SOUR LIFELINES\n"
        forindi(indi, num) {
                print("i")
                call outindi(indi)
        }
        forfam(fam, num) {
                print("f")
                call outfam(fam)
        }
        "0 TRLR \n"
}

proc outindi (indi)
{
        set(root, inode(indi))
        set(noname, 1)
        set(nosex, 1)
        set(nobirt, 1)
        set(nobapt, 1)
        set(nodeat, 1)
        set(noburi, 1)
        "0 " xref(root) " " tag(root) nl()
        set(node, child(root))
        while (node) {
                if (and(noname, not(strcmp("NAME", tag(node))))) {
                        "1 NAME " value(node) nl()
                        set(noname, 0)
                } elsif (and(nosex, not(strcmp("SEX", tag(node))))) {
                        "1 SEX " value(node) nl()
                        set(nosex, 0)
                } elsif (and(nobirt, not(strcmp("BIRT", tag(node))))) {
                        call outevent(node)
                        set(nobirt, 0)
                } elsif (and(nobapt, not(strcmp("CHR", tag(node))))) {
                        call outevent(node)
                        set(nobapt, 0)
                } elsif (and(nodeat, not(strcmp("DEAT", tag(node))))) {
                        call outevent(node)
                        set(nodeat, 0)
                } elsif (and(noburi, not(strcmp("BURI", tag(node))))) {
                        call outevent(node)
                        set(noburi, 0)
                } elsif (not(strcmp("FAMC", tag(node)))) {
                        "1 FAMC " value(node) nl()
                } elsif (not(strcmp("FAMS", tag(node)))) {
                        "1 FAMS " value(node) nl()
                }
                set(node, sibling(node))
        }
}

proc outfam (fam)
{
        set(nomarr, 1)
        set(root, fnode(fam))
        "0 " xref(root) " " tag(root) nl()
        set(node, child(root))
        while (node) {
                if (not(strcmp("HUSB", tag(node)))) {
                        "1 HUSB " value(node) nl()
                } elsif (not(strcmp("WIFE", tag(node)))) {
                        "1 WIFE " value(node) nl()
                } elsif (not(strcmp("CHIL", tag(node)))) {
                        "1 CHIL " value(node) nl()
                } elsif (and(nomarr, not(strcmp("MARR", tag(node))))) {
                        call outevent(node)
                        set(nomarr, 0)
                }
                set(node, sibling(node))
        }
}

proc outevent (evt)
{
        set(nodate, 1)
        set(noplac, 1)
        set(nosour, 1)
        "1 " tag(evt) "\n"
        set(evt, child(evt))
        while (evt) {
                if (and(nodate, not(strcmp("DATE", tag(evt))))) {
                        "2 DATE " value(evt) nl()
                        set(nodate, 0)
                } elsif (and(noplac, not(strcmp("PLAC", tag(evt))))) {
                        "2 PLAC " value(evt) nl()
                        set(noplac, 0)
                } elsif (and(nosour, not(strcmp("SOUR", tag(evt))))) {
                        "2 SOUR " value(evt) nl()
                        set(nosour, 0)
                }
                set(evt, sibling(evt))
        }
}
