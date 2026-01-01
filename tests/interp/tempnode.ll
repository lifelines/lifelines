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

  print("Initial State")
  print(nl())

  print(pvalue(n0))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is added to n0, n1/n2/n3 should become non-temp nodes */

  addnode(n1, n0, 0)

  print("After addnode()")
  print(nl())

  print(pvalue(n0))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is removed from n0, n1/n2/n3 should become temp nodes again */

  detachnode(n1)

  print("After detachnode()")
  print(nl())

  print(pvalue(n0))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())
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

  print("Initial State")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is added to n0, n1/n2/n3 should become non-temp nodes */

  addnode(n1, n0, 0)

  print("After addnode() to n1")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is added to n9, n1/n2/n3 should remain non-temp nodes */

  addnode(n1, n9, 0)

  print("After addnode() to n9")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is detached from n9, n1/n2/n3 should remain non-temp nodes */

  detachnode(n9)

  print("After detachnode() from n9")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is detached from n0, n1/n2/n3 should become temp nodes again */

  print("After detachnode() from n0")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is attached to n0, n1/n2/n3 should become non-temp nodes again */

  addnode(n1, n0, 0)

  print("After addnode() to n0 again")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())

  /* Once n1 is deteched from n0, n1/n2/n3 should become temp nodes again */

  print("After detachnode() from n0 again")
  print(nl())

  print(pvalue(n0))
  print(nl())
  print(pvalue(n9))
  print(nl())

  print(pvalue(n1))
  print(nl())
  print(pvalue(n2))
  print(nl())
  print(pvalue(n3))
  print(nl())
}

proc main() {
  call test1a()
  call test1b()
}
