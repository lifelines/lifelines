/*
 * @progname       extract_gedcom.ll
 * @version        1.2 of 1995-08-27
 * @author         Scott McGee (smcgee@microware.com)
 * @category       
 * @output         GEDCOM
 * @description

This program allows the user to select a group of individuals from a database
and generate a GEDCOM file for them. It allows selection of multiple people 
by following family links, and then allows addition of all ancestors of the
selected set or of the first individual, and then all descendants of the 
selected set or of the orignal individual. It also allows addition of all 
persons with a specified number of relations to any individual in any of the 
groups added above.

For each person asked about, you will be given some information on them to
aid in deciding if they are the one you want or not. This is similar to a
person display when browsing with LifeLines. 

This program will also output all source records referred to in any person
record in the gedcom output.

Thanks to Tom Wetmore for many small routines that have been addapted for
use in this program as well as LifeLines itself.

Scott McGee
*/

include("extract_set.li")
include("tools.li")
include("outsources.li")

global(first)          /* first person shouldn't be asked about */
global(first_indi)     /* starting person */

proc main () {
  getindi(indi)
  if (indi) {
    set(first_indi, indi)
    set(out, extract_set(indi))
    call extract_gedcom(indi, out)
  }
  else {
    print("No one identified -- terminating\n")
  }
}

proc extract_gedcom(indi, out) {
  print("Generating GEDCOM file for ", d(lengthset(out)), " individuals.\n")
  gengedcom(out)
  call outsources(out)
}
