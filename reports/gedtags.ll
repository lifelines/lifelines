/*
 * @progname       gedtags.ll
 * @version        2001-06-28
 * @author         Paul B.McBride (pbmcbride@rcn.com)
 * @category       
 * @output         Text
 * @description

    produces a unique list of all tags used in the database
    listed like the following:
    INDI
    INDI.BIRT
    INDI.BIRT.DATE
    INDI.BIRT.PLAC
    ...

    each line of the output will be unique.

    this can be useful in understanding the structure of the data in a GEDCOM
    file, or in checking for errors.

    sort the output file using an external sort program.

    Warning: for some versions of LifeLines probably prior to 3.0.3
    a save() should surround the values to be stored in tables and lists.

 * Paul B.McBride (pbmcbride@rcn.com) 28 June 2001
 */

global(tagnames)
global(taglevels)
global(content)

proc main ()
{
   list(tagnames)
   list(taglevels)
   table(content)

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

   /* insert sorting code here if desired */

   forlist(tagnames,n,p) { n "\n" }
}

proc out(item)
{
   traverse(root(item),y,level) {

     setel(taglevels,add(level,1),tag(y))

     set(i,0)
     set(s,"")
     while(le(i,level)) {
       if(gt(i,0)) {
         set(s,concat(s,"."))
       }
       set(s,concat(s, getel(taglevels,add(i,1))))
       incr(i)
     }
     if(eq(lookup(content, s),0)) {
       enqueue(tagnames,s)
       insert(content,s,1)
     }
   }
}
