/*
 * @progname       sources.ll
 * @version        1.0
 * @author         
 * @category       
 * @output         Text
 * @description    
 *
 * Print the sources associated with an individual.
 */
global(refn)    /* root node of references record */
global(reftab)  /* table of reference keys reported on */
global(ref1)

proc main ()
{
    getindi(refn, "Enter name of record that hold the references")
    if (eq(refn, 0)) {
        print("The references could not be found; program not run\n")
    } else {
        set(refn, inode(refn))
        call foundrefs()
    }
}

proc foundrefs ()
{
    table(refs)
    set(ref1, 0)
    getindi(indi, "Enter a person to show sources for.")
    while (indi) {
        call showperson(indi)
        "\n"
        getindi(indi, "Enter another person to show sources for.")
    }
    print("Program over!\n")
}

proc showperson (indi)
{
    call showvitals(indi)
    call showsources(indi)
}

proc showvitals (i)
{
    name(i) ".\n"
    set(e,birth(i))
    if(and(e,long(e))) { "Born " long(e) ".\n" }
    set(e,death(i))
    if(and(e,long(e))) { "Died " long(e) ".\n" }
}

proc showsources (i)    /* finds all SOUR lines in a record */
{
    table(reftab)
    set(ref1, 0)
    traverse (inode(i), s, n) {
        if (eq(0, strcmp("SOUR", tag(s)))) {
            call showsource(value(s))
        }
    }
}

proc showsource (v)     /* process each SOUR line in a record */
{
    set(ref, 0)
    fornodes (refn, s) {  /* look at each REFN line in references */
        if (eq(0, strcmp(v, value(s)))) { /* found one with matching code! */
            set(ref, s)  /* so set ref to this REFN node */
        }
    }
    if (ref) {  /* non-null if matching code were found */
        if (not(lookup(reftab, v))) {   /* and we hadn't seen it yet */
            if (not(ref1)) {    /* Print "References:" before first one */
                "References:\n"
                set(ref1, 1)
            }
            "\t" value(child(ref)) "\n"  /* This could be much better! */
            insert(reftab, v, 1)  /* So we won't show it again! */
        }
    }
}
