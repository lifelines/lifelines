/*
 * @progname       xml-dtd.ll
 * @version        1.0
 * @author         Rafal T. Prinke
 * @category       
 * @output         XML DTD
 * @description

       This report produces an XML DTD (Document Type Definition)
       from a LifeLines database. It is intended for comparing
       tag usage and checking for structural inconsistencies
       in a GEDCOM file - especially when sharing the same file
       with others in a research project.

       The LL report produces "tagpairs.txt" file which contains
       a list of unique parent-child tags. Then the perl program
       "dtd.pl" must be run, which will sort that list to
       "tagpairs2.txt" and then write out "llines.dtd" file.

       The perl program is appended at the end of this file.
       It is rather clumsy but works. The resulting DTD does
       not produce errors and can be further processed
       with XML software - but could be better.

       It may also be used in connection with my other
       LL report - xmlize2.ll version 2.2 - which produces
       an XML instance from a LifeLines database.

   xml-dtd.ll - v. 0.2 Rafal T. Prinke,  8 May  2001
                v. 1.0 Rafal T. Prinke, 30 June 2001

*/


global(tagi)
global(content)

proc main ()
{

  list(tagi)
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

newfile("tagpairs.txt",0)
forlist(tagi,o,p) { getel(tagi,p) "\n" }

}


proc out(item)

{
    traverse(root(item),y,level) {
      set(para,concat(tag(parent(y)),"-",tag(y)))
      set(pcdat,concat(tag(y),"-#PCDATA"))
      set(xxref,concat(tag(y),"-|ID"))
      set(xxpnt,concat(tag(y),"-|IDREF"))

/*** parent-child tag pair ***/

      if(eqstr(substring(para,1,1),"-")) {
          set(para,concat("LLGEDCOM-",tag(y)))
      }
      if(not(lookup(content,para))) {
         push(tagi,para)
         insert(content,para,1)
      }

/*** references, xrefs and pcdata ***/


      if(value(y)) {
         if(reference(value(y))) {
            if(not(lookup(content,xxpnt))) {
                push(tagi,xxpnt)
                insert(content,xxpnt,1) }
            }
         else {
            if(not(lookup(content,pcdat))) {
                push(tagi,pcdat)
                insert(content,pcdat,1) }
            }
      }

      if(xref(y)) {
         if(not(lookup(content,xxref))) {
            push(tagi,xxref)
            insert(content,xxref,1)
         }
      }
   }
}


/*** start of dtd.pl ***

#!C:\Perl\bin\perl.exe

$parent = "q";
$firstline = 1;
$attr = 0;

open FILE,"tagpairs.txt";
@tagi = <FILE>;
@stagi = sort @tagi;
close FILE;

open FILE,">tagpairs2.txt";
print FILE @stagi;
close FILE;

open FILE,"tagpairs2.txt";

open DTD,">llines.dtd";
close DTD;

open DTD,">>llines.dtd";

while ($line = <FILE>) {
  chop $line;
  @pair = split(/-/, $line);
  if ($parent eq $pair[0]) {
         if (substr($pair[1],0,1) ne "|") {
            print DTD " | ", $pair[1];
            $attr = 0;
         }
         else {
            if ($attr == 0) { print DTD ")*>" }
            print DTD "\n<!ATTLIST ", $pair[0], " ", substr($pair[1],1),
" ", substr($pair[1],1), " #REQUIRED >";
            $attr = 1;
         }
  }

  else { if ($firstline == 0) {
            if (substr($pair[1],0,1) ne "|") {
                if ($attr == 0) { print DTD ")*>" }
                print DTD "\n", "<!ELEMENT ", $pair[0], " (", $pair[1];
                $parent = $pair[0];
                $attr = 0;
            }
            else {
                if ($attr == 0) { print DTD ")*>" }
                print DTD "\n<!ELEMENT ", $pair[0], " EMPTY>\n<!ATTLIST
", $pair[0], " ", substr($pair[1],1), " ", substr($pair[1],1), "
#REQUIRED >";
                $attr = 1;
                $parent = $pair[0];
            }
         }
         else {
            if (substr($pair[1],0,1) ne "|") {
                $firstline = 0;
                print DTD "<!ELEMENT ", $pair[0], " (", $pair[1];
                $parent = $pair[0];
                $attr = 0;
            }
            else {
                $firstline = 0;
                if ($attr == 0) { print DTD ")*>" }
                print DTD "\n<!ELEMENT ", $pair[0], " EMPTY>\n<!ATTLIST
", $pair[0], " ", substr($pair[1],1), " ", substr($pair[1],1), "
#REQUIRED >";
                $attr = 1;
                $parent = $pair[0];
            }
         }
  }
}

if ($attr == 0) { print DTD ")*>" }

close DTD;

*** end of dtd.pl ***/
