/*
 * @progname    pedtex.ll
 * @version     1.0
 * @author      Eric Majani
 * @category
 * @output      TeX
 * @description
 * 
 *    generates TeX files for pedigree charts
 */

global(depth)
global(level)

proc main ()
{
      getindi(indi)
      set(prompt,"Enter number of generations desired")
      getintmsg(depth,prompt)
      "\\input setup" nl()
      "\\tree " nl()
      set(level,1)
      call pedout(indi)
      "\\endtree " nl()
      "\\end " nl()

}

proc printindi(indi)
{
      "{\\bf " name(indi) "}" nl()
      if (e, birth(indi)) { "    b. " short(e) nl() }
      if(male(indi))
       {
         spouses(indi,sp,fam,num)
            {
               if(e,marriage(fam)) { "   m. " short(e) nl() }
            }
       }
      if (e, death(indi)) { "    d. " short(e) nl() }
}

proc pedout(indi)
{
      call printindi(indi)
      set(level,add(level,1))
      if(le(level,depth))
      {
            if (par,father(indi))
            {
                  set(fath,father(indi))
                  "\\subtree " nl()
                  call pedout(fath)
                  "\\endsubtree " nl()
            }
            if (par,mother(indi))
            {
                  set(moth,mother(indi))
                  "\\subtree " nl()
                  call pedout(moth)
                  "\\endsubtree " nl()
            }
      }
      set(level,sub(level,1))
}
