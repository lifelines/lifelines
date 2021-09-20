/*
@progname test_eqv_pvalue.ll
@author Matt Emmerton
@description Test eqv_pvalue on various data types
*/

proc main ()
{
  "Starting Test" nl()

  set(TRUE,1)
  set(FALSE,0)

  /* sanity cases */
  call testeq(eq(NULL,NULL),      FALSE, "Both args null")
  call testeq(eq(valstring,NULL), FALSE, "First arg null")
  call testeq(eq(NULL,valstring), FALSE, "Second arg null")

  /* value cases - program types */
  call testeq(eq("Pi_is_3.14", "Pi_is_3.14"), TRUE, "String equality")
  call testeq(ne("Pi_is_3.14", "Pi_is_6.28"), TRUE, "String inequality")
  call testeq(eq(3.14, 3.14), TRUE, "Float equality")
  call testeq(ne(3.14, 6.28), TRUE, "Float inequality")
  call testeq(eq(314, 314),   TRUE, "Integer equality")
  call testeq(ne(314, 628),   TRUE, "Integer inequality")
  call testeq(eq(TRUE, TRUE), TRUE, "Boolean equality")
  call testeq(ne(TRUE, FALSE),TRUE, "Boolean inequality")

  /* value cases - GEDCOM types */
  getindi(indi1) /* I1040 */
  getindi(indi2) /* I1020 */
  call testeq(eq(indi1,indi1), TRUE, "INDI equality")
  call testeq(ne(indi1,indi2), TRUE, "INDI inequality")

  getfam(fam1) /* F1000 */
  getfam(fam2) /* F1001 */
  call testeq(eq(fam1,fam1), TRUE, "FAM equality")
  call testeq(ne(fam1,fam2), TRUE, "FAM inequality")

  forsour (sour, n) {
    if (eq(n,1)) { set(sour1,sour) } /* S10 */
    if (eq(n,2)) { set(sour2,sour) } /* S20 */
    if (eq(n,3)) { break() }
  }
 
  call testeq(eq(sour1,sour1), TRUE, "SOUR equality")
  call testeq(ne(sour1,sour2), TRUE, "SOUR inequality")

  foreven (even, n) {
    if (eq(n,1)) { set(even1,even) } /* E30 */
    if (eq(n,2)) { set(even2,even) } /* E40 */
    if (eq(n,3)) { break() }
  }
 
  call testeq(eq(even1,even1), TRUE, "EVEN equality")
  call testeq(ne(even1,even2), TRUE, "EVEN inequality")

  foreven (othr, n) {
    if (eq(n,1)) { set(othr1,othr) } /* X50 */
    if (eq(n,2)) { set(othr2,othr) } /* X60 */
    if (eq(n,3)) { break() }
  }
 
  call testeq(eq(othr1,othr1), TRUE, "OTHR equality")
  call testeq(ne(othr1,othr2), TRUE, "OTHR inequality")

  /* pointer cases - GEDCOM types */
  list(list1)
  push(list1, indi1)
  push(list1, indi2)
  list(list2)
  push(list1, indi1)
  push(list1, indi2)
  call testeq(eq(list1,list1), TRUE, "LIST equality (pointer)")
  call testeq(ne(list1,list2), TRUE, "LIST inequality (pointer)")

/* TODO: TABLE */

  indiset(indiset1)
  addtoset(indiset1, indi1, 0)
  indiset(indiset2)
  addtoset(indiset2, indi2, 0)
  call testeq(eq(indiset1,indiset1), TRUE, "INDISET equality (pointer)")
  call testeq(ne(indiset1,indiset2), TRUE, "INDISET inequality (pointer)")

  list(list3)
  setel(list3, 2, "two")
  list(list4)
  setel(list4, 1, "one")
  call testeq(eq(list3,list3), TRUE, "LIST equality (pointer)")
  call testeq(ne(list3,list4), TRUE, "LIST inequality (pointer)")

  set(birth1, birth(indi1))
  set(birth2, birth(indi2))
  call testeq(eq(birth1,birth1), TRUE, "NODE equality (pointer)")
  call testeq(ne(birth1,birth2), TRUE, "NODE inequality (pointer)")
}

proc testeq (testcond, result, message)
{
  if (eq(testcond,result)) { "PASS: " }
  else { "FAIL: " }
  message
  nl()
}
