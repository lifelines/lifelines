/*
 * @progname       select.ll
 * @version        3
 * @author         Wetmore, Groleau, McGee
 * @category       
 * @output         Custom
 * @description    

    Customizable report stub to do the following:
 o  Select a person with all ancestors and all descendents.
 o  Add to selection all other persons within a user-specified number of
       links from any person in the first selection.
 o  Turn the selected set of persons into a list
 o  Call a report subprogram to process the list.

    WRITTEN BY TOM WETMORE,       21 Jul 1995
    minor mods by Wes Groleau,    25 Aug 1995
    Scott McGee fixed Wes's bug,  26 Aug 1995   :-)
*/

        list(o)         /* output list           */

   /* have user provide start person and link distance */

        getindi(i, "Please identify start person.")
        if (not(i)) { return() }
        getint(n, "Please enter link distance.")

   /* create set with all ancestors and descendents */

        indiset(s) addtoset(s, i, 1)
        indiset(a) set(a, ancestorset(s))    /* could be made optional */
        indiset(d) set(d, descendentset(s))  /* could be made optional */
        set(s, union(s, union(a, d)))

   /* create set of additional, linked-to persons */

        indiset(t) set(t, spouseset(s))
        set(n, sub(n, 1))
        while (gt(n, 0)) {
                set(a, parentset(t))
                set(d, childset(t))
                set(b, siblingset(t))
                set(c, spouseset(t))
                set(t, union(t, union(a, union(d, union(b, c)))))
                set(n, sub(n, 1))
        }

   /* create final set of all selected persons and generate the report */

        set(s, union(s, t))

        if(s){
                forindiset(s, j, n, m) {
                        enqueue(o, j)
                }
        }

        call do_list(o)    /* your routine here */
}



proc do_list (o) {    /* your routine here */    }
