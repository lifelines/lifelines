/*
 * @progname       ps-circle.ll
 * @version        2.6.2 of 2003-12-10
 * @author         Jim Eggert (eggertj@ll.mit.edu), Henry Sikkema (hasikkema@yahoo.ca)
 * @category
 * @output         PostScript
 * @description

		   Print a five to ten-generation ancestry circle chart in PostScript.

Version 2.5, December 2002  		by Henry Sikkema (hasikkema@yahoo.ca)
Version 1.1, September 2002
Version 1, 15 September 1993		by Jim Eggert (eggertj@ll.mit.edu)

This program generates a basic five to ten-generation ancestry circle chart.
Its output is a Postscript file specifying the chart.  This program
uses a modified version of the CIRC.PS program written by
David Campbell and John Dunn.

You must choose the number of generations to print (5 - 10 generations).
For a larger number of generations the print may get VERY small but may
be enlarged using a program such as Corel Draw or other programs and printed
onto a larger paper or printed in parts.

You have the option of creating a colour gradient background or an
alternating colour scheme for males and females.  The gradient does take a while to
process since all I do is to draw and fill circles with decreasing radius.  Please
email (see above) me if you know how to make a better gradient. To change the colours
you need to modify the resulting Postscript file.  The colours are given
in RGB format.  The default colors are RED for female text and BLUE for male text,
the backgrounds are opposite: light blue to female box fillin and light red for
male box fill in.  The default colour gradient is a light brown on the inside
to a darker brown on the outside for an attempted antique look.

http://sikkema.netfirms.com/family/tree/ps-circle/ps-circle.html

The data currently printed depends on the level number and on the length
of the names.  When there are more than one given name (i.e. second and
third names), if they are too long they are eliminated.

The full birth date is printed if there is no known death date.  In this
case, the date is preceeded by 'b:' to indicate that the date is a birth,
for example (b: 12 Sep 1901); the only exception is on level one where
the 'b:' is dropped for the sake of space.  When only a death date is known,
it will be preceeded by a dash, for example (-1978).  In every other case, only
the birth and death years are printed, for example (1901-1978).

The case (capitalization) of the names are not changed at all from the GEDCOM file.

This data is currently printed:
				First line            Second Line          Third line
-----------------------------------------------------------------
Level  1:   Given Names           Surname              Dates
Level  2:   Full Name             Dates                ---
Level  3:   Full Name             Dates                ---
Level  4:   First Name            Surname              Dates
Level  5:   First Name            Surname              Dates
Level  6:   First Name            Surname              Dates
Level  7:   Full Name             Dates                ---
Level  8:   Full Name             Dates                ---
Level  9:   Full Name, Dates      ---                  ---
Level 10:   Full Name, Dates      ---                  ---

Future:  - color coding based on country of origin.  (Robert Simms)
         - marriage date estimate
         - proper zooming in Ghostview
         - eliminate blank pages with small radius
*/

global(indicentre)
global(marrest)
global(version)
global(printmarr)
global(gradient)
global(maxlevel)
global(printdate)
global(numindilines)
global(nummarr)
global(enc_choice)           /* int, specifies character encoding to use */
global(x_pages)
global(y_pages)
global(radius)
global(font_name)

proc removeparentheses(n){
	set(b,index(n,"(",1))
	if(gt(b,0)){/*remove ( if it exists*/
		set(cb,index(n,")",1))
		if(gt(cb,b)){ /*remove upto the ) */
			set(startpt,add(cb,1))
		}else{
			set(startpt,add(b,1))
		}
		set(endpt,strlen(n))
		if(gt(endpt,startpt)){
			set(n,concat(trim(n,sub(b,1)),substring(n,startpt,endpt)))
		}else{
			set(n,trim(n,sub(b,1)))
		}
	}
	set(b,index(n,")",1))
	if(gt(b,0)){/*remove ) if it exists*/
		set(startpt,add(b,1))set(endpt,strlen(n))
		if(gt(endpt,startpt)){
			set(n,concat(trim(n,sub(b,1)),substring(n,startpt,endpt)))
		}else{
			set(n,trim(n,sub(b,1)))
		}
	}
	set(b,substring(n,strlen(n),strlen(n)))
	if(eq(b," ")){  /*remove final space if it exists*/
		set(n,trim(n,sub(strlen(n),1)))
	}
	set(b,index(n,"  ",1))
	if(gt(b,0)){/*remove double space if it exists*/
		set(startpt,add(b,1))set(endpt,strlen(n))
		if(gt(endpt,startpt)){
			set(n,concat(trim(n,sub(b,1)),substring(n,startpt,endpt)))
		}else{
			set(n,trim(n,sub(b,1)))
		}
	}
	n
}
proc put_given_name(person,length){
	if (ne(trimname(person,add(length,strlen(surname(person)),1)),"")){set(l,trimname(person,add(length,strlen(surname(person)),1)))}else{set(l,givens(person))}
	if(ne(trim(l,sub(index(l,surname(person),1),2)),"")){set(n,trim(l,sub(index(l,surname(person),1),2)))}
	call removeparentheses(n)
}

proc put_full_name(person,sur_upper,n_order,length){
	set(n,fullname(person,sur_upper,n_order,length))
	call removeparentheses(n)

}
proc endline(ahnen,offset,info,max){") " d(ahnen) " " d(offset) " " d(info) " " d(max) "} addind\n"}

