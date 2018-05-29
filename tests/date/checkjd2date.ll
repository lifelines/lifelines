/* From Jim Eggert */
proc main() {
  set(startjd,date2jd("04 OCT 1580"))
  set(delta,0)
  while (lt(delta,mul(366,4))) {
    set(jd, add(startjd, delta))
    d(jd) " " date(jd2date(jd)) "\n"
    incr(delta, 10)
  }
}
