/*
 * @progname    desc_ged.ll
 * @version     1
 * @author      Nicklaus
 * @category
 * @output      Text
 * @description
	        Generate gedcom of descendents.
		For specified set of individuals for specified # of generations
		down from the top individuals. (e.g. name all your great-grandparents
		for all of your close cousins in one gedcom file)

	Author:  Dennis Nicklaus  nicklaus@fnal.gov  June 1997
*/
/* MODIFY this to put in your name and address! */
proc print_header()
{
  "0 HEAD\n"
  "1 SOUR LIFELINES\n"
  "2 VERS 3.0.2\n"
  "2 NAME LifeLines for UNIX\n"
  "1 DATE " stddate(gettoday()) nl()
  "0 @SM1@ SUBM\n"
  "1 NAME your name here\n"
  "1 ADDR your street here\n"
  "2 CONT your town\n"
  "2 CONT your email\n"
}

proc main ()
{
    getindimsg(person,"Enter person to output GEDCOM descendants of")
    indiset(thisgen)
    indiset(allgen)
    while (person){
	addtoset(thisgen, person, 0)
	addtoset(allgen, person, 0)

	set(person,0)
        getindimsg(person,"Enter next person to output GEDCOM descendants of")
     }

    set(allgen, union(allgen,spouseset(allgen)))
    getintmsg (ngen,
               "Enter number of generations")
    set(gen,1)
    while(and(lengthset(thisgen),le(gen,ngen))) {
        set (thisgensize,lengthset(thisgen))
        print ("generation ",d(gen)," ",d(thisgensize))
        if (gt(thisgensize,1)) {
            print(" people\n")
         } else {
            print(" person\n")
         }
        set(gen,add(gen,1))

        set(thisgen,childset(thisgen))
        set(allgen,union(allgen,thisgen))
        set(allgen,union(allgen,spouseset(thisgen)))
    }
    call print_header()
    gengedcom(allgen)

        call sour_init()
        call sour_addset(allgen)
        call sour_ged()

    "0 TRLR\n"

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

