/*
 * @progname    desc-tex2.ll
 * @version     2 of 1995-01-16
 * @author      Majani and Roegel
 * @category
 * @output      TeX
 * @description
 *
 * DESC-TEX2 report for Lifelines genealogical system
 * prints a descendent chart in TeX format, with credits
 *
 * Version 1 by Eric Majani (eric@elroy.jpl.nasa.gov)  end 1992
 * ------------------------
 *
 *     This has been modified to add the suggested poster support by
 *     Pete Glassenbury and Jim Eggert.  
 *
 * Version 2 by Denis Roegel (roegel@loria.fr) 31 december 1994
 * -------------------------                    -- 16 january 1995 
 *
 *     Various modifications including: better multilingual support;
 *                                      More information on the tree;
 *                                      nodes are framed
 *                                          except those corresponding
 *                                          to 2nd, 3rd family, etc.
 *                                          (that way, each descendant is
 *                                           framed only once);
 *                                      When two or more branches rejoin,
 *                                        only the first traversed is complete;
 *                                      Genealogical symbols added for
 *                                          birth, wedding and death;
 *                                      Support for clipping;
 *                                      Support for "hand-merging" of trees
 *                                            (see file ex1.tex);
 *                                      Six kinds of subtree nodes
 *                                          (see file drtree.tex);
 *                                      Fonts are easily customizable
 *                                          (see in drsetup.tex);
 *                                      Currently, sans serif fonts are used
 *                                        since this seems best for xeroxing
 *                                        and it improves readability;
 *                                      Title and credits.
 *
 *       Thanks to Tom Wetmore, John Chandler and Michael P. Gerlek 
 *       for useful comments, hints, and bits of functions.
 *
 * Other comments welcome, but please, for question regarding TeX,
 * ask your local guru. Please.
 *

  THINGS TO DO: find out why there are messages like this one:

     Overfull \vbox (0.79999pt too high) detected at line 298
 
      (I think this is something due to bad framing, since 
       0.8pt is twice the thickness of the frame)


  KNOWN PROBLEMS:

    1) Since the whole tree has to be stored in one TeX box,
       and that this box is limited in the amount of information
       it may contain, you are likely to be unable to put more than
       200 or 300 descendants on a tree; the limit is not fixed
       and depends on the amount of information per person.
       If you only display the name of a descendant, you can go farther
       than if you display his whole life!

    2) An other problem occurs before even the first problem,
       but does not lead to a TeX error message (while the first
       problem does); the tree seems to be strangely mixed;
       I do not yet know the reason, but I suspect it to be in
       the tree macros. The problem was already present with the original
       tree macros.


 *       This is the first program I (DR) have modified.              
 */

global(depth)
global(maxdepth) /* the max depth encountered so far */
global(level)
global(done)  /* will record the families already done */
global(refno)             /* reference number for indi */

proc main ()
{
      table(done)
      getindi(indi)
      set(prompt,"Enter number of generations desired")
      getintmsg(depth,prompt)
      set(prompt,"Do you want a clipping ? (y/n)")
      getstrmsg(answer,prompt)
      if (or(eqstr(answer,"y"),eqstr(answer,"Y")))
        {set(clipping,1)}
      else
        {set(clipping,0)}
      set(prompt,"Do you use US paper ? (y/n)")
      getstrmsg(answer,prompt)
      if (or(eqstr(answer,"y"),eqstr(answer,"Y")))
        {set(uspaper,1)}
      else
        {set(uspaper,0)}
      set(prompt,"Do you want credits ? (y/n)")
      getstrmsg(answer,prompt)
      if (or(eqstr(answer,"y"),eqstr(answer,"Y")))
        {set(credits,1)}
      else
        {set(credits,0)}
      "%\\mag1000 % this is default value of \\mag\n"
      if (clipping) {"\\input pstricks\n"}
      "\\input poster\n"
      "\\input drsetup\n"
      "\\Poster[hcenter=true,%\n"
       "        vcenter=true,%\n"
      if (clipping) {"        clip=pstricks,%\n"}
       "        cropwidth=.4pt" 
      call paperdims(uspaper) 
               "]\n"
      "\\vbox{\\setbox0=\\vbox{%"
      "    Because \\Poster processes in horizontal mode,\n"
      "%   but the tree macros are in vertical mode.\n"
      if (le(nfamilies(indi),1))
         {"\\tree{normal}\n"}
      else
         {
         "\\tree{unframed;norules}\n"
         "\\subtree{framed;rules:right}\n"
         }
      set(maxdepth,1)
      set(level,1)
      call descout(indi)
      if (le(2,nfamilies(indi))) {"\\endsubtree\n"}
      "\\endtree\n"
      "}% End of \\vbox\n"      
       "\\title{" call show_name(indi)     "}"
              "{" call print_title_birth(indi) "}"
              "{" call print_title_death(indi) "}"
              "{" decr(maxdepth) d(maxdepth)     "}"
              "{\\wd0}\n"
      "\\vskip2cm\n"
      "\\copy0\n"
      "\\vskip1cm\n"
      "\\noindent\\rlap{\\hbox to\\wd0{\\hss"
      if (credits) {"\\credits"}
                                             "}}}\n"
      "\\endPoster\n"
      "\\end\n"

}


proc printindi(indi)
{
      "{\\descfont " call show_name(indi) "}" nl()
      call print_occupation(indi,5)
      call print_birth(indi,5)
      call print_baptism(indi,5)
      call print_death(indi,5)
}

