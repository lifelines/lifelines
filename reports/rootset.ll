/*
 * @progname       rootset.ll
 * @version        0.5
 * @author         Robert Simms
 * @category       
 * @output         Text
 * @description

   Given a list of surnames, finds the set of all people with those
   surnames, then reports on the heads of each line within that set.
*/
proc main() {
        indiset(rootset)
        indiset(tempset)

        getstr(surname, "Specify a surname")
        while(nestr(surname, "")) {
                genindiset(concat("*/", surname), tempset)
                set(rootset, tempset)
                getstr(surname, "Specify another surname [return if done]")
        }

        set( tempset, childset(rootset))
        set( rootset, difference(rootset, tempset))
        call lprintset(rootset)
        call printset(rootset)
}

proc printset(x) {
        forindiset(x, x_ind, y, x_n) {
                key(x_ind) " - " name(x_ind) nl()
        }
}

proc lprintset(x) {
        forindiset(x, x_ind, y, x_n) {
                print(key(x_ind), " - ", name(x_ind), nl())
        }
}
