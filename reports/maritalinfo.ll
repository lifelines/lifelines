/*
* @progname       maritalinfo.ll
* @version        1.0 (2002-11-13)
* @author         Perry Rapp
* @category       sample
* @output         screen
* @description
*
*                 Simple example of looping through marital (& divorce) info
*/

/* get marital & divorce lists, and then display them */
proc main() {

  getfam(family)
  list(marriages)
  getmarriages(family, marriages)
  if (not(empty(marriages))) {
    print("Marital events:", nl())
    forlist (marriages, node, offset) {
      if (eq(tag(node), "ENGA")) {
        call event_out("Engagement: ", node)
      }
      if (eq(tag(node), "MARR")) {
        call event_out("Marriage: ", node)
      }
    }
  }
  list(divorces)
  getdivorces(family, divorces)
  if (not(empty(divorces))) {
    print("Divorce events:", nl())
    forlist (divorces, node, offset) {
      if (eq(tag(node), "ANUL")) {
        call event_out("Annulment: ",  node)
      }
      if (eq(tag(node), "DIV")) {
        call event_out("Divorce: ", node)
      }
      if (eq(tag(node), "DIVF")) {
        call event_out("DivorceFiling: ", node)
      }
    }
  }

}

/* send out event info with header, to screen right now */
proc event_out(hdr, event) {
  print(event_string(hdr, event), nl())
}

/* make a display string out of an event and a header */
func event_string(hdr, event) {
  set(outstr, concat(hdr, short(event)))
  return(outstr)
}

/* get list of all marital events in family */
func getmarriages(family, evlist) {
  fornodes (root(family), node) {
    if (eq(tag(node), "MARR")) { push(evlist, node) }
    if (eq(tag(node), "ENGA")) { push(evlist, node) }
  }
}

/* get list of all divorce-style events in family */
func getdivorces(family, evlist) {
  fornodes (root(family), node) {
    if (eq(tag(node), "ANUL")) { push(evlist, node) }
    if (eq(tag(node), "DIV")) { push(evlist, node) }
    if (eq(tag(node), "DIVF")) { push(evlist, node) }
  }
}