proc putperson(family, person, level, ahnen, info,dateformat) {
	list(levellength)
	setel(levellength,1,25)
	setel(levellength,2,26)
	setel(levellength,3,23)
	setel(levellength,4,16)
	setel(levellength,5,15)
	setel(levellength,6,15)
	setel(levellength,7,21)
	setel(levellength,8,21)
	setel(levellength,9,21)
	setel(levellength,10,21)
	setel(levellength,11,21)

	set(max,0)
	set(offset,0)

	if(eq(dateformat,1)){
		if (eq(level,1)) {
			if (givens(person)){set(max,add(max,1))}
			if (surname(person)){set(max,add(max,1))}
			if (or(year(death(person)),year(birth(person)))){set(max,add(max,1))}

			if (givens(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" call put_given_name(person,getel(levellength,level)) call endline(ahnen,offset,info,max)}
			if (surname(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" surname(person) call endline(ahnen,offset,info,max)}
			if (or(year(death(person)),year(birth(person)))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" if (year(death(person))){year(birth(person))"-" year(death(person))}else{if(eq(indicentre,0)){"b:"}date(birth(person))}call endline(ahnen,offset,info,max)}

		}elsif(and(ge(level,2),le(level,6))){
			if (givens(person)){set(max,add(max,1))}
			if (surname(person)){set(max,add(max,1))}
			if (or(year(death(person)),year(birth(person)))){set(max,add(max,1))}

			if (givens(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" call put_given_name(person,getel(levellength,level)) call endline(ahnen,offset,info,max)}
			if (surname(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" surname(person) call endline(ahnen,offset,info,max)}
			if (or(year(death(person)),year(birth(person)))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" if (year(death(person))){"("year(birth(person))"-" year(death(person))")"}else{if (year(birth(person))){"b:"date(birth(person))}}call endline(ahnen,offset,info,max)}

		}elsif(or(eq(level,7),eq(level,8))){
			if (or(givens(person),surname(person))){set(max,add(max,1))}
			if (or(year(death(person)),year(birth(person)))){set(max,add(max,1))}

			if (or(givens(person),surname(person))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" call put_full_name(person,0,1,getel(levellength,level)) call endline(ahnen,offset,info,max)}
			if (or(year(death(person)),year(birth(person)))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" if (year(death(person))){"("year(birth(person))"-" year(death(person))")"}else{if (year(birth(person))){"b:"date(birth(person))}}call endline(ahnen,offset,info,max)}
		}elsif(ge(level,9)){set(offset,add(offset,1))set(max,add(max,1))
			set(numindilines,add(numindilines,1))d(numindilines)" {""(" call put_full_name(person,0,1,getel(levellength,level)) " " if (year(death(person))){"("year(birth(person))"-" year(death(person))")"}else{if (year(birth(person))){"b:"date(birth(person))}}	call endline(ahnen,offset,info,max)
		}
	}elsif(ge(dateformat,2)){  /*  (yyyy-yyyy) date format ------------------------------------- */
		if (eq(level,1)) {
			if (givens(person)){set(max,add(max,1))}
			if (surname(person)){set(max,add(max,1))}
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){set(max,add(max,1))}

			if (givens(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" call put_given_name(person,getel(levellength,level)) call endline(ahnen,offset,info,max)}
			if (surname(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" surname(person) call endline(ahnen,offset,info,max)}
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" if (year(birth(person))){year(birth(person))}else{if(eq(dateformat,3)){"    "}} "-" if (year(death(person))){year(death(person))}else{if(eq(dateformat,3)){"    "}}call endline(ahnen,offset,info,max)}

		}elsif(and(ge(level,2),le(level,6))){
			if (givens(person)){set(max,add(max,1))}
			if (surname(person)){set(max,add(max,1))}
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){set(max,add(max,1))}

			if (givens(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" call put_given_name(person,getel(levellength,level)) call endline(ahnen,offset,info,max)}
			if (surname(person)){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "(" surname(person) call endline(ahnen,offset,info,max)}
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "((" if (year(birth(person))){year(birth(person))}else{if(eq(dateformat,3)){"    "}} "-" if (year(death(person))){year(death(person))}else{if(eq(dateformat,3)){"    "}}	")"call endline(ahnen,offset,info,max)}
		}elsif(or(eq(level,7),eq(level,8))){
			if (or(givens(person),surname(person))){set(max,add(max,1))}
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){set(max,add(max,1))}

			if (or(givens(person),surname(person))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "("  call put_full_name(person,0,1,getel(levellength,level)) call endline(ahnen,offset,info,max)}
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){set(numindilines,add(numindilines,1))d(numindilines)" {"set(offset,add(offset,1)) "((" if (year(birth(person))){year(birth(person))}else{if(eq(dateformat,3)){"    "}} "-" if (year(death(person))){year(death(person))}else{if(eq(dateformat,3)){"    "}}	")"call endline(ahnen,offset,info,max)}
		}elsif(ge(level,9)){set(offset,add(offset,1))set(max,add(max,1))
			set(numindilines,add(numindilines,1))d(numindilines)" {""(" call put_full_name(person,0,1,getel(levellength,level))
			if (or(eq(dateformat,3),or(year(death(person)),year(birth(person))))){" (" if (year(birth(person))){year(birth(person))}else{if(eq(dateformat,3)){"    "}} "-" if (year(death(person))){year(death(person))}else{if(eq(dateformat,3)){"    "}}")"}
			call endline(ahnen,offset,info,max)
		}
	}

	if (eq(printmarr,1)){
		if (eq(marrest,1)){ /*marriage date estimation does not yet work!*/
			if (ne(date(marriage(family)),"")){if (eq("M",sex(person))){set(nummarr,add(nummarr,1))d(nummarr)" {(" stddate(marriage(family)) ") " d(ahnen) " " d(info)"} addmarr\n"}}
		}else{
			if (ne(date(marriage(family)),"")){if (eq("M",sex(person))){set(nummarr,add(nummarr,1))d(nummarr)" {(" stddate(marriage(family)) ") " d(ahnen) " " d(info)"} addmarr\n"}}
		}
	}
}

proc semicirc(family, person, level, ahnen, info, maxlevel,dateformat) {

	if (and(person,le(level,maxlevel))) {
		call putperson(family,person,level,ahnen,info,dateformat)
		set(nextlevel, add(level,1))
		set(nextahnen, mul(ahnen,2))
		call semicirc(parents(person), father(person), nextlevel, nextahnen, info,maxlevel,dateformat)
		call semicirc(parents(person), mother(person), nextlevel, add(nextahnen,1), info,maxlevel,dateformat)
	}
}

proc putpageprintouts(xn,yn){
  set(page_num, 0)
  set(yi, sub(yn, 1))
  while(ge(yi, 0)) {
	 set(yi_ord, sub(sub(yn, 1), yi))
	 set(xi, sub(xn, 1))
	 while(ge(xi, 0)) {
		set(page_num, add(page_num, 1))
		"%%Page: " d(page_num) " " d(page_num) "\n"
		"cleartomark mark\n"
		d(xi) " " d(yi) " print-a-page\n"
		"showpage\n"
		set(xi, sub(xi, 1))
	 }
	 set(yi, sub(yi, 1))
  }
}

proc printfile(){
"%!PS-Adobe-3.0\n"
"%%Title: (PS-CIRCLE.PS - Circular Genealogical Pedigree Chart in Postscript format)\n"
"%%Creator: " version " - a Lifelines circle ancestry chart report generator\n"
"%%CreationDate: " stddate(gettoday()) "\n"
"%%Pages: "d(mul(x_pages,y_pages))"\n"
"%%PageOrder: Ascend\n"
"%%Orientation: Portrait\n"
"%%EndComments\n\n"

"%%BeginDefaults\n"
"%%ViewingOrientation: 1 0 0 1\n"
"%%EndDefaults\n\n"

"%%BeginProlog\n\n"
"%   much of the code involved with font encoding and with multipaging\n"
"%   is borrowed from Robert Simms <rsimms@ces.clemson.edu>\n\n"

"%page margins\n"
"/margin_top 20 def\n"
"/margin_bottom 20 def\n"
"/margin_left 20 def\n"
"/margin_right 20 def\n\n"

"%number of pages in each direction\n"

"/xpages "d(x_pages)" def\n"
"/ypages "d(y_pages)" def\n\n"

"/fontname /"font_name" def\n\n"

"/portrait true def\n\n"

"/inch {72 mul} def\n\n"

"/*SF {                 % Complete selectfont emulation\n"  /**/
"  exch findfont exch\n"
"  dup type /arraytype eq {makefont}{scalefont} ifelse setfont\n"
"} bind def\n\n"

"/BuildRectPath{\n"
"	dup type dup /integertype eq exch /realtype eq or{\n"
"			4 -2 roll moveto 	%Operands are: x y width height\n"
"			dup 0 exch rlineto\n"
"			exch 0 rlineto\n"
"			neg 0 exch rlineto\n"
"			closepath\n"
"		}{\n"
"			dup length 4 sub 0 exch 4 exch{\n"
"				1 index exch 4 getinterval aload pop\n"
"				BuildRectPath\n"
"			}for\n"
"			pop\n"
"		}ifelse\n"
"} bind def\n\n"

"/*RC { gsave newpath BuildRectPath fill grestore } bind def\n\n"  /**/

"% install Level 2 emulations, or substitute built-in Level 2 operators\n"
"/languagelevel where\n"
"  {pop languagelevel}{1} ifelse\n"
"2 lt {\n"
"  /RC /*RC load def\n"  /**/
"  /SF /*SF load def\n"  /**/
"}{\n"
"  /RC /rectclip load def      % use RC instead of rectclip\n"
"  /SF /selectfont load def    % use SF instead of selectfont\n"
"} ifelse\n\n"

"%Coordinate conversion utilities\n"
"/polar { %(ang rad) -> (x y)\n"
"	/rad exch def		/ang exch def\n"
"	/x rad ang cos mul def		/y rad ang sin mul def\n"
"	x y\n"
"} def\n\n"

"/midang {\n"
"	/inf exch def\n"
"	inf 1 eq {360 2 maxlevel exp div mul -90.0 add}           %for first level male, go counter clockwise from bottom\n"
"				{360 2 maxlevel exp div mul 90.0 add} ifelse     %for first level female, go clockwise from bottom\n"
"} def\n\n"

"%Shortcut macros\n"
"/m {moveto} def		/l {lineto} def\n\n"

"%Constants\n"
"/pi 3.14159265358979 def\n"
"/ptsize 10 def\n"
"/offset ptsize 1.25 mul neg def\n\n"

"/radius {4.0 7.0 div exch indicentre add mul inch} def\n"

"%begin font encoding   borrowed from Robert Simms\n"
if(ne(enc_choice, 0)) {
	"/encvecmod* {  % on stack should be /Encoding and an encoding array\n"
	"	% make an array copy so we don't try to modify the original via pointer\n"
	"	dup length array copy\n"
	"	encvecmod aload length dup 2 idiv exch 2 add -1 roll exch\n"
	"	{dup 4 2 roll put}\n"
	"	repeat\n"
	"} def\n"
	"/reenc {\n"
	"	findfont\n"
	"	dup length dict begin\n"
	"		{1 index /FID eq {pop pop} {\n"
	"			1 index /Encoding eq {\n"
	"					encvecmod* def\n"
	"				}{def} ifelse\n"
	"			} ifelse\n"
	"		} forall\n"
	"		currentdict\n"
	"	end\n"
	"	definefont pop\n"
	"} def\n"
}
if(eq(enc_choice, 1)) {
	"% Adjust the font so that it is iso-8859-1 compatible\n"
	"/languagelevel where {pop languagelevel}{1} ifelse 2 ge {\n"
	"	/encvecmod* {pop ISOLatin1Encoding} def	% Use built-in ISOLatin1Encoding if PS interpreter is Level 2\n"
	"}{\n"
	/* This array indicates changes to go from the Standard Encoding Vector
		 to the ISOLatin1 Encoding Vector for ISO-8859-1 compatibility,
		 according to the PostScript Language Reference Manual, 2nd ed.
		 The characters from A0 to FF are essential for 8859-1 conformance.
	 */
	"	/encvecmod [\n"
	"		16#90 /dotlessi   16#91 /grave        16#92 /acute      16#93 /circumflex\n"
	"		16#94 /tilde      16#95 /macron       16#96 /breve      16#97 /dotaccent\n"
	"		16#98 /dieresis   16#99 /.notdef      16#9a /ring       16#9b /cedilla\n"
	"		16#9c /.notdef    16#9d /hungarumlaut 16#9e /ogonek     16#9f /caron\n"
	"		16#a0 /space      16#a1 /exclamdown   16#a2 /cent       16#a3 /sterling\n"
	"		16#a4 /currency   16#a5 /yen         16#a6 /brokenbar   16#a7 /section\n"
	"		16#a8 /dieresis   16#a9 /copyright   16#aa /ordfeminine 16#ab /guillemotleft\n"
	"		16#ac /logicalnot 16#ad /hyphen      16#ae /registered  16#af /macron\n"
	"		16#b0 /degree     16#b1 /plusminus   16#b2 /twosuperior 16#b3 /threesuperior\n"
	"		16#b4 /acute      16#b5 /mu          16#b6 /paragraph    16#b7 /periodcentered\n"
	"		16#b8 /cedilla    16#b9 /onesuperior 16#ba /ordmasculine 16#bb /guillemotright\n"
	"		16#bc /onequarter 16#bd /onehalf    16#be /threequarters 16#bf /questiondown\n"
	"		16#c0 /Agrave      16#c1 /Aacute    16#c2 /Acircumflex 16#c3 /Atilde\n"
	"		16#c4 /Adieresis   16#c5 /Aring     16#c6 /AE          16#c7 /Ccedilla\n"
	"		16#c8 /Egrave      16#c9 /Eacute    16#ca /Ecircumflex 16#cb /Edieresis\n"
	"		16#cc /Igrave      16#cd /Iacute    16#ce /Icircumflex 16#cf /Idieresis\n"
	"		16#d0 /Eth         16#d1 /Ntilde    16#d2 /Ograve      16#d3 /Oacute\n"
	"		16#d4 /Ocircumflex 16#d5 /Otilde    16#d6 /Odieresis   16#d7 /multiply\n"
	"		16#d8 /Oslash      16#d9 /Ugrave    16#da /Uacute      16#db /Ucircumflex\n"
	"		16#dc /Udieresis   16#dd /Yacute    16#de /Thorn       16#df /germandbls\n"
	"		16#e0 /agrave      16#e1 /aacute    16#e2 /acircumflex 16#e3 /atilde\n"
	"		16#e4 /adieresis   16#e5 /aring     16#e6 /ae          16#e7 /ccedilla\n"
	"		16#e8 /egrave      16#e9 /eacute    16#ea /ecircumflex 16#eb /edieresis\n"
	"		16#ec /igrave      16#ed /iacute    16#ee /icircumflex 16#ef /idieresis\n"
	"		16#f0 /eth         16#f1 /ntilde    16#f2 /ograve      16#f3 /oacute\n"
	"		16#f4 /ocircumflex 16#f5 /otilde    16#f6 /odieresis   16#f7 /divide\n"
	"		16#f8 /oslash      16#f9 /ugrave    16#fa /uacute      16#fb /ucircumflex\n"
	"		16#fc /udieresis   16#fd /yacute    16#fe /thorn       16#ff /ydieresis\n"
	"	] def\n"
	"} ifelse\n\n"
} elsif(eq(enc_choice, 2)) {
	 /* The following array specifies changes to make to a font encoding
		 to make characters A0 through FF match the ISO Latin alphabet no. 2
		 This will work as long as there are instructions in the font for
		 drawing the glyphs named here.  Missing glyphs would be
		 substituted with /.notdef from the font by the PostScript interpreter.
	*/
	"/encvecmod [\n"
	"	16#a0 /space     16#a1 /Aogonek 16#a2 /breve     16#a3 /Lslash\n"
	"	16#a4 /currency  16#a5 /Lcaron  16#a6 /Sacute    16#a7 /section\n"
	"	16#a8 /dieresis  16#a9 /Scaron  16#aa /Scedilla  16#ab /Tcaron\n"
	"	16#ac /Zacute    16#ad /hyphen  16#ae /Zcaron    16#af /Zdotaccent\n"
	"	16#b0 /degree    16#b1 /aogonek 16#b2 /ogonek    16#b3 /lslash\n"
	"	16#b4 /acute     16#b5 /lcaron  16#b6 /sacute    16#b7 /caron\n"
	"	16#b8 /cedilla   16#b9 /scaron  16#ba /scedilla  16#bb /tcaron\n"
	"	16#bc /zacute    16#bd /hungarumlaut 16#be /zcaron 16#bf /zdotaccent\n"
	"	16#c0 /Racute    16#c1 /Aacute  16#c2 /Acircumflex 16#c3 /Abreve\n"
	"	16#c4 /Adieresis 16#c5 /Lacute  16#c6 /Cacute    16#c7 /Ccedilla\n"
	"	16#c8 /Ccaron    16#c9 /Eacute  16#ca /Eogonek   16#cb /Edieresis\n"
	"	16#cc /Ecaron    16#cd /Iacute  16#ce /Icircumflex 16#cf /Dcaron\n"
	"	16#d0 /Dcroat    16#d1 /Nacute   16#d2 /Ncaron    16#d3 /Oacute\n"
	"	16#d4 /Ocircumflex 16#d5 /Ohungarumlaut 16#d6 /Odieresis 16#d7 /multiply\n"
	"	16#d8 /Rcaron    16#d9 /Uring   16#da /Uacute    16#db /Uhungarumlaut\n"
	"	16#dc /Udieresis 16#dd /Yacute  16#de /Tcommaaccent 16#df /germandbls\n"
	"	16#e0 /racute    16#e1 /aacute  16#e2 /acircumflex 16#e3 /abreve\n"
	"	16#e4 /adieresis 16#e5 /lacute  16#e6 /cacute    16#e7 /ccedilla\n"
	"	16#e8 /ccaron    16#e9 /eacute  16#ea /eogonek   16#eb /edieresis\n"
	"	16#ec /ecaron    16#ed /iacute  16#ee /icircumflex 16#ef /dcaron\n"
	"	16#f0 /dcroat    16#f1 /nacute  16#f2 /ncaron     16#f3 /oacute\n"
	"	16#f4 /ocircumflex 16#f5 /ohungarumlaut 16#f6 /odieresis 16#f7 /divide\n"
	"	16#f8 /rcaron    16#f9 /uring   16#fa /uacute    16#fb /uhungarumlaut\n"
	"	16#fc /udieresis 16#fd /yacute  16#fe /tcommaaccent  16#ff /dotaccent\n"
	"] def\n\n"
} elsif(eq(enc_choice, 3)) {
	 /* This array indicates changes necessary to go from the Standard Encoding
		 Vector to one matching the int'l characters and some others in the
		 IBM Extended Character Set
	 */
	"/encvecmod [\n"
	"	16#80 /Ccedilla    16#81 /udieresis 16#82 /eacute      16#83 /acircumflex\n"
	"	16#84 /adieresis   16#85 /agrave    16#86 /aring       16#87 /ccedilla\n"
	"	16#88 /ecircumflex 16#89 /edieresis 16#8a /egrave      16#8b /idieresis\n"
	"	16#8c /icircumflex 16#8d /igrave    16#8e /Adieresis   16#8f /Aring\n"
	"	16#90 /Eacute      16#91 /ae        16#92 /AE          16#93 /ocircumflex\n"
	"	16#94 /odieresis   16#95 /ograve    16#96 /ucircumflex 16#97 /ugrave\n"
	"	16#98 /ydieresis   16#99 /Odieresis 16#9a /Udieresis   16#9b /cent\n"
	"	16#9c /sterling    16#9d /yen       16#9e /.notdef     16#9f /florin\n"
	"	16#a0 /aacute      16#a1 /iacute    16#a2 /oacute      16#a3 /uacute\n"
	"	16#a4 /ntilde      16#a5 /Ntilde    16#a6 /ordfeminine 16#a7 /ordmasculine\n"
	"	16#a8 /questiondown 16#a9 /.notdef  16#aa /.notdef     16#ab /onehalf\n"
	"	16#ac /onequarter  16#ad /exclamdown 16#ae /guillemotleft  16#af /guillemotright\n"
	"	16#e1 /germandbls  16#ed /oslash    16#f1 /plusminus   16#f6 /divide\n"
	"	16#f8 /degree      16#f9 /bullet\n"
	"] def\n\n"
}
if(ne(enc_choice, 0)) {
	"/gedfont fontname reenc\n"
	"/fontname /gedfont def\n\n"
}
"%end font encoding   end of section borrowed from Robert Simms\n"

if (eq(gradient,1)){
	"/gradient{   %draw and fill 256 circles with a decreasing radius and slightly diffent colour\n"
	"	/blue2 exch def	/green2 exch def	/red2 exch def\n"
	"	/blue1 exch def	/green1 exch def	/red1 exch def\n\n"

	"	/maxrad maxlevel radius def\n"
	"	/delta_r maxrad neg 256 div def                          %find radius step to use\n\n"

	"	gsave\n"
	"		maxrad delta_r 0.0 {                                  %step through the circles from large to small\n"
	"			/r exch def\n"
	"			/ratio r maxrad div def\n"
	"			/red red1 red2 sub ratio mul red2 add def          % work out the new colour\n"
	"			/blue blue1 blue2 sub ratio mul blue2 add def\n"
	"			/green green1 green2 sub ratio mul green2 add def\n\n"

	"			red green blue setrgbcolor\n"
	"			newpath 0.0 0.0 r 0 360 arc fill                   %draw and fill circles\n"
	"		} for\n"
	"	grestore\n"
	"} def\n\n"
}
"/fan{  %Fan Template\n"
	"	gsave\n"
if(or(ne(printmarr,1),ne(transparent,1))){
	"	%begin gender specific shading of boxes\n"
	"	/c 1 def                          %flag for the alternating colours\n"
	"	1 indicentre sub 1 maxlevel {%shade the boxes if necessary\n"
	"		/i exch def\n"
	"		/delta_ang 360.0 2 i exp div def  %set the angle stepsize\n"
	"		/r1 i radius def		/r2 i 1 sub radius def        %find the inner and outer radius for the box\n"
	if (ge(maxlevel,8)){
		"		i 8 ge {0}{0.7 radfactor div} ifelse"
	}else{
		"		.7 radfactor div"
	}
	" setlinewidth                %if level is beyond 7 make lines thinnest possible\n\n"
	"		90.0 delta_ang 449.99 { %step through all angles from 90› to 90›+360› (450›)\n"
	"			/ang1 exch def		/ang2 ang1 delta_ang add def     %find the beginning and ending angle for each box\n"
	"			newpath\n"
	"				i 0 gt{%draw the box\n"
	"					ang1 r1 polar m 0 0 r1 ang1 ang2 arc ang2 r2 polar l 0 0 r2 ang2 ang1 arcn\n"
	"				}{\n"
	"					0 0 1 radius 0 0 1 radius 0 360 arc\n"
	"				}ifelse\n"
	"			closepath\n"
if(eq(transparent,0)){
	"				i 0 gt {                              %fill in box if necessary\n"
	"					c 1 eq {/c1 0 def rf gf bf setrgbcolor} {/c1 1 def rm gm bm setrgbcolor} ifelse\n"
	"				}{\n"
	"					centrepersonsex 0 eq {rm gm bm setrgbcolor} {rf gf bf setrgbcolor} ifelse\n"
	"				}ifelse\n"
	"				gsave fill grestore\n"
	"				i 0 gt{/c c1 def}if                                    %exchange color for next box\n"
	"			rl gl bl setrgbcolor\n\n"
}
if(eq(printmarr,0)){
if(eq(transparent,0)){
	"				i 9 le {stroke} if              %draw outline of box if level is less than 10\n"
}else{
	"				stroke\n"
}
}
	"		}for\n"
	"	}for %end gender specific shading of boxes\n"
}
if (eq(printmarr,1)){
	"	%begin draw boxes around husband and wife\n"
	"	rl gl bl setrgbcolor\n"
	"	2 indicentre sub 1 maxlevel {                    %step through the levels\n"
	"		/i exch def\n"
	if (ge(maxlevel,8)){
		"		i 8 ge {0}{0.7 radfactor div} ifelse"
	}else{
		"		.7 radfactor div"
	}
	" setlinewidth\n\n"
	"		/delta_ang 360.0 2 i 1 sub exp div def  %set the angle stepsize\n"
	"		90.0 delta_ang 449.99 {\n"
	"			/ang1 exch def		/ang2 ang1 delta_ang add def\n"
	"			/r1 i radius def	/r2 i 1 sub radius def\n\n"

	"			%draw tic marks around marriage date\n"
	"			/delta_r r1 r2 sub 15 div def\n"
	"			/angave ang1 delta_ang 2 div add def\n"
	"			/r_inner r2 delta_r add def\n"
	"			/r_outer r1 delta_r sub def\n\n"

	"			newpath angave r_outer polar m angave r1 polar l stroke\n"
	"			r2 0 gt{\n"
	"				newpath angave r2 polar m angave r_inner polar l stroke\n"
	"			}if\n\n"

if(eq(transparent,0)){
	"			rm gm bm setrgbcolor         %erase small gap between male and female\n"
	"			.5 setlinewidth\n"
	"			newpath angave r_outer polar m angave r_inner polar l stroke\n"
	"			rl gl bl setrgbcolor\n"
	if (ge(maxlevel,8)){
		"		i 8 ge {0}{0.7 radfactor div} ifelse"
	}else{
		"		.7 radfactor div"
	}
	" setlinewidth\n"
}

	"			%finish tic marks\n\n"

	"			newpath	%draw box around parents\n"
	"				ang1 r1 polar m 0 0 r1 ang1 ang2 arc\n"
	"				ang2 r2 polar l 0 0 r2 ang2 ang1 arcn closepath\n"
	"			stroke\n"
	"		}for\n"
	"	}for	%end draw boxes around husband and wife\n\n"
}


if (eq(printdate,1)){
	"	0 0 0 setrgbcolor\n"
	"	fontname 5 SF\n"
	"	/radiusprint maxlevel radius 1.01 mul def\n"
	"	datetoday radiusprint 300 circtext\n"
}
"	grestore\n"
"} def\n\n"

"/angtext{   %Angled Line Printing Procedure for outer lines than do not curve\n"
"	/inf exch def		/offst exch def		/ang exch def		/levelnum exch def		/str exch def\n\n"

"	gsave\n"
"	ang rotate                                               %rotate coordinate system for printing\n\n"

"	/r1 levelnum 1 sub radius def		/r2 levelnum radius def\n"
if(eq(printmarr,1)){
"	levelnum 1 eq indicentre 0 eq and{/r1 0 def /r2 0 def}if\n\n"
}
"	/y r1 r2 add 2 div def\n\n"

"	inf 0 eq{0 offst -10 mul 15 add translate}{y 0.0 translate}ifelse\n\n"

"	str stringwidth pop 2 div neg offst moveto\n"
"	str show\n"
"	grestore\n"
"} def\n\n"

"/circtext{   %Circular Line Printing Procedure for inner lines than do curve\n\n"

"	/angle exch def	/textradius exch def	/str exch def\n\n"

"	/xradius textradius ptsize 4 div add def\n"
"	gsave\n"
"		angle str findhalfangle add rotate\n"
"		str {/charcode exch def ( ) dup 0 charcode put circchar} forall\n"
"	grestore\n"
"} def\n\n"

"/findhalfangle {stringwidth pop 2 div 2 xradius mul pi mul div 360 mul} def\n\n"

"/circchar{   %print each character at a different angle around the circle\n"
"	/char exch def\n\n"

"	/halfangle char findhalfangle def\n"
"		gsave\n"
"		halfangle neg  rotate\n"
"		textradius 0 translate\n"
"		-90 rotate\n"
"		char stringwidth pop 2 div neg 0 moveto\n"
"		char show\n"
"	grestore\n"
"	halfangle 2 mul neg rotate\n"
"} def\n\n"

"/setprintcolor{\n"
"	/ahnen exch def		/inf exch def\n"
"	ahnen 2 div dup cvi eq {redmale greenmale bluemale setrgbcolor}{redfemale greenfemale bluefemale setrgbcolor} ifelse\n"
"	ahnen inf mul 1 eq {redmale greenmale bluemale setrgbcolor} if\n"
"} def\n\n"

"/position{  %compute position from ahnentafel number\n"
"	/ahnenn exch def\n"
"	ahnenn 2 maxlevel -1 add exp lt {\n"
"		/a 2 ahnenn log 1.9999 log div floor exp def\n"
"		/numerator 2 a mul -1 add -2 ahnenn a neg add mul add def\n"
"		/fact 2 maxlevel -2 add exp def\n"
"		numerator a div fact mul\n"
"	}{2 maxlevel exp ahnenn neg add} ifelse\n"
"} def\n\n"

"/level {1 add log 2 log div ceiling cvi} def %compute generation level from ahnentafel number\n\n"

"/info{\n"
"	/max exch def		/inf exch def		/noffset exch def		/ahnen exch def\n"
"	/fntfactor {[0 0.85 0.85 0.8 0.7 0.5 0.4 0.3 0.3 0.25 0.25 0.25 0.25] exch get} def %set different font sizes for each level\n\n"

"	ahnen 2 maxlevel exp lt {\n"
"		/place ahnen position def\n"
"		/levelnum ahnen level def    %get the level number of the current person\n"
"		/radtab levelnum radius def  %get the radius of the current level\n"
"		/ftsize ptsize levelnum fntfactor mul def  %find the new fontsize depending on the current level number\n"
"		/offset ftsize 1.25 mul neg def            %find the distance that the text should be printed from the ring\n"
"		inf ahnen setprintcolor      %print the names and information in alternating colors as defined below in line #350\n"
"		fontname ftsize SF %set the font to use\n\n"

"		levelnum 5 lt {levelnum radtab place noffset inf max inner}  % the inner four rings\n"
"						{levelnum place noffset inf 0 max outer} ifelse  % all outer rings\n"
"	} if\n"
"} def\n\n"

if(eq(indicentre,1)){
	"/indiinfo{\n"
	"	/inf exch def		/noffset exch def		/ahnen exch def\n"
	"	/ftsize ptsize 0.9 mul def  %find the new fontsize depending on the current level number\n"
	"	/offset ftsize 1.25 mul neg def            %find the distance that the text should be printed from the ring\n"
	"	inf ahnen setprintcolor      %print the names and information in alternating colors as defined below in line #350\n"
	"	fontname ftsize SF %set the font to use\n\n"

	"	0 0 noffset 0 angtext\n"
	"} def\n\n"
}

"/nstr 7 string def\n"
"/prtn {-0.5 inch 5.5 inch m nstr cvs show} def\n"
"/prt {-0.5 inch 5.5 inch m	show} def\n\n"

if (eq(printmarr,1)){
	"/minfo{\n"
	"	/inf exch def		/ahnen exch def\n"
	"	/fntfactor {[0 0.7 0.7 0.6 0.6 0.5 0.4 0.3 0.3 0.25 0.25 0.25 0.25] exch get} def %set different font sizes for each level\n\n"

	"	ahnen 2 maxlevel exp lt {\n"
	"		/place ahnen 1 eq {0}{ahnen 2 div position}ifelse def  %get the position of the text counting on the outer ring from bottom upwards\n"
	"		/levelnum ahnen level def   %get the level number of the current person\n"
	"		/ftsize ptsize levelnum fntfactor mul 0.80 mul def  %find the new fontsize depending on the current level number\n"
	"		/offset ftsize 0.35 mul neg def            %find the distance that the text should be printed from the ring\n"
	"		rl gl bl setrgbcolor\n"
	"		dup\n"
	"		/namelength exch length def\n"
	"		/f namelength 11 lt {1}{11 namelength div}ifelse def\n"
	"		fontname ftsize f mul SF %set the font to use\n\n"

	"		levelnum place 0 inf 1 1 outer\n"
	"	} if\n"
	"} def\n\n"
}

"/inner{\n"
"	/max exch def		/inf exch def		/noffset exch def		/place exch def		/radtab exch def		/levelnum exch def\n"
"	% slight modifications for each level for line spacing\n"
if(eq(indicentre,0)){
	"		max 3 eq {/factor {[0.0 0.98 0.97 0.97 0.975] exch get} def}if\n"
	"		max 2 eq {/factor {[0.0 0.80 0.885 0.935 0.94] exch get} def}if\n"
	"		max 1 eq {/factor {[0.0 0.70 0.835 0.905 0.91] exch get} def}if\n\n"
}
if(eq(indicentre,1)){
	"		max 3 eq {/factor {[0.0 0.96 0.98 0.98 0.975] exch get} def}if\n"
	"		max 2 eq {/factor {[0.0 0.96 0.935 0.945 0.94] exch get} def}if\n"
	"		max 1 eq {/factor {[0.0 0.96 0.905 0.915 0.91] exch get} def}if\n\n"
}

"	levelnum 1 eq indicentre 0 eq and{/offset offset 0.75 mul def} if  %max the offset a bit smaller for the first level\n"
"	radtab levelnum factor mul noffset offset mul add place inf midang circtext\n"
"} def\n\n"

"/outer{\n"
"	/max exch def	/marr exch def		/inf exch def		/noffset exch def		/place exch def		/levelnum exch def\n\n"

"			% in the following:\n"
"			%      f1 spreads the text out apart from eachother when more positive (larger)\n"
"			%      f2 shifts the set of text counter clockwise when more positive (larger)\n"
if(eq(maxlevel,5)){
	"		max 3 eq {levelnum 5 eq {/f1 -2.5 def	/f2 1.35 def} if}if\n"
	"		max 2 eq {levelnum 5 eq {/f1 -2.5 def	/f2 0.25 def} if}if\n\n"
}
if(eq(maxlevel,6)){
	"		max 3 eq {levelnum 5 eq {/f1 -2.5 def	/f2 6.50 def} if\n"
	"					 levelnum 6 eq {/f1 -1.7 def	/f2 1.50 def} if}if\n"
	"		max 2 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.7 def	/f2 1.50 def} if}if\n\n"
}
if(eq(maxlevel,7)){
	"		max 3 eq {levelnum 5 eq {/f1 -2.5 def	/f2 6.50 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.30 def} if}if\n"
	"		max 2 eq {\n"
	"			 		 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 3.30 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 0.70 def} if}if\n"
	"		max 1 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.30 def} if\n"
	"					 levelnum 7 eq {/f1 -2.0 def	/f2 1.20 def} if}if\n\n"
}
if(eq(maxlevel,8)){
	"		max 3 eq {levelnum 5 eq {/f1 -2.5 def	/f2 6.50 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.30 def} if}if\n"
	"		max 2 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 3.30 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 2.20 def} if\n"
	"					 levelnum 8 eq {/f1 -0.7 def	/f2 0.80 def} if}if\n"
	"		max 1 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 3.30 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 1.50 def} if\n"
	"					 levelnum 8 eq {/f1 -0.7 def	/f2 0.50 def} if}if\n\n"
}
if(eq(maxlevel,9)){
	"		max 3 eq {levelnum 5 eq {/f1 -2.5 def	/f2 6.50 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.30 def} if}if\n"
	"		max 2 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.00 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 2.00 def} if\n"
	"					 levelnum 8 eq {/f1 -0.6 def	/f2 1.40 def} if}if\n"
	"		max 1 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.00 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 2.00 def} if\n"
	"					 levelnum 8 eq {/f1 -0.6 def	/f2 1.40 def} if\n"
	"					 levelnum 9 eq {/f1  0.0 def	/f2 0.00 def} if}if\n\n"
}
if(eq(maxlevel,10)){
	"		max 3 eq {levelnum 5 eq {/f1 -2.5 def	/f2 6.50 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.30 def} if}if\n"
	"		max 2 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.00 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 2.00 def} if\n"
	"					 levelnum 8 eq {/f1 -0.6 def	/f2 1.40 def} if}if\n"
	"		max 1 eq {\n"
	"					 levelnum 5 eq {/f1 -2.5 def	/f2 4.85 def} if\n"
	"					 levelnum 6 eq {/f1 -1.6 def	/f2 4.00 def} if\n"
	"					 levelnum 7 eq {/f1 -1.0 def	/f2 1.70 def} if\n"
	"					 levelnum 8 eq {/f1 -0.6 def	/f2 1.20 def} if\n"
	"					 levelnum 9 eq {/f1  0.0 def	/f2 0.40 def} if\n"
	"					 levelnum 10 ge{/f1  0.0 def	/f2 0.225 def}if}if\n\n"
}

"	marr 1 eq {/f1 0.0 def		/f2 0.0 def} if\n\n"

"	/ang place inf midang f1 noffset mul f2 add add def\n"
"	levelnum ang offset inf angtext\n"
"} def\n\n"

"%   borrowed from Robert Simms\n"
if(eq(indicentre,1)){
	"/addcenterindi {centerperson_array 3 1 roll put} def\n"
}
if(eq(printmarr,1)){
	"/addmarr {marriage_array 3 1 roll put} def\n"
}
	"/addind {person_array 3 1 roll put} def\n\n"
}

proc main() {
	monthformat(4)
	stddate(0)
	dayformat(2)

	set(version, "ps-circle.ll version 2.6.2, 10 December 2003 - code by Henry Sikkema")

	set(numindilines,-1)
	set(nummarr,-1)

	set(mc, -1)

	while (lt(mc,0)){
		list(options)
		setel(options,1,"Family in centre (husband/wife).")
		setel(options,2,"Individual in centre")
		set(mc,menuchoose(options, "Select the number of generations you want printed:"))
		if(eq(mc,0)){break()}
		if(eq(mc,1)){set(indicentre,0)	getfam(fam)}
		if(eq(mc,2)){set(indicentre,1)	getindi(person)}
	}

	list(options)
	setel(options,1,"5 generations.")
	setel(options,2,"6 generations.")
	setel(options,3,"7 generations.")
	setel(options,4,"8 generations.")
	setel(options,5,"9 generations.")
	setel(options,6,"10 generations.")
	set(maxlevel,menuchoose(options, "Select the numbers of generation you want printed:"))
	if(eq(maxlevel,0)){break()}
	set(maxlevel,add(maxlevel,4))

	list(options)
	setel(options,1,"Full birth date info if no date is given: ex b:11 Oct 1758")
	setel(options,2,"Year only format:  example (1758-1823)")
	setel(options,3,"Year only format (spaces for unknown date) ex: (    -1823)")
	set(mc, menuchoose(options, "Select date format:"))
	if(eq(mc,0)){break()}
	if(eq(mc,1)){set(dateformat,1)}
	if(eq(mc,2)){set(dateformat,2)}
	if(eq(mc,3)){set(dateformat,3)}

	list(options)
	setel(options,1,"Yes, print marriage dates only if exact date is known.")
	setel(options,2,"Yes, print marriage date even when estimate is found in file")
	setel(options,3,"No, do not print marriage dates.")
	set(mc, menuchoose(options, "Print marriage dates?"))
	if(eq(mc,0)){break()}
	if(eq(mc,1)){set(printmarr,1)set(marrest,0)}
	if(eq(mc,3)){set(printmarr,0)}
	if(eq(mc,2)){set(printmarr,1)set(marrest,1)}

	list(options)
	setel(options,1,"Colour text (default: blue for males, red for females)")
	setel(options,2,"Black Text  (best for printing on non-colour printers)")
	set(mc, menuchoose(options, "Select text colour option:"))
	if(eq(mc,0)){break()}
	if(eq(mc,1)){set(colourtext,1)}
	if(eq(mc,2)){set(colourtext,0)}

	list(options)
	setel(options,1,"Gender Specific Colour scheme (default: pink for males, light blue for females)")
	setel(options,2,"Transparent Background (best for printing on non-colour printers)")
	setel(options,3,"Gradient Colour scheme")
	set(mc,menuchoose(options, "Select text colour option:"))
	if (eq(mc,0)){break()}
	if (eq(mc,1)){set(alternating,1)set(gradient,0)}
	if (eq(mc,2)){set(alternating,0)set(gradient,0)}
	if (eq(mc,3)){set(alternating,0)set(gradient,1)}

	list(options)
	setel(options,1,"Yes, put on today's date.")
	setel(options,2,"No, do not put on today's date.")
	set(mc,menuchoose(options, "Do you want today's date printed on the circle?"))
	if (eq(mc,0)){break()}
	if (eq(mc,1)){set(printdate,1)}
	if (eq(mc,2)){set(printdate,0)}

	list(options)
	setel(options,1,"Helvetica/Arial")
	setel(options,2,"Times-Roman")
	setel(options,3,"Courier")
	setel(options,4,"AvantGarde-Book")
	setel(options,5,"Times-Roman")
	setel(options,6,"ZapfChancery")

	set(mc,menuchoose(options, "Choose a font to use:"))
	if (eq(mc,0)){break()}
	if (eq(mc,1)){set(font_name,"Helvetica")}
	if (eq(mc,2)){set(font_name,"Times-Roman")}
	if (eq(mc,3)){set(font_name,"Courier")}
	if (eq(mc,4)){set(font_name,"AvantGarde-Book")}
	if (eq(mc,5)){set(font_name,"Palatino-Roman")}
	if (eq(mc,6)){set(font_name,"ZapfChancery")}

	list(options)
	setel(options,1,"Single page (maximum circle size on a single page)")
	setel(options,2,"Multipage according to number of pages selected")
	setel(options,3,"Multipage according to radius of chart")
	set(mc,menuchoose(options, "Select page type: "))
	if (eq(mc,0)){break()}
	if (eq(mc,1)){
		set(x_pages,1)set(y_pages,1)set(radius,0)
	}
	if(gt(mc,1)){
		print(   "Radius (inches)  # of pages  Radius (inches)  # of pages"
			,nl(),"  0-8               1x1=1     32-33             4x4=16"
			,nl(),"  8-10              2x1=2     33-42             5x4=20"
			,nl()," 10-16              2x2=4     42-43             6x4=24"
			,nl()," 16-21              3x2=6     43-50             6x5=30"
			,nl()," 21-25              3x3=9     50-54             7x5=35"
			,nl()," 25-32              4x3=12    54-59             7x6=42",nl()
		)
	}
	if (eq(mc,2)){
		getint( x_pages, "Number of horizontal portrait pages on chart")
		getint( y_pages, "Number of vertical portrait pages on chart")
		set(radius,0)
	}
	if (eq(mc,3)){
		getint(radius, "Enter desired radius in inches:")
		if (le(radius,8)){set(x_pages,1)set(y_pages,1)}
		if (and(ge(radius,8),lt(radius,10))){set(x_pages,2)set(y_pages,1)}
		if (and(ge(radius,10),lt(radius,16))){set(x_pages,2)set(y_pages,2)}
		if (and(ge(radius,16),lt(radius,21))){set(x_pages,3)set(y_pages,2)}
		if (and(ge(radius,21),lt(radius,25))){set(x_pages,3)set(y_pages,3)}
		if (and(ge(radius,25),lt(radius,32))){set(x_pages,4)set(y_pages,3)}
		if (and(ge(radius,32),lt(radius,33))){set(x_pages,4)set(y_pages,4)}
		if (and(ge(radius,33),lt(radius,42))){set(x_pages,5)set(y_pages,4)}
		if (and(ge(radius,42),lt(radius,43))){set(x_pages,6)set(y_pages,4)}
		if (and(ge(radius,43),lt(radius,50))){set(x_pages,6)set(y_pages,5)}
		if (and(ge(radius,50),lt(radius,54))){set(x_pages,7)set(y_pages,5)}
		if (and(ge(radius,54),lt(radius,59))){set(x_pages,7)set(y_pages,6)}
	}
	print(nl())
/*
**  ISO-Latin 1, or ISO 8859-1, is a world-wide standard for most languages
**  of Latin origin: Albanian, Basque, Breton, Catalan, Cornish, Danish, Dutch
**  English, Faroese, Finish (exc. S,s,Z,z with caron),
**  French (exc. OE, oe, Y with dieresis), Frisian, Galician, German,
**  Greenlandic, Icelandic, Irish Gaelic (new orthography), Italian, Latin,
**  Luxemburgish, Norwegian, Portuguese, Rhaeto-Romanic, Scottish Gaelic,
**  Spanish, Swedish.
**
**  ISO Latin 2, or ISO 8859-2, covers these languages:  Albanian, Croatian,
**  Czech, English, German, Hungarian, Latin, Polish, Romanian (cedilla below
**  S,s,T,t instead of comma), Slovak, Sloverian, Sorbian.
*/
	 list(options)
	 setel(options, 1, "ISO Latin 1 most West European languages")
	 setel(options, 2, "ISO Latin 2 Central and East European languages")
	 setel(options, 3, "IBM PC (covers at least the international chars)")
	 set(enc_choice, menuchoose(options,
		 "Select font reencoding, or (q) to use what's in the fonts"))
	if (eq(enc_choice,0)){break()}

	call printfile()

	if (eq(printdate,1)){
		monthformat(6) /*capitalized full word (eg, January, February) */
		"/datetoday (Date: " stddate(gettoday()) ") def\n\n"
		monthformat(4) /*capitalized abbreviation (eg, Jan, Feb) */
	}

	"/indicentre "d(indicentre)" def %1=put individual in centre,0=family at centre\n"
	if(eq(indicentre,1)){if(eq(sex(person),"M")){set(psex,0)}else{set(psex,1)}
	"/centrepersonsex "d(psex)" def %0=male; 1=female\n\n"}else{"\n"}

	"/maxlevel " d(maxlevel) " def\n"

	"% color  of the text in RGB format\n"
	if(eq(colourtext,1)){
		"/redmale   0.0 def  /greenmale   0.0 def  /bluemale   1.0 def\n"
		"/redfemale 1.0 def  /greenfemale 0.0 def  /bluefemale 0.0 def\n\n"
	}else{
		"/redmale   0.0 def  /greenmale   0.0 def  /bluemale   0.0 def\n"
		"/redfemale 0.0 def  /greenfemale 0.0 def  /bluefemale 0.0 def\n\n"
	}

	if (eq(gradient,1)){
		"0.6431 0.3255 0.0228  % inside centre color in RGB format\n"
		"0.9922 0.7686 0.5490  % outside rim color in RGB format    to form a radial gradient\n"
		"gradient\n\n"
		"/transparent 1 def         % 1=transparent, 0=color shading\n\n"

		"/rf 0.0 def /gf 0.0 def /bf 0.0 def %rgb female box fill\n"
		"/rm 0.0 def /gm 0.0 def /bm 0.0 def %rgb male box fill\n\n"

	}else{
		if (eq(alternating,0)){
			"/transparent 1 def         % 1=transparent, 0=color shading\n\n"

			"/rf 1.0 def /gf 1.0 def /bf 1.0 def %rgb female box fill\n"
			"/rm 1.0 def /gm 1.0 def /bm 1.0 def %rgb male box fill\n\n"
		}else{
			"/transparent 0 def         % 1=transparent, 0=color shading\n\n"

			"/rf 0.8 def /gf 0.8 def /bf 1.0 def %rgb female box fill\n"
			"/rm 1.0 def /gm 0.8 def /bm 0.8 def %rgb male box fill\n\n"
		}
	}
/*	"/printmarr "d(printmarr)" def\n"*/

	"/rl 0.0 def /gl 0.0 def /bl 0.0 def %  rgb for lines\n"

"%     partially borrowed from Robert Simms\n"
"% Find printable dimension for chart with a sequence of steps\n\n"

"% get printable area for each page\n"
"clippath pathbbox newpath\n"
"/ury exch def /urx exch def\n"
"/lly exch def /llx exch def\n\n"

"/llx llx margin_left add def /lly lly margin_bottom add def\n"
"/urx urx margin_right sub def /ury ury margin_top sub def\n\n"

"% get available width and height for printing on a sheet of paper\n"
"/wp urx llx sub def\n"
"/hp ury lly sub def\n\n"

"% get width and height of the multi-page printable area\n"
"/tw0 wp xpages mul def\n"
"/th0 hp ypages mul def\n\n"

"tw0 th0 gt {\n"
if(eq(radius,0)) {"	/mindim th0 def\n"}
"	th0 wp div ceiling cvi xpages lt {/xpages th0 wp div ceiling cvi def /tw0 wp xpages mul def /ypages ypages def}{/xpages xpages def /ypages ypages def}ifelse\n"
"}{\n"
if(eq(radius,0)) {"	/mindim tw0 def\n"}
"	tw0 hp div ceiling cvi ypages lt {/ypages tw0 hp div ceiling cvi def /th0 hp ypages mul def /xpages xpages def}{/xpages xpages def /ypages ypages def}ifelse\n"
"}ifelse\n\n"

if(gt(radius,0)) {
	"/radfactor " d(radius) " inch 8 inch div def\n"
}else{
	"/radfactor mindim 8 inch div def\n"
}
"/scalefactor 7.0 maxlevel indicentre add div radfactor mul def\n\n"

"/print-a-page { % page printing procedure\n"
"	/ypage exch ypages 2 div 1 sub sub def  %y-correction to center chart\n"
"	/xpage exch xpages 2 div 1 sub sub def  %x-correction to center chart\n"
"	ypage ypages lt xpage xpages lt and { %only print if page is in correct range\n"
"		gsave\n"
"			llx lly translate\n"
"			0 0 wp hp RC		% specify (rectangular) clipping path to keep the margins clean\n"
"			xpage wp mul ypage hp mul translate	% move origin so that desired portion of chart lands within clipping path\n"
"			scalefactor dup scale  %enlarge scale to fit page\n"
"			fan  %draw circle template\n"
	if(eq(indicentre,1)){"			centerperson_array {exec indiinfo} forall %put in center person\n"}
								"			person_array {exec info} forall %put in all people with dates\n"
	if(eq(printmarr,1)) {"			marriage_array {exec minfo} forall %put in marriage dates\n"}
"			1 dup scale %reset scale to normal\n"
"		grestore\n"
"	} if\n"
"} def      % print-a-page procedure\n\n"

"%%EndProlog\n"
"%%BeginSetUp\n\n"

"/fillarray{% store vertical lines and individual records in arrays\n"

if(eq(indicentre,1)){
	"0 {(" call put_given_name(person,20) ") " d(psex) " 1 0} addcenterindi\n"
	"1 {(" surname(person) ") " d(psex) " 2 0} addcenterindi\n"
	"2 {(" if (or(eq(dateformat,0),year(death(person)))){year(birth(person))"-" year(death(person))}else{date(birth(person))}") " d(psex) " 3 0} addcenterindi\n"
	call semicirc(parents(person),father(person),1,1,1,maxlevel,dateformat)
	call semicirc(parents(person),mother(person),1,1,2,maxlevel,dateformat)
}else{
	call semicirc(fam,husband(fam),1,1,1,maxlevel,dateformat)
	call semicirc(fam,wife(fam),1,1,2,maxlevel,dateformat)
}
"} def\n\n"

if(eq(indicentre,1)){"/centerperson_array 3 array def\n"}
if(eq(printmarr,1)){"/marriage_array "d(add(nummarr,1))" array def\n"}
"/person_array "d(add(numindilines,1))" array def\n"

"fillarray\n\n"

"mark\n\n"
"%%EndSetUp\n"
call putpageprintouts(x_pages,y_pages)
"%%EOF\n"
print("Output file full-name: ", outfile(), nl())
}
