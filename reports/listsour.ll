/*
 * @progname       listsour.ll
 * @version        2
 * @author         Hannu V&auml;is&auml;nen
 * @category       
 * @output         Text
 * @description

   List source records.

   Written by Hannu Väisänen, 1 May 1997.
*/

global(sour)

proc main()
{
  table(sour)

  forindi (person, m) {
    print ("i")
    call print (person)
  }
  forfam (family, m) {
    print ("f")
    call print (family)
  }
}

proc print (p)
{
  traverse (root(p), node, i) {
    if (eqstr(tag(node), "SOUR")) {
      if (reference(value(node))) {
        if (not(lookup(sour, value(node)))) {
          insert (sour, save(value(node)), 1)
          set (n, dereference(value(node)))
          value(node) "\n"
          fornodes (n, m) {
            tag(m) " " value(m) "\n"
          }
          "\n"
        }
      }
    }
  }
}
