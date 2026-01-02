/*
@progname tempnode.ll
@author Matt Emmerton
@description Test usage of temporary nodes
*/

/* test1a: test addnode and deletenode */
proc test1a() {

  /* n0 (INDI) is an existing node, expected to be non-temp */
  set (indi, firstindi())
  set (n0, inode(indi))

  /* n1/n2/n3 are all temp nodes, in a tree structure */

  set (n1, createnode ("TAG1", "NODE1"))
  set (n2, createnode ("TAG2", "NODE2"))
  set (n3, createnode ("TAG3", "NODE3"))

  addnode(n2, n1, 0)
  addnode(n3, n2, 0)

  "Initial State" nl()

  pvalue(n0) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()

  /* Once n1 is added to n0, n1/n2/n3 should become non-temp nodes */

  addnode(n1, n0, 0)

  "After addnode()" nl()

  pvalue(n0) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()

  /* Once n1 is removed from n0, n1/n2/n3 should become temp nodes again */

  detachnode(n1)

  "After detachnode()" nl()

  pvalue(n0) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()
}

/* test 1b: test addnode/deletenode with reuse */
proc test1b() {

  /* n0 (INDI) is an existing node, expected to be non-temp */
  set (indi, firstindi())
  set (n0, inode(indi))

  /* n9 (INDI) is an existing node, expected to be non-temp */
  set (indi, lastindi())
  set (n9, inode(indi))

  /* n1/n2/n3 are all temp nodes, in a tree structure */

  set (n1, createnode ("TAG1", "NODE1"))
  set (n2, createnode ("TAG2", "NODE2"))
  set (n3, createnode ("TAG3", "NODE3"))

  addnode(n2, n1, 0)
  addnode(n3, n2, 0)

  "Initial State" nl()

  pvalue(n0) nl()
  pvalue(n9) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()

  /* Once n1 is added to n0, n1/n2/n3 should become non-temp nodes */

  addnode(n1, n0, 0)

  "After addnode() to n1" nl()

  pvalue(n0) nl()
  pvalue(n9) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()

  /* Once n1 is detached from n0, n1/n2/n3 should become temp nodes again */

  detachnode(n1)

  "After detachnode() from n9" nl()

  pvalue(n0) nl()
  pvalue(n9) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()

  /* Once n1 is added to n9, n1/n2/n3 should become non-temp nodes again */

  addnode(n1, n9, 0)

  "After addnode() to n9" nl()

  pvalue(n0) nl()
  pvalue(n9) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()

  /* Once n1 is detached from n0, n1/n2/n3 should become temp nodes again */

  detachnode(n1)

  "After detachnode() from n0" nl()

  pvalue(n0) nl()
  pvalue(n9) nl()

  pvalue(n1) nl()
  pvalue(n2) nl()
  pvalue(n3) nl()
}

proc main() {

  "Starting Test" nl()

  call test1a()
  call test1b()

  "Ending Test" nl()
}
