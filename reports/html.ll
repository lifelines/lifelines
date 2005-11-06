/*
 * @progname       html.ll
 * @version        1.0
 * @author         Dave Close, <dave@compata.com>
 * @category
 * @output         HTML
 * @description
 *
 *                 Produces a set of interlinked HTML files, one for each
 *                 person in the data base, and a master name index file.

Here is a report program I've been using to
generate HTML to allow browsing of my data base. It produces one HTML
file for each person in the data base, and a master index file with
links to each of the other files. Each person's file is hyperlinked
to all his direct relatives. I believe the output is standard HTML and
does not use any peculiar extensions, so it should be viewable with
nearly any browser.
Permission is granted to anyone to use this code for any purpose.

It does call the divorce() function which I added to my copy of
Lifelines and posted to this list 2002 November 12.

See:
http://listserv.nodak.edu/cgi-bin/wa.exe?A2=ind0211b&L=lines-l&T=0&F=&S=&P=356
 */

global ( page_width )
global ( left_margin )

proc main ()
{
  set ( page_width, 500 )
  set ( left_margin, 0 )
  indiset ( iset )

  forindi ( indi, ni )
  {
    addtoset ( iset, indi, 1 )

    newfile ( concat ( key ( indi ), ".html" ), 0 )
    "<html>" nl () "<head>" nl ()
    "<title>" name ( indi ) "</title>" nl ()
    "</head>" nl () "<body bgcolor=white>" nl ()
    "<h1>" name ( indi ) "</h1>" nl ()

    /* person */
    if ( male ( indi ) )
    {
      "Male<br>" nl ()
    }
    else
    {
      "Female<br>" nl ()
    }
    dayformat ( 1 ) monthformat ( 1 ) dateformat ( 11 )
    set ( dat, atoi ( stddate ( birth ( indi ) ) ) )
    if ( ne ( dat, 0 ) )
    {
      dayformat ( 2 ) monthformat ( 4 ) dateformat ( 8 )
      "Born: " stddate ( birth ( indi ) ) ", "
      place ( birth ( indi ) ) "<br>" nl ()
    }
    else
    {
      "<font color=red> Birth information missing </font><br>" nl ()
    }
    dayformat ( 1 ) monthformat ( 1 ) dateformat ( 11 )
    set ( dat, atoi ( stddate ( death ( indi ) ) ) )
    if ( ne ( dat, 0 ) )
    {
      dayformat ( 2 ) monthformat ( 4 ) dateformat ( 8 )
      "Died: " stddate ( death ( indi ) ) ", "
      place ( death ( indi ) ) "<br>" nl ()
    }
    "<br>" nl ()

    /* parents */
    set ( pa, father ( indi ) )
    set ( ma, mother ( indi ) )
    if ( eqstr ( key ( pa ), "" ) )
    {
      "<font color=red> Father unknown </font><br>" nl ()
    }
    else
    {
      "Father: <a href=" key ( pa ) ".html>" name ( pa ) "</a><br>" nl ()
    }
    if ( eqstr ( key ( ma ), "" ) )
    {
      "<font color=red> Mother unknown </font><br>" nl ()
    }
    else
    {
      "Mother: <a href=" key ( ma ) ".html>" name ( ma ) "</a><br>" nl ()
    }
    "<br>" nl ()

    /* families */
    if ( gt ( nfamilies ( indi ), 0 ) )
    {
      if ( eq ( nfamilies ( indi ), 1 ) )
      {
        "Family:"
      }
      else
      {
        "Families:"
      }
      "<table border=1>" nl ()
      families ( indi, fam, sp, num )
      {
        "<tr><td valign=top>Spouse: <a href=" key ( sp ) ".html>"
        name ( sp ) "</a><br>" nl ()
        if ( marriage ( fam ) )
        {
          "Married: "
          dayformat ( 1 ) monthformat ( 1 ) dateformat ( 11 )
          set ( dat, atoi ( stddate ( marriage ( fam ) ) ) )
          if ( ne ( dat, 0 ) )
	  {
            dayformat ( 2 ) monthformat ( 4 ) dateformat ( 8 )
            stddate ( marriage ( fam ) ) ", "
	  }
	  else
	  {
	    "<font color=red>Date unknown, </font>"
	  }
	  if ( eqstr ( place ( marriage ( fam ) ), "" ) )
	  {
	    "<font color=red>Place unknown</font><br>" nl ()
	  }
	  else
	  {
            place ( marriage ( fam ) ) "<br>" nl ()
	  }
        }
        if ( divorce ( fam ) )
        {
          "Divorced: "
          dayformat ( 1 ) monthformat ( 1 ) dateformat ( 11 )
          set ( dat, atoi ( stddate ( divorce ( fam ) ) ) )
          if ( ne ( dat, 0 ) )
	  {
            dayformat ( 2 ) monthformat ( 4 ) dateformat ( 8 )
            stddate ( divorce ( fam ) ) ", "
	  }
	  else
	  {
	    "<font color=red>Date unknown, </font>"
	  }
	  if ( eqstr ( place ( divorce ( fam ) ), "" ) )
	  {
	    "<font color=red>Place unknown</font><br>" nl ()
	  }
	  else
	  {
            place ( divorce ( fam ) ) "<br>" nl ()
	  }
        }
        dayformat ( 1 ) monthformat ( 1 ) dateformat ( 11 )
        set ( dsp, atoi ( stddate ( death ( sp ) ) ) )
        set ( din, atoi ( stddate ( death ( indi ) ) ) )
        set ( ddv, atoi ( stddate ( divorce ( fam ) ) ) )
        if ( ne ( dsp, 0 ) )
        {
          if ( lt ( dsp, din ) )
          {
            dayformat ( 2 ) monthformat ( 4 ) dateformat ( 8 )
            "Widowed: " stddate ( death ( sp ) ) "<br>" nl ()
          }
        }
        "</td><td valign=top>"
        if ( eq ( nchildren ( fam ), 0 ) )
        {
          "No children" nl ()
        }
        else
        {
          "Children:<br>" nl () children ( fam, child, no )
          {
            "<a href=" key ( child ) ".html>" name ( child ) "</a><br>" nl ()
          }
        }
        "</td></tr>" nl ()
      }
      "</table>" nl ()
    }
    else
    {
      "<font color=red>No family information known</font><br>" nl ()
    }
    "<br><a href=tree.html> Return to complete list of persons </a>" nl ()
    "</body>" nl () "</html>" nl ()
  }

  newfile ( "tree.html", 0 )
  "<html>" nl () "<head>" nl ()
  "<title> Persons </title>" nl ()
  "</head>" nl () "<body bgcolor=white>" nl ()
  "<p>The following persons are recorded in this data base." nl ()
  "After selecting any one of them, you may proceed directly to their" nl ()
  "direct relatives through additional links, or return to this page.</p>" nl ()
  "<table valign=top><tr><td valign=top>" nl ()
  namesort ( iset )
  set ( n1, lengthset ( iset ) )
  incr ( n1 ) incr ( n1 )
  set ( n2, div ( n1, 3 ) )
  forindiset ( iset, indi, a, b )
  {
    "<a href=" key ( indi ) ".html>"
    surname ( indi ) ", " givens ( indi ) "</a><br>" nl ()
    if ( eq ( mod ( b, n2 ), 0 ) )
    {
      "</td><td valign=top>" nl ()
    }
  }
  "</td></tr></table></body></html>" nl ()
}
