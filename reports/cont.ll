/*
 * @progname    cont.ll
 * @version     1.0
 * @author      V&auml;is&auml;nen
 * @category
 * @output      Text
 * @description

This program iterates over all persons and families in a database
and reports all records that have erroneous CONT lines.

It finds errors like

2 TAG  blah     2 CONT blah     2 CONT blah
2 CONT blah       3 CONT blah   2 TAG  blah

If the output is

  These individuals may have problems with CONT lines
  These families may have problems with CONT lines

then the program found no errors.


Written by Hannu Väisänen 22 September 1999.
*/

proc main()
{
  "These individuals may have problems with CONT lines\n"

  forindi (person, m) {
    print ("i")
    call check (person, 0)
  }

  "These families may have problems with CONT lines\n"
  forfam (family, m) {
    print ("f")
    call check (family, 1)
  }
}

proc check (person, isfam)
{
  set (prev_level, 0)
  set (prev_tag, "xxxx")

  traverse (root(person), node, n) {
    if (eqstr(tag(node), "CONT")) {
      if (eqstr(prev_tag, "CONT")) {
        if (ne(prev_level, n)) {
          if (eq(isfam,1)) {
            "Husband " key(husband(person)) " " name(husband(person)) "\n"
            "Wife    " key(wife(person))    " " name(wife(person)) "\n\n"
          }
          else {
            key(person) " " name(person) "\n"
          }
        }
      }
      else {
        if (ne(add(prev_level,1), n)) {
          if (eq(isfam,1)) {
            "Husband " key(husband(person)) " " name(husband(person)) "\n"
            "Wife    " key(wife(person))    " " name(wife(person)) "\n\n"
          }
          else {
            name(person) " " key(person) "\n"
          }
        }
      }
    }
    set (prev_level, n)
    set (prev_tag, tag(node))
  }
}
