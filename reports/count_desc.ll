/*
 * @progname       count_desc.ll
 * @version        2.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program counts descendants of a person by generation.
Only unique individuals in each generation are counted.
A person counts in all the generations he/she is in,
but only counts once in the grand total.

count_desc - a LifeLines descendants counting program
         by Jim Eggert (eggertj@atc.ll.mit.edu)
         Version 1,  19 November 1992
         Version 2,  16 February 1995, use lengthset(), print(,)
*/

proc main ()
{
    getindimsg(person,"Enter person to count descendants of")
    indiset(thisgen)
    indiset(allgen)
    addtoset(thisgen, person, 0)
    print("Counting generation ")
    "Number of descendants of " key(person) " " name(person)
    " by generation:\n"
    set(thisgensize,1)
    set(gen,neg(1))
    while(thisgensize) {
        set(thisgensize,0)
        if (thisgensize,lengthset(thisgen)) {
            set(gen,add(gen,1))
            print(d(gen)," ")
            "Generation " d(gen) " has " d(thisgensize) " individual"
            if (gt(thisgensize,1)) { "s" }
            ".\n"
            set(thisgen,childset(thisgen))
            set(allgen,union(allgen,thisgen))
        }
    }
    "Total unique descendants in generations 1-" d(gen)
    " is " d(lengthset(allgen)) ".\n"
}
