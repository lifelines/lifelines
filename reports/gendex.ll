/*
 * @progname       gendex.ll
 * @version        1.2
 * @author         Scott McGee (smcgee@microware.com)
 * @category       
 * @output         HTML
 * @description

This report program converts a LifeLines database into html gendex document.
You will need to change the contents of proc html_address() and to
set the value of HREF appropriately to your server.

@(#)gendex.ll	1.2 10/14/95
*/

global(INDEX)
global(HREF)

proc main()
{
    indiset(INDEX)
    set(HREF, "/INDEX=")
    print("processing database\n")
    set(count, 0)
    set(name_count, 0)
    forindi(me,num)
    {
      if(eq(count, 100)){
        set(count, 0)
        print(".")
      }else{
        incr(count)
        incr(name_count)
      }
      addtoset(INDEX,me,1)
    }
    print("\nwriting file\n")
    call create_gendex_file()
    print("\n", d(name_count), " individuals\n")
}


proc create_gendex_file() {
  set(fn, save("GENDEX.txt"))
  newfile(fn, 0)
  forindiset(INDEX, me, v, n)
  {
    set(path, concat(HREF, save(key(me)), "/?LookupInternal"))
    path
    "|"
    surname(me)
    "|"
    givens(me) " /"
    surname(me) "/"
    "|"
    if (evt, birth(me)) {
      date(evt)
    }    
    "|"
    if (evt, birth(me)) {
      place(evt)
    }    
    "|"
    if (evt, death(me)) {
      date(evt)
    }    
    "|"
    if (evt, death(me)) {
      place(evt)
    }    
    "|\n"
  }
}
