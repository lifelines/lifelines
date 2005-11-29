/*
 * @progname       find.ll
 * @version        2.1
 * @author         Prinke, Perry Rapp
 * @category       
 * @output         GUI
 * @description    Display menu of persons with TAG having matching VALUE

This utility finds all persons whose records contain a specified
TAG and VALUE and displays the resulting list as a menu.

   find.ll - Rafal Prinke, rafalp@plpuam11.amu.edu.pl, 7 OCT 1995

The options allow to:

- find all occurrences of a given TAG when no VALUE is given
- find all occurrences of a given VALUE when no TAG is given
- find all occurrences of a given VALUE under a given TAG when
       both are given (the CONT|CONC|TYPE tags are also searched)

The displayed VALUE is a 25 characters long substring of the field
value starting from the first occurence of the input value.

The results are displayed in a menuchoice list.
The first choice is to print the remaining choices to a file.
*/
option(explicit)

proc main()
{
  list(mnu)

  getstr(tg, "TAG (enter=ANY)")
  set(tg, upper(tg))

  getstr(vl, "VALUE (enter=ANY)")
  set(vl, upper(vl))

  while (1)
  {
    getstr(rtype, "Records to search (I, F, S, E, X, or <enter> for any)")
    set(rtype, upper(rtype))
    if (or(eq(rtype, ""), index("IFSEX", rtype, 1)))
    {
       break()
    }
  }

  set(outputChoice, "Print to output file")
  enqueue(mnu, outputChoice)

  /* people */
  if (or(eq(rtype, ""), eq(rtype, "I")))
  {
    forindi (rec, n)
    {
      call search(rec, tg, vl, mnu)
    }
  }
  /* families */
  if (or(eq(rtype, ""), eq(rtype, "F")))
  {
    forfam (rec, n)
    {
      call search(rec, tg, vl, mnu)
    }
  }
  /* sources */
  if (or(eq(rtype, ""), eq(rtype, "S")))
  {
    forsour (rec, n)
    {
      call search(rec, tg, vl, mnu)
    }
  }
  /* events */
  if (or(eq(rtype, ""), eq(rtype, "E")))
  {
    foreven (rec, n)
    {
      call search(rec, tg, vl, mnu)
    }
  }
  /* others */
  if (or(eq(rtype, ""), eq(rtype, "X")))
  {
    forothr (rec, n)
    {
      call search(rec, tg, vl, mnu)
    }
  }

  if (eq(length(mnu), 1))
  {
    print("No matches found")
  }
  else
  {
    set(chc, menuchoose(mnu, "Use record keys as below to browse to desired record"))
    if (eq(chc, 1))
    {
      "Search for tag <" tg "> and value <" vl ">"
      if (eq(rtype, "")) { " in all records" }
      else { " in " rtype " records" }
      " yielded " d(sub(length(mnu), 1)) " hits:\n"
      forlist(mnu,s,c) {
        if (ne(s, outputChoice))
        {
          s nl()
        }
      }
    }
  }
}

/*
 Search rec (an INDI or FAM or ...)
  for occurrences of tag tg with value vl
  (Either may be empty as wildcards)
*/
proc search(rec, tg, vl, mnu)
{
  set(rnod, root(rec))
  set(nodtyp, tag(rnod))
  traverse (rnod, n, x)
  {
    set(xtag, upper(tag(n)))
    set(xval, upper(value(n)))
    if (eq(strlen(vl), 0))
    {
      set(ofst, 1) 
    }
    else
    {
      set(ofst, index(xval, vl, 1)) 
    }
    if (or(or(and(eqstr(tg, xtag), or(index(xval, vl, 1),
        eq(strlen(vl), 0))), and(eq(strlen(tg), 0), index(xval, vl, 1))),
        and(index("CONTYPECONC", xtag, 1), index(xval, vl, 1)))) 
    {
      set(z, substring(value(n), ofst, strlen(xval)))
      if (gt(strlen(z), 25))
      {
        set(z, substring(z, 1, 25))
      }
      set(result, concat(rjustify(key(rec), 6), " - "))
      if (eq(nodtyp, "INDI"))
      {
        set(result, concat(result,
           rjustify(fullname(rec, 1, 1, 18), 18)))
      }
      set(result, concat(result, 
         " - ", tag(parent(n)), ":", d(x), "_", tag(n), ":", z))
      enqueue(mnu, result)
    }
  } /* traverse */
}
