/*
 * @progname       zombies.ll
 * @version        1995-06
 * @author         H. V<a-umlaut>is<a-umlaut>nen
 * @category       
 * @output         Text
 * @description

  zombies - a list of people who should be dead but who are not.
  This program lists all persons who have been born over 100 years
  and who have a birth event but not a dead event.

  Birth year is output first, so that you can sort the output file by

    sort file


  Written by H. V<a-umlaut>is<a-umlaut>nen, June 1995.
*/

proc main ()
{
  forindi(indi, num) {
    set(birt,0)
    set(dead,0)
    if (b, birth(indi)) {set(birt,1)}
    if (d, death(indi)) {set(dead,1)}
    if (and(eq(birt,1), eq(dead,0))) {
      if (le(atoi(year(b)),1894)) {
        year(b) " " fullname(indi,0,1,50) " " date(b) "\n"
      }
    }
  }
}
