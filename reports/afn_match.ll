/*
 * @progname    afn_match.ll
 * @version     1.0 of 1995-08-25
 * @author      Scott McGee
 * @category
 * @output      Text
 * @description
 *
 * Find individuals with matching Ancestral File numbers, report matches.
 *

This program is designed to search a database and find individuals with
the same AFN's. The output is a report of such matching individuals.

Last updated 25 Aug, 1995 by Scott McGee (smcgee@microware.com)
*/

global(first)

proc main (){
  table(t)

  set(first, 1)
  print("Processing database ")
  set(cnt, 0)
  forindi(indi, n){
    if(afn, get_afn(indi)){
      if(match, lookup(t, afn)){
        call found_match(indi, save(afn), match)
      }else{
        insert(t, save(afn), indi)
      }
    }
    incr(cnt)
    if(eq(cnt, 100)){
      set(cnt, 0)
      print(".")
    }
  }
}

func get_afn(indi){
  if(indi){
    fornodes(inode(indi), subnode){
      if(eqstr(tag(subnode), "AFN")){
        return(value(subnode))
      }
    }
  }
  return(0)
}

proc found_match(i2, afn, i1){
  if(first){
    set(first, 0)
    "Ancestral File Number match report\n\n"
    "produced by afn_match.ll version 1.0\n"
    "by Scott McGee (smcgee@microware.com)\n\n"
    "Database: "
    database()
    "\nDate:     "
    long(gettoday())
    "\n\n"
    "AFN      Key1      Key2      Name1\n"
    "_________________________________________________________________________\n"
  }
  afn
  col(10)
  key(i1)
  col(20)
  key(i2)
  col(30)
  name(i1, 0)
  "\n"
}
