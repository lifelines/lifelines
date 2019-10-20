/* tests for if/else */

/* test 1a: single if/else, without varb */
proc test1a(d) {
  set(r, "default")
  if (gt(d,3)) { set(r, " greater than 3") }
  else         { set(r, " not greater than 3") }
  print("test1a ", d(d), r, "\n")
}

/* test 1b: single if/else, with varb */
proc test1b(d) {
  set(r, "default")
  if (gt(d,3)) { set(r, " greater than 3") }
  else         { set(r, " not greater than 3") }
  print("test1b ", d(d), r, "\n")
}

/* test2a: multiple if/elsif/else, without varb */
proc test2a(d) {
  if    (gt(d, 3)) { set(r, " is greater than 3") }
  elsif (gt(d, 2)) { set(r, " is greater than 2") }
  elsif (gt(d, 1)) { set(r, " is greater than 1") }
  elsif (gt(d, 0)) { set(r, " is greater than 0") }
  elsif (eq(d, 0)) { set(r, " is equal to 0") }
  else                { set(r, " is negative") }
  print("test2a ", d(d), r, "\n")
}

/* test2a: multiple if/elsif/else, with varb */
proc test2b(d) {
  if    (m, gt(d, 3)) { set(r, " is greater than 3") }
  elsif (m, gt(d, 2)) { set(r, " is greater than 2") }
  elsif (m, gt(d, 1)) { set(r, " is greater than 1") }
  elsif (m, gt(d, 0)) { set(r, " is greater than 0") }
  elsif (m, eq(d, 0)) { set(r, " is equal to 0") }
  else                { set(r, " is negative") }
  print("test2b ", d(d), r, "\n")
}

proc main() {
  /* test1a */
  call test1a(5)
  call test1a(2)

  /* test1b */
  call test1b(5)
  call test1b(2)

  /* test2a */
  call test2a(4)
  call test2a(3)
  call test2a(2)
  call test2a(1)
  call test2a(0)
  call test2a(-1)

  /* test2b */
  call test2a(4)
  call test2a(3)
  call test2a(2)
  call test2a(1)
  call test2a(0)
  call test2a(-1)
}
