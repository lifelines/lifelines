/* 
 * @progname       menu.ll
 * @version        1.0
 * @author         ?
 * @category       
 * @output         GUI
 * @description    

   Menu driven shell for LifeLines report programs

menu.ll - Menu driven shell for LifeLines report programs

To use this shell, rename "proc main()" in a report to "proc <call_name>()",
then add the file/description/call names in three places here.

The menu loops until the first item is selected so pressing "q"
from LifeLines user interface does not result in quitting.

*/

include("eol.li")
include("longline.li")
include("stat9.li")

/* more file inclusions go here */

proc main() {
list(mnu)
enqueue(mnu, "         >> EXIT to LifeLines MAIN MENU << ")
enqueue(mnu, "eol2.ll     - End of Line Ancestors - Tom Wetmore, John Chandler")
enqueue(mnu, "longline.ll - Longest Lines         - John Chandler")
enqueue(mnu, "stat9.ll    - Statistics            - Jim Eggert")

/* more report descriptions go here */

set(xitem, 0)
while (ne(1, xitem)) {
set(xitem, menuchoose(mnu, "Choose the program to run"))

if     (eq(xitem, 2)) { call eol() }
elsif  (eq(xitem, 3)) { call longline() }
elsif  (eq(xitem, 4)) { call stat9() }

/* more procedure calls go here */

}
}