proc printcouple(indi,fam,num)
{
      if(eq(num,1))
      {
         "{\\descfont " call show_name(indi) "}" nl()
         "\\sepline " nl()
      call print_occupation(indi,5)
      call print_birth(indi,5)
      call print_baptism(indi,5)
      call print_death(indi,5)
      }
      else 
       {"{$\\langle$ \\descfont " call show_name(indi) " $\\rangle$}" nl()}
      call print_marriage(fam,5)
}

proc printfam(indi,fam,sp)
{
      "\\hglue5mm\\spouse{" call show_name(sp) "}" nl()
      call print_occupation(sp,10)
      call print_birth(sp,10)
      call print_baptism(sp,10)
      call print_death(sp,10)
}

proc descout(indi)
{
   if (eq(0,nfamilies(indi))){call printindi(indi)}
   else {
        families(indi,fam,sp,num) 
         {
               call printcouple(indi,fam,num)
               if (sp) {call printfam(indi,fam,sp)} 
               if (lookup(done, key(fam))) {call already_seen()}
               else{
                    if (le(maxdepth,level)) {incr(maxdepth)}
                    set(level,add(level,1))
                    if(le(level,depth))
                    {
                       children(fam,child,no)
                          {
                             "\\subtree{normal}% " nl()
                             call descout(child)
                             "\\endsubtree " nl()
                          }
                    }
                    set(level,sub(level,1))
                   }
               set(refno,add(refno,1))  /* increment global counter */
               insert(done, save(key(fam)), refno)
               if(ne(num,nfamilies(indi)))
               {
                  "\\endsubtree " nl()
                  if (eq(level,1))
                     {"\\subtree{unframed;rules:right}% "}
                  else {"\\subtree{unframed;rules:left,right}% "} nl()
               }
            
            }     
        }
}


proc print_birth(indi,mmshift)
{
     if (e, birth(indi)) { "\\hglue" d(mmshift) "mm " call btag() 
                           call show_date_place(e) nl() }
}

proc print_baptism(indi,mmshift)
{
     if (e, baptism(indi)) { "\\hglue" d(mmshift) "mm " call bapttag() 
                           call show_date_place(e) nl() }
}



proc print_title_birth(indi)
{
     if (e, birth(indi)) { call show_date(e) }
}



proc print_marriage(fam,mmshift)
{
     if (e,marriage(fam)) { "\\hglue" d(mmshift) "mm "  call mtag() 
                              call show_date_place(e) nl() }
}
 
proc print_death(indi,mmshift)
{
     if (e, death(indi)) { "\\hglue" d(mmshift) "mm " call dtag() 
                           call show_date_place(e) nl() }
}

proc print_title_death(indi)
{
     if (e, death(indi)) { call show_date(e) }
}


proc print_occupation(indi,mmshift)
{
      fornodes (inode(indi), n) {
             if (eq(strcmp(tag(n), "OCCU"), 0)) {
                "\\hglue" d(mmshift) "mm\\occupation{"
                 value(n) "}" nl()
             }
      }
}

proc show_name (i)
{
        list(parts)
        extractnames(inode(i), parts, n, s)
        set(head, dequeue(parts))
        call print_name_element(head) 
        forlist (parts, el, n) {" " call print_name_element(el)}
}

proc show_date_place(e)
{
       if (date(e)) { "{\\datefont " call show_date(e) "} " }
       "{\\placefont " call at() "}"
       "{\\placefont "
       if (place(e)) { place(e) }
       else {" ? "}
       "}"  /* end of \placefont */
}


proc show_date(e)
{
       list(parts)
       extracttokens(date(e), parts, n, " ")
       set(head, dequeue(parts))
       call print_date_element(head) 
       forlist (parts, el, m) {" " call print_date_element(el)}
}

/* This is for my personal conventions: I use /Unknown/
   for unkown names */
proc print_name_element(el)
{
     if (eqstr("UNKNOWN", el)) { "\\unknown{}" }
     elsif (eqstr("Unknown", el)) { "\\unknown{}" }
     else { el }
}

proc print_date_element(el)
{
     if (eqstr("ABT", el)) { "$\\sim$" }
                        elsif (eqstr("JAN", el)) { "\\jan{}" }
                        elsif (eqstr("FEB", el)) { "\\feb{}" }
                        elsif (eqstr("MAR", el)) { "\\mar{}" }
                        elsif (eqstr("APR", el)) { "\\apr{}" }
                        elsif (eqstr("MAY", el)) { "\\may{}" }
                        elsif (eqstr("JUN", el)) { "\\jun{}" }
                        elsif (eqstr("JUL", el)) { "\\jul{}" }
                        elsif (eqstr("AUG", el)) { "\\aug{}" }
                        elsif (eqstr("SEP", el)) { "\\sep{}" }
                        elsif (eqstr("OCT", el)) { "\\oct{}" }
                        elsif (eqstr("NOV", el)) { "\\nov{}" }
                        elsif (eqstr("DEC", el)) { "\\dec{}" }
                        else { el }
}

/* Ideally, all the following should go in a TeX library for Lifelines */

proc btag()
{
  "\\btag\\ "
}

proc bapttag()
{
  "\\bapttag\\ "
}



proc mtag()
{
  "\\mtag\\ "
}

proc dtag()
{
  "\\dtag\\ "
}

proc at()
{
  "\\at\\ "
}

proc paperdims(uspaper)
{
  if (not(eq(uspaper,1))) 
     {
     /* this is for A4 paper */
     ",paperwidth=210mm,paperheight=297mm"
     }

  /* US paper is the default */

}

proc already_seen()
{
  "\\subtree{normal}% " nl()
  "\\seeabove " nl()
  "\\endsubtree " nl()
}

func eqstr(s1,s2)
{
  if (eq(strcmp(s1,s2),0)) {return(1)}
  else {return(0)}
}

