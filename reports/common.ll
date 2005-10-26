/*
 * @progname    common.ll
 * @version     0 of 1996-06-11
 * @author      H. V&auml;is&auml;nen
 * @category
 * @output      Text
 * @description
                 Show common ancestors of a person.

   Pedigree collapse means that someone is descended from some persons
   in two or more ways. If person's father and mother are related,
   this program lists the common ancestors and the people between them
   and the person.

   The program probably does not work if person is descended from
   common ancestors in more than two ways or if there is different
   number of generations in those two ways.

   by H. V<a-umlaut>is<a-umlaut>nen
   Version 0, 11 June 1996
*/

proc main()
{
  getindi (person)
  "Common ancestors of " name (person) "\n\n\n"


  /* Father and his ancestors. */
  indiset (father_set)
  if (f, father(person)) {
    addtoset (father_set, f, 0)
    set (father_set, union (father_set, ancestorset (father_set)))
  }

  /* Mother and her ancestors. */
  indiset (mother_set)
  if (m, mother(person)) {
    addtoset (mother_set, m, 0)
    set (mother_set, union (mother_set, ancestorset (mother_set)))
  }

  /* Their intersection. */
  indiset (intersection_set)
  set (intersection_set, intersect (father_set, mother_set))
  valuesort (intersection_set)

  /* Is minimum of v always zero? I'm not sure... */
  set (min, 10000)
  forindiset (intersection_set, indi, v, n) {
    if (lt(v, min)) {set (min, v)}
  }

  /* First common ancestors. */
  indiset (common_ancestor_set)
  forindiset (intersection_set, indi, v, n) {
    if (eq(min, v)) {
      addtoset (common_ancestor_set, indi, 0)
    }
  }

  if (eq(lengthset(common_ancestor_set), 0)) {
    print ("Person's father and mother are not related.")
    "Person's father and mother are not related.\n"
    return()
  }

  set (max_name_length, max_length (common_ancestor_set))

  /* Print first common ancestors. */
  forindiset (common_ancestor_set, indi, v, n) {
    col (20)
    call print_indi (indi, v, add(max_name_length, 20)) "\n"
  }
  "\n"


  /* Descendants of first common ancestors. */
  indiset (descendant_set)
  set (descendant_set, descendantset(common_ancestor_set))


  /* Links from the father's side. */
  indiset (set1)
  set (set1, intersect (descendant_set, father_set))
  valuesort (set1)

  /* Links from the mother's side. */
  indiset (set2)
  set (set2, intersect (descendant_set, mother_set))
  valuesort (set2)


  set (max_name_length, max_length(set1))
  set (length2, max_length(set2))

  if (gt(length2, max_name_length)) {set (max_name_length, length2)}



  /* Print father's line on the left, mother's line on the right. */
  table (mom)
  forindiset (set2, indi, v, n) {
    insert (mom, d(v), indi)
  }
  forindiset (set1, indi, v, n) {
    call print_indi (indi, v, add(max_name_length,1)) col(40)
    call print_indi (lookup(mom, d(v)), v, add(max_name_length,40)) "\n"
  }

  "\n"
  col (20)
  call print_indi (person, add(v,1), add(max_name_length, 20)) "\n"
}


proc print_indi (indi, v, length)
{
  name (indi) col(length) " ("
  if (p, birth(indi)) {year(p)} else {"    "}
  " - "
  if (p, death(indi)) {year(p)} else {"    "}
  ") (" d(v) ")"
}


/* Maximum length of a name of a person in person_set.
 */
func max_length (person_set)
{
  set (max_name_length, 0)
  forindiset (person_set, indi, v, n) {
    if (lt(max_name_length, strlen(name(indi)))) {
      set (max_name_length, strlen(name(indi)))
    }
  }
  return (max_name_length)
}
/*
-----------------------------------------------------------------------

This is an example of the output. I have deleted the surnames because
they contain 8 bit letters.

Common ancestors of Juho XXXXXXXX


                   Maria AAAAAAAAAAA (1606 - 1661) (0)
                   Heikki XXXXXXXX   (1603 - 1670) (0)

Heikki XXXXXXXX (1628 - 1705) (1)      Lauri XXXXXXXX  (1637 - 1701) (1)
Heikki XXXXXXXX (1666 - 1731) (2)      Eeva XXXXXXXX   (1659 - 1724) (2)
Aatami XXXXXXXX (1687 - 1733) (3)      Anna BBBBBBBB   (1691 - 1746) (3)
Juho XXXXXXXX   (1721 - 1775) (4)      Ulla CCCCCCCC   (1724 - 1789) (4)

                   Juho XXXXXXXX  (1761 - 1848) (5)
*/
