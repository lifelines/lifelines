/*
 * @progname       places.ll
 * @version        5.0
 * @author         Olsen, Eggert
 * @category       
 * @output         Text
 * @description    
 *
 * Prints out the value of all the lines in your database with the PLAC tag,
 * along with enough information so you can find the line easily. The purpose
 * of this report is so you can find all the places that seem wrong
 * (misspelled, ambiguous, incomplete [left out the county name], etc), and
 * double-check them or correct them. 

places

Version 1, 25 Nov 1992 by David Olsen (dko@cs.wisc.edu)
Version 2,  3 Dec 1992 by Jim Eggert (eggertj@atc.ll.mit.edu)
    (Changed numbers to real key values.)
Version 3,  5 Feb 1993 by David Olsen (dko@cs.wisc.edu)
    (Prints place names in reverse order.  Runs faster.)
Version 4, 13 Feb 1993 by Jim Eggert (eggertj@atc.ll.mit.edu)
    (Prints place names in regular or reverse order.)
Version 5,  1 Sep 1993 by Jim Eggert (eggertj@atc.ll.mit.edu)
    (Fixes a bug involving families with no parents)

Report program for LifeLines v. 2.3.3.

Prints out the value of all the lines in your database with the PLAC
tag, along with enough information so you can find the line easily.
The places are printed either exactly as they appear in the database
(e.g. Madison, Dane, Wisconsin) or in reverse order (e.g. Wisconsin,
Dane, Madison).  The purpose of this report is so you can find all the
places that seem wrong (mispelled, ambiguous, incomplete, etc.), and
double-check them or correct them.

The places are printed out in the order that they appear in the
database, so the report is not very useful in its native form.  To
make it more useful, run the output file through the program 'sort',
making it much easier to spot incorrect names.  For example, if you
have 100 entries within Middlesex County, Massachusetts, but in one of
them you mispelled Middlesex as Middlesx, it will be very easy to spot
this in the sorted output.

If the place name is part of an individual record, it is followed by
the key and name of the individual and by the hierarchy of tags
between the INDI tag and the PLAC tag (usually just a single tag, such
as BIRT or DEAT).  If the place name is part of a family record, it is
followed by the family key, the key and name of the husband (or wife
if there is no husband, or first child if there is no parent), the
relationship in the family of that person, and the hierarchy of tags
between the FAM tag and the PLAC tag (usually just the single tag
MARR).

Some sample output (in reverse order) that has already been sorted:

California, Los Angeles, Long Beach  | I130 Newel Knight YOUNG | DEAT
California, Los Angeles, Los Angeles  | I6811 Gunella CHRISTIANSEN | DEAT
California, Los Angeles, Newhall  | I836 Cena Elizabeth HAWKINS | DEAT
California, Los Angeles, San Fernando  | I836 Cena Elizabeth HAWKINS | BURI
California, Napa, Napa  | I1439 Cora Anna BEAL | DEAT
California, Riverside, Riverside  | F328 (I1370 Benjamin BERRY, husb) | MARR
California, San Bernadino,   | I6843 Emily LUDVIGSEN | BURI
California, San Bernadino, San Bernadino  | I1364 Francis LYTLE | DEAT
California, San Bernadino, San Bernadino  | I1365 Sophronia Jane MILLETT | DEAT
California, San Bernadino, San Bernadino  | I1367 Nancy Ellen LYTLE | BIRT
California, San Bernadino, San Bernadino  | I1369 Hulda Lorene LYTLE | BIRT
California, San Bernadino, San Bernadino  | I694 Mary Ann HENRY | BIRT
California, Shasta, Redding  | I1378 Eliza Lemira MILLETT | DEAT
California, Whittier, Rose Hills  | I2318 Zetta Fern MORTENSEN | BURI
Canada, Nova Scotia, Cape Breton  | F184 (I749 Ezra KING, husb) | MARR
Canada, Nova Scotia, Cape Breton  | I749 Ezra KING | DEAT


*/

proc main()
{
  list(tag_stack)
  list(place_names)

  set(reverse,0)
  getstrmsg(yesno,"Reverse place name components? (y/n)")
  if (strlen(yesno)) {
    if (not(strcmp(upper(trim(yesno,1)),"Y"))) { set(reverse,1) }
  }
  print("Printing all places.\n")
  print("Be patient.  This may take a while.\n")

  forindi (person, id) {

    traverse (inode(person), node, level) {

      setel(tag_stack, add(level, 1), tag(node))

      if (eq(strcmp(tag(node), "PLAC"), 0)) {
        extractplaces(node, place_names, num_places)
        if (reverse) {
          pop(place_names)
          while (p, pop(place_names)) { ", " p }
        }
        else {
          dequeue(place_names)
          while (p, dequeue(place_names)) { ", " p }
        }
        "  | " key(person) " " name(person) " |"
        forlist (tag_stack, tag, tag_number) {
          if (and(gt(tag_number, 1), le(tag_number, level))) { " " tag }
        }
        "\n"
      }
    }
  }

  forfam (fam, fnum) {

    traverse (fnode(fam), node, level) {
      setel(tag_stack, add(level, 1), tag(node))

      if (eq(strcmp(tag(node), "PLAC"), 0)) {
        extractplaces(node, place_names, num_places)
        if (reverse) {
          pop(place_names)
          while (p, pop(place_names)) { ", " p }
        }
        else {
          dequeue(place_names)
          while (p, dequeue(place_names)) { ", " p }
        }
        "  | " key(fam)
        " ("
        if (person,husband(fam)) { set(relation,", husb") }
        elsif (person,wife(fam)) { set(relation,", wife") }
        else {
          children(fam,child,cnum) {
            if (eq(cnum,1)) {
              set(person,child)
              set(relation,", chil")
            }
          }
        }
        if (person) {
          key(person) " " name(person) relation
        }
        ") |"
        forlist (tag_stack, tag, tag_number) {
          if (and(gt(tag_number, 1), le(tag_number, level))) { " " tag }
        }
        "\n"
      }
    }
  }
}
