/*
 * @progname    burial_index.ll
 * @version     1.0
 * @author      Nicklaus
 * @category
 * @output      Text
 * @description

   Write an (unsorted) list of every person in the database
   whose burial place contains a requested string
   (which is the "town" that this report asks for).
   It matches for the town anywhere in the place field.
   so town can also be a state or county.
   Personally, many of my relatives are from Iowa, so I like to make
   a file of everyone buried in Iowa by entering Iowa to the prompt.


   For MY typical record, which looks like
   1 NAME First /Last/
   1 BIRT
   2 DATE 31 Dec 1900
   1 DEAT
   2 DATE 1 Jan 2000
   1 BURI
   2 PLAC town,county,state
   3 ADDR cemeteryname      (technically should be 2 ADDR acc. to new GEDCOM std.)

   burial_index produces a line which looks like:

       town,cemeteryname : Last, First (1900-2000)

   If your database looks like:
   1 BURI
   2 PLAC cemeteryname,town,county,state
   Then you'll probably want to change this report around a bit. Where
   I do: "getel(parts,1)", you'll want: "getel(parts,2) getel(parts,1)".

   For married women, it attempts to make their name what it may
   be on their tombstone, that is, the surname of their first
   husband, but includes a "nee" (= "born", but without the accent mark)
   and the maiden name.  This generally works great for the standard
   once-married person. If a woman was married multiple times,
   it puts all the husbands' surnames on there, starting with the first
   husband, ending with the maiden surname. So my ancestor, Ruth,
   maiden surname Matthews, who first married E. Scott, 2nd J. Alkire,
   and 3rd married N. Bates, gets an entry like this:
         Scott, a.k.a. Alkire, a.k.a. Bates, nee Matthews, Ruth (1831-1917)
   where a.k.a. stands for "also known as". In doing all this,
   it'll (possibly wrongly) assume both that a woman was married and took
   on the father's surname for any family she was a parent in.
   It's pretty tough to cover every case automatically, so you just
   have to examine and edit the output when it's done if you care.

   It is probably useful to run the output of this through Unix sort.
   There is also a companion program, bury.c, which reformats the sorted
   output to make it prettier.

   An example is at: http://www.geocities.com/grandmashannon/iowa_burials.txt

   Written 1999, Dennis Nicklaus, nicklaus@fnal.gov

*/

proc main ()
{
   list(parts)
   getstrmsg(town, "Enter town for burial index")
        set (town,save(town))

    print("Looking for ") print(town)

        "                         Burials in " town "\n"
    forindi(person, number) {
        set(e,burial(person))

        if (and(e,place(e))) {
                if (index(place(e),town,1)) {
                        extractplaces(e,parts,np)
                        getel(parts,1)
                        call doSite(e) " : "
                        if (female(person)) {
                                /* print out married surnames of women */
                                set(nffam,nfamilies(person))
                                families(person,fam,sp,spi) {
                                        surname(sp)
                                        if (eq(spi,nffam)) {
                                           /* the next IF  is designed to catch a
                                              case where a woman
                                              had one child where the father wasn't
                                              known and she didn't otherwise marry.
                                              In that case, just her maiden
                                              surname will appear, no "nee".
                                              Odd cases will still circumvent this,
                                              and make things look odd, such as
                                              multiple kids by different unknown
                                              fathers, ... I don't care. */

                                           if (or(sp,gt(nffam,1))) {
                                             ", nee "
                                           }
                                        }
                                        else {
                                           /* cover the case where the father's
                                              name isn't known at all.  Don't
                                              print an extra "a.k.a".
                                              odd cases will still look bad,
                                              such as married, then mother with
                                              unknown father. */
                                           if (sp) {
                                              ", a.k.a. "
                                           }
                                        }
                                }
                        }
                        fullname(person,0,0,80)
                        " (" year(birth(person)) "-"    year(death(person)) ")"
                        "\n"
                }
        }
    }
}
proc doSite(event)
{
  fornodes(event, subnode) {
    if (eq(0,strcmp("PLAC", tag(subnode)))) {
      fornodes(subnode, subnode2) {
        if (eq(0,strcmp("ADDR", tag(subnode2)))) {
            ", " value(subnode2)
  }}}}
}



/* bury.c.
   Written 1999, Dennis Nicklaus, nicklaus@fnal.gov
   This program is used as a filter to help format the output of the
   Lifelines report called "burial_index".
   This program makes it so each cemetery name only appears once, with
   the list of people buried in that cemetery listed below it.
   You can compile this simply with:
        cc -o bury bury.c
   To use this, first run the burial index program, then run the
   output of that through Unix's sort (just default arguments to sort),
   then run it through this program.  Suppose your output from burial
   index is called "iowa.txt".
   What I typically do is:
   sort iowa.txt | bury > iowa.sort

   How it works: It just compares each "cemetery name" with the previous one
   in the file.  If the cemetery name is different, it begins a new heading
   for that cemetery, and lists under it each name that follows with the
   same cemetery name.  That's why it's important to run through sort, first.
   For MY typical record, which looks like
   1 NAME First /Last/
   1 BIRT
   2 DATE 31 Dec 1900
   1 DEAT
   2 DATE 1 Jan 2000
   1 BURI
   2 PLAC town,county,state
   3 ADDR cemeteryname
   The lifelines report burial_index produces a line which looks like:

       town,cemetery : Last, First (1900-2000)

   Since I generally make a index for a town, county, or state, running
   sort with default (no) parameters works for me.
   These sorted lines are the input to this program.
*/
/* Start C code.
#include <stdio.h>
char getline (char *line)
{
  char c;
  int in=0;
  c=getchar();
  while ((c != '\n') && (c != EOF)){
    line[in++] = c;
    c=getchar();
  }
  line[in]=0;
  return c;
}
main()
{
  char line[200],last[200],*name;
  int colon,in,maxcompare;
  while(getline(line) != EOF){
    colon = strcspn(line,":");
    maxcompare = strlen(last);
    if (colon > maxcompare) maxcompare = colon;
    if (strncmp(line,last,maxcompare)){
      strncpy(last,line,colon);
      last[colon] = '\0';
      printf("\n\t\t\t%s\n",last);
    }
    name = line+colon+1;
    printf("%s\n",name);
  }
}
 end of C code */
/*  Sample output after going through bury.c:
                        Carlisle, Carlisle Cemetery
 Morgan, Chester Howell (1889-1900)
 Morgan, Elmer Eugene (1861-1931)
 Morgan, nee Dressler, Mary Alice (1861-1950)

                        Carroll
 Walden, nee Lucey, Kathleen J. ``Kay'' (1918-1996)

                        Carroll, Mt. Olivet Cemetery
 Foley, George (1878-1948)
 Foley, nee Cuddy, Mary Cornelia (1885-1972)
 Hamill, Robert J. (1872-1953)
 Hamill, nee Lucey, Jennie Frances (1874-1940)
 Lucey, Edward J. (1849-1922)
 Lucey, George Raymond (1884-1971)
 Lucey, Rosemary (1920-1951)
 Lucey, nee Kemp, Clara Catherine (1887-1969)
 Lucey, Jeremiah ``Jerry'' (1886-1914)
 Lucey, John (1883-1914)
 Lucey, Julia (-1914)
 Lucey, Margaret (-1914)
 Lucey, nee Grace, Mary Elizabeth (1856-1914)

*/
