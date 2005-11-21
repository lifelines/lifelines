/*
 * @progname       refn.ll
 * @version        1.0
 * @author         Larry Hamilton
 * @category     
 * @output         Text
 * @description    Report of all User Reference Numbers (REFN).

 Prints out the value of all the lines in your database with the REFN tag,
 along with enough information so you can find the line easily. The purpose
 of this report is so you can find all the REFNs, and double-check them for
 duplicates.

 *                Modified from Olsen, Eggert - places.ll
 *
 *                by Larry Hamilton (lmh@hamiltongensociety.org)
 *                Version 1.0, November 10, 2005
 *
 * The REFNs are printed out in the order that they appear in the database.
 *
 *                  To sort the output:
 * on Unix\Linux: sort -f originalfilename -o sortedfilename
 * on Windows:    sort originalfilename /O sortedfilename
 *
 */

proc main()
{
  list(tag_stack)

  print("Printing all REFNs.\n")
  print("Be patient. This may take a while.\n\n")
  print("If there are no REFNs in the database,\nthere will not be a prompt for an output file name.\n")

  forindi (person, id) {

    traverse (inode(person), node, level) {

      setel(tag_stack, add(level, 1), tag(node))

      if (eq(strcmp(tag(node), "REFN"), 0)) {
        tag(node) " " value(node) " | " key(person) " " name(person)
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

      if (eq(strcmp(tag(node), "REFN"), 0)) {
       tag(node) " " value(node) " | " key(fam) " ("
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
