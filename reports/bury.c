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
    if (strncmp(line,last,maxcompare)){ /* then they are different */
      strncpy(last,line,colon);
      last[colon] = '\0';
      printf("\n\t\t\t%s\n",last);
    }
    name = line+colon+1;
    printf("%s\n",name);
  }
}
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
