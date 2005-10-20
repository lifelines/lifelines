/*
 * @progname    anc2_ged.ll
 * @version     1.0
 * @author      Dennis Nicklaus
 * @category
 * @output      GedCom
 * @description
 *              make a gedcom file of the ancestors of a set of individuals
 *
 */
proc main ()
{
        indiset(a)
        monthformat(4)
        indiset(b)
        getindi(i)

    while (i){
        addtoset(a, i, 0)
        set(i,0)
        getindimsg(i,"Enter next person to output GEDCOM ancestors of")
     }
        set(b,ancestorset(a))
        set(b,union(b,a))

        call print_header()
        gengedcom(b)
        call sour_init()
        call sour_addset(b)
        call sour_ged()

    "0 TRLR\n"

}

proc print_header()
{
  "0 HEAD\n"
  "1 SOUR Lifelines\n"
  "1 DATE " stddate(gettoday()) nl()
  "0 @SM1@ SUBM\n"
  "1 NAME " getproperty("user.fullname") "\n"
  "1 ADDR " getproperty("user.address") "\n"
  "2 CONT " getproperty("user.email") "\n"
}
global(sour_list)
global(sour_table)

proc sour_init()
{
        table(sour_table)
        list(sour_list)
}
/* sour_addind() adds the sources referenced for this individual */

proc sour_addind(i)
{
         traverse(root(i), m, l) {
                if (nestr("SOUR", tag(m))) { continue() }
                set(v, value(m))
                if (eqstr("", v)) { continue() }
                if(reference(v)) {
                          if (ne(0, lookup(sour_table, v))) { continue() }
                          set(v, save(v))
                          insert(sour_table, v, 1)
                          enqueue(sour_list, v)
                }
         }
}

proc sour_addset(s)
{
        forindiset (s, i, a, n) {
                call sour_addind(i)
                families(i, f, sp, m) {
                  call sour_addind(f)
                }
        }
}

/* sour_ged() outputs the current source list in GEDCOM format */

proc sour_ged()
{
        table(other_table)
        list(other_list)

        forlist(sour_list, k, n) {
                set(r, dereference(k))
                traverse(r, s, l) {
                        d(l)
                        if (xref(s)) { " " xref(s) }
                        " " tag(s)
                        if (v, value(s)) {
                          " " v
                          if(reference(v)) {
                            if (ne(0, lookup(other_table, v))) { continue() }
                            if (ne(0, lookup(sour_table, v))) { continue() }
                            set(v, save(v))
                            insert(other_table, v, 1)
                            enqueue(other_list, v)
                          }
                        }
                        "\n"
                }
        }
        forlist(other_list, k, n) {
                set(r, dereference(k))
                traverse(r, s, l) {
                        d(l)
                        if (xref(s)) { " " xref(s) }
                        " " tag(s)
                        if (v, value(s)) { " " v }
                        "\n"
                }
        }
}
