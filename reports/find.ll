/*
 * @progname       find.ll
 * @version        2.0
 * @author         Prinke
 * @category       
 * @output         GUI
 * @description    Display menu of persons with TAG having matching VALUE

   find.ll - Rafal Prinke, rafalp@plpuam11.amu.edu.pl, 7 OCT 1995

This utility finds all persons whose records contain a specified
TAG and VALUE and displays the resulting list as a menu.

The options allow to:

- find all occurances of a given TAG when no VALUE is given
- find all occurances of a given VALUE when no TAG is given
- find all occurances of a given VALUE under a given TAG when
       both are given (the CONT|CONC|TYPE tags are also searched)

The displayed VALUE is a 25 characters long substring of the field
value starting from the first occurence of the input value.

*/


proc main() {
list(mnu)
getstr(tg, "TAG (enter=ANY)")
set(tg, save(upper(tg)))
getstr(vl, "VALUE (enter=ANY)")
set(vl, save(upper(vl)))
  forindi (i, n) {
    set(r, inode(i))
    traverse (r, n, x) {
       set(xtag, save(upper(tag(n))))
       set(xval, save(upper(value(n))))
       if (eq(strlen(vl), 0)) { set(ofst, 1) }
       else { set(ofst, index(xval, vl, 1)) }
       if (or(or(and(eqstr(tg, xtag), or(index(xval, vl, 1),
          eq(strlen(vl), 0))), and(eq(strlen(tg), 0), index(xval, vl, 1))),
          and(index("CONTYPECONC", xtag, 1), index(xval, vl, 1)))) {
                set(z, save(substring(value(n), ofst, strlen(xval))))
                  if (gt(strlen(z), 25)) { set (z, save(substring(z, 1, 25))) }
                enqueue(mnu, save(concat(rjustify(key(i), 6), " - ",
                  rjustify(fullname(i, 1, 1, 18), 18),
                  " - ", tag(parent(n)), ":", d(x), "_", tag(n), ":", z)))
       }
    }
  }
if (eq(length(mnu), 0)) { print("No matches found") }
else {
menuchoose(mnu, "Just find the INDI Key you need, press Q and Browse to it") }
}
