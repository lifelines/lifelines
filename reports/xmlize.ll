/*
 * @progname       xmlize2.ll
 * @version        2.2
 * @author         Rafal T. Prinke
 * @category       
 * @output         XML
 * @description

         This report converts all LifeLines records
         to XML tagged file, with <LLGEDCOM> as
         the root element.
         It is now more consistent with Mike Kay's GedML.
         References are all empty tags with IDREF attribute.
         The NAME element contains one S element - for
         surname (ie. it is a reserved tag and cannot be
         used elsewhere).

         I am leaving specific tags for vital events
         rather than converting them to attributes
         of generic EVEN tags as Mike does in GedML.

         The CONT tag is replaced with an empty <P/> tag
         to indicate a new paragraph and the CONC tag
         is replaced with a space to assure continuity
         of a paragraph.

   xmlize2.ll - v. 1.1 Rafal T. Prinke, 8 March 2001
                v. 2.0 - - , 28 April 2001
                v. 2.1 - - , 29 April 2001
                v. 2.2 - - , 30 June  2001 - changed REF to IDREF
*/


global(previous)
global(numery)
global(tagi)

proc main ()
{
  list(numery)
  list(tagi)
  set(previous,-1)

  "<?xml version=" qt() "1.0" qt()

/* remove the next line for Unicode encoding or
   edit for a different 8-bit encoding using one of these:
      ISO-8859-1       - ISO Latin-1 (Western Europe)
      ISO-8859-2       - ISO Latin-2 (Central-Eastern Europe)
      windows-1252     - Windows Lat-1 (Western Europe)
      windows-1250     - Windows Lat-2 (Central-Eastern Europe)
*/

  " encoding=" qt() "ISO-8859-2" qt()
  " standalone=" qt() "yes" qt() "?>"

  "\n"
  "<LLGEDCOM>"

  forindi(pers,x) {
      call out(pers)
  }
  forfam(fm,x) {
      call out(fm)
  }
  foreven(evn, n) {
      call out(evn)
  }
  forsour(src, n) {
      call out(src)
  }
  forothr(oth, n) {
      call out(oth)
  }

  "\n</LLGEDCOM>"
}


proc out(item)

{
  "\n"
  traverse(root(item),y,level) {

    if(eqstr(tag(y),"CONT")) { "<P/>" value(y) }

    elsif(eqstr(tag(y),"CONC")) { " " value(y) }

    else {

       while(and(le(level,previous),not(empty(tagi)))) {
           "</" pop(tagi) ">"
           set(nic,pop(numery))
           set(previous,sub(previous,1))
       }
       "<" tag(y)

       if(index(value(y),"@",1)) {
           set(wart,value(y))
           " IDREF=" qt() substring(wart,2,sub(strlen(wart),1)) qt()
"/>"
       }

       else {
           if(eq(level,0)) {
               " ID=" qt() substring(xref(y),2,sub(strlen(xref(y)),1))
qt()
           }
          ">"

/* insert tag S for surname with space before and after if necessary */

    if(eqstr(tag(y),"NAME")) {
      set(bef,substring(value(y),1,sub(index(value(y),"/",1),1)))
      bef
      if(nestr(substring(bef,strlen(bef),strlen(bef))," ")) { " " }
      "<S>"

substring(value(y),add(index(value(y),"/",1),1),sub(index(value(y),"/",2),1))
      "</S>"

set(aft,substring(value(y),add(index(value(y),"/",2),1),strlen(value(y))))
      if(and(nestr(substring(aft,1,1)," ") ,ne(strlen(aft),0) )) { " " }
      aft
    }

    else { value(y) }

          push(numery,level)
          push(tagi,tag(y))
          set(previous,level)
    }
  }
}

   while(not(empty(tagi))) {
      "</" pop(tagi) ">"
      set(nic,pop(numery))
   }
}
