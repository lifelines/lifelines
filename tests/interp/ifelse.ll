proc elsifchain1(d) {
  set(r, " is not positive")
  if    (m, gt(d, 3)) { set(r, " is greater than 3") }
  elsif (m, gt(d, 2)) { set(r, " is greater than 2") }
  elsif (m, gt(d, 1)) { set(r, " is greater than 1") }
  elsif (m, gt(d, 0)) { set(r, " is greater than 0") }
  print(d(d), r, "\n")
}

proc elsifchain2(d) {
  set(r, " is not positive")
  if    (gt(d, 3)) { set(r, " is greater than 3") }
  elsif (gt(d, 2)) { set(r, " is greater than 2") }
  elsif (gt(d, 1)) { set(r, " is greater than 1") }
  elsif (gt(d, 0)) { set(r, " is greater than 0") }
  print(d(d), r, "\n")
}

proc main() {
  /* with varb */
  call elsifchain1(4)
  call elsifchain1(3)
  call elsifchain1(2)
  call elsifchain1(1)
  call elsifchain1(0)
  call elsifchain1(-1)

  /* without varb */
  call elsifchain2(4)
  call elsifchain2(3)
  call elsifchain2(2)
  call elsifchain2(1)
  call elsifchain2(0)
  call elsifchain2(-1)

}
