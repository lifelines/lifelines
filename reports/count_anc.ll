/*
 * @progname       count_anc.ll
 * @version        2.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program counts ancestors of a person by generation.
Only unique individuals in each generation are counted.
A person counts in all the generations he/she is in,
but only counts once in the grand total.

count_anc - a LifeLines ancestors counting program
         by Jim Eggert (eggertj@atc.ll.mit.edu)
         Version 1,  19 November 1992
         Version 2,  16 February 1995, use lengthset(), print(,)
*/

proc main ()
{
    getindimsg(person,"Enter person to count ancestors of")
    indiset(thisgen)
    indiset(allgen)
    addtoset(thisgen, person, 0)
    print("Counting generation ")
    "Number of ancestors of " key(person) " " name(person)
    " by generation:\n"
    set(thisgensize,1)
    set(gen,1)
    while(thisgensize) {
        set(thisgensize,0)
        if (thisgensize,lengthset(thisgen)) {
            set(gen,sub(gen,1))
            print(d(gen)," ")
            "Generation " d(gen) " has " d(thisgensize) " individual"
            if (gt(thisgensize,1)) { "s" }
            ".\n"
            set(thisgen,parentset(thisgen))
            set(allgen,union(allgen,thisgen))
        }
    }
    "Total unique ancestors in generations " d(gen) " to -1 is "
    d(lengthset(allgen)) ".\n"
}
