/* 
 * @progname       find
 * @version        1.0
 * @author         Prinke
 * @category       
 * @output         GUI
 * @description    

The program asks the user for the TAG and its VALUE, and the displays
a menu of all the matches.
The VALUE matches as many characters as the user inputs, so "occu/s"
will display a menu of all persons whose occupation name starts with an "s".

find.ll   - Rafal Prinke, rafalp@plpuam11.amu.edu.pl

Case is not important (unless there are 8-bit characters).

Please, note that two lines are over 80 chars and must be joined together
before running the program */



proc main() {
list(mnu)
getstr(tg, "Tag: ")
set(tg, save(upper(tg)))
getstr(vl, "Value: ")
set(vl, save(upper(vl)))
   forindi (i, n) {
        set(r, inode(i))
        traverse (r, n, x) {
           if (and(eqstr(tg, upper(tag(n))), eqstr(vl, upper(substring(value(n), 1, strlen(vl)))))) {
           enqueue(mnu, save(concat(key(i), " -- ", name(i), " -- ", d(x), ": ", tag(n), " = ", value(n), "\n")))
           }
        }
   }
menuchoose(mnu, "Just find the INDI Key you need, press Q and Browse to it")
}
