/*
 * @progname    desc-tex.ll
 * @version     1995-01-01
 * @author      Eric Majani (eric@elroy.jpl.nasa.gov)
 * @category
 * @output      TeX
 * @description
 *
 * Descendent tree in TeX.
 * This has been modified to add the suggested poster support by
 * Pete Glassenbury and Jim Eggert.  This is not an official copy from
 * Eric nor have I tried it.
 *
 * Slight changes by D. Roegel (roegel@loria.fr), 1/1/1995
 */

global(depth)
global(level)

proc main ()
{
      getindi(indi)
      set(prompt,"Enter number of generations desired")
      getintmsg(depth,prompt)
      "\\input setup\n"
      "\\input poster\n"
      /* next line corrected by D. Roegel, 1/1/1995 */
      "\\Poster[hcenter=true,vcenter=true,paperwidth=210mm,paperheight=297mm]\n"
      "\\vbox{%   Because \\Poster processes in horizontal mode,\n"
      "%   but your tree macros are in vertical mode.\n"
      "\\tree\n"
      set(level,1)
      call descout(indi)
      "\\endtree\n"
      "}% End of \\vbox\n"
      "\\endPoster\n"
      "\\end\n"

}

proc printindi(indi)
{
      "{\\bf " name(indi) "}" nl()
      if (e, birth(indi)) { "    b. " short(e) nl() }
      spouses(indi,sp,fam,num) { if(e,marriage(fam)) { "   m. " short(e) nl() }
 }
      if (e, death(indi)) { "    d. " short(e) nl() }
}

proc printcouple(indi,fam,num)
{
      if(eq(num,1))
      {
         "{\\bf " name(indi) "}" nl()
         if (e, birth(indi)) { " b. " short(e) nl() }
      }
      if (e,marriage(fam)) { " m. " short(e) nl() }
      if(eq(num,nspouses(indi)))
      {
         if (e, death(indi)) { " d. " short(e) nl() }
      }
}

proc printfam(indi,fam,sp)
{
      "\\spouse{ " name(sp) "}" nl()
}

proc descout(indi)
{
      if(eq(0,nspouses(indi)))
      {
         call printindi(indi)
      }
      spouses(indi,sp,fam,num)
         {
            call printcouple(indi,fam,num)
            call printfam(indi,fam,sp)
            set(level,add(level,1))
            if(le(level,depth))
            {
               children(fam,child,no)
                  {
                     "\\subtree " nl()
                     call descout(child)
                     "\\endsubtree " nl()
                  }
            }
            set(level,sub(level,1))
            if(ne(num,nspouses(indi)))
            {
               "\\endsubtree " nl()
               "\\subtree " nl()
            }
         }
}

