/*
 * @progname       count_paternal_desc.ll
 * @version        2.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program counts paternal descendants of a person by generation.
Only unique individuals in each generation are counted.
A person counts in all the generations he/she is in,
but only counts once in the grand total.
Male paternal descendants are also counted separately.

count_paternal_desc - a LifeLines descendants counting program
         by Jim Eggert (eggertj@atc.ll.mit.edu)
         Version 1,  1 August 1994
         Version 2, 16 February 1995, use lengthset(), print(,)
*/

proc main ()
{
    getindimsg(person,"Enter person to count paternal descendants of")
    indiset(thisgen)
    indiset(allgen)
    indiset(allmalegen)
    addtoset(thisgen, person, 0)
    if (male(person)) { addtoset(allmalegen, person, 0) }
    print("Counting generation ")
    "Number of paternal descendants of " key(person) " " name(person)
    " by generation:\n"
    set(gen,neg(1))
    while(lengthset(thisgen)) {
        indiset(thismalegen)
        forindiset(thisgen,person,val,count) {
            if (male(person)) {
                addtoset(thismalegen,person,0)
            }
        }
	incr(gen)
	print(d(gen)," ")
	"Generation " d(gen) " has " d(lengthset(thisgen))
	" paternal descendant"
	if (gt(lengthset(thisgen),1)) { "s" }
	" of which " d(lengthset(thismalegen)) " are male.\n"
	set(thisgen,childset(thismalegen))
	set(allgen,union(allgen,thisgen))
	set(allmalegen,union(allmalegen,thismalegen))
    }
    "Total unique paternal descendants in generations 1-" d(gen)
    " is " d(lengthset(allgen))
    " of which " d(lengthset(allmalegen))
    " are male paternal descendants.\n"
}
