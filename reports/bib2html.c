#include <stdio.h>
/* bib2html.c.  By Dennis Nicklaus nicklaus@fnal.gov, July 1998.
   Converts the bib.tex bibliography file output by the book-latex lifelines report
   into an HTML file, which is suitable for use as the bibliography file
   referenced by the HTML output of the html.dn report.
   
   Compile this simply as 
          cc -o bib2html bib2html.c
   Then run it as a filter (assuming your file/database name is "dad"):
          bib2html < dad-bib.tex > dadbib.html
   

   Things will be a lot nicer if you first sort your bib.tex file by source
   number, something like:
          sort -n -t S -k 2 < dad-bib.tex > bibsort
   and then run it (bibsort) through bib2html

   This simple filter is by no means completely robust.  You might have things
   in your bibliography that will confuse it. (Other LaTex commands, e.g.)
   It can handle {\em text} constructs, but that is about all.
   Ya' get what ya' pay for, I guess.
*/	


main()
{
  char c,word[80],*cptr;

  printf("<HTML>\n<DL>\n");
  while ((c=getchar()) != EOF){
    if (c== '\\'){
      c=getchar();
      if (c== 'b'){ /* then it is a new bibitem */
	while ((c=getchar()) != '{'); /* go to bracket opening bibnumber */
	cptr = word;
	while ((c=getchar()) != '}'){ 
	  *cptr =c;
	  cptr++;
	}
	*cptr = '\0';/* end of bibitem name/number */
	printf("\n<DT> <A NAME=\"%s\" ></A> <B> %s </B>",word,word);
      }
      else{
	if (c== 'e'){ /* then it had better be \em */
	  c=getchar();
	  printf(" <I> ");
	}
      }
    }
    else if (c == '}') 	  printf("</I>");
    else if (c == '{'); /* ignore it */
    else putchar(c);
  }
  printf("</DL>\n");
}

      
