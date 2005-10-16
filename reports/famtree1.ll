/*
 * @progname       famtree1.ll
 * @version        2
 * @author         James P. Jones, jjones@nas.nasa.gov
 * @category
 * @output         PostScript
 * @description
 *
 *   This report builds a postscript ancestry chart, a "tree", containing
 *   data on five generations. It prompts for the individual to begin with
 *   and draws the tree including this person. The further from this person
 *   the less data is printed. Maximum data include:
 *      o date and place of birth
 *      o date and place of marriage
 *      o date and place of death
 *      o last place of residence
 *      o spouses of person #1 (up to five)
 *    as well as:
 *      o name, address, phone number and e-mail address of compiler
 *      o date of chart
 *
 *   version one:  9 May 1993
 *
 *   Code by James P. Jones, jjones@nas.nasa.gov
 *
 *       Contains code from:
 *       "famrep4"  - By yours truely, jjones@nas.nasa.gov
 *       "pedigree" - By Tom Wetmore, ttw@cbnewsl.att.com
 *                  - and Cliff Manis, cmanis@csoftec.csf.com
 *       "ancestor.ps" - orginial postscript program by Phil Lloyd,
 *                  lloyd@prl.philips.co.uk (See disclaimer below).
 *
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   Note:
 *
 *      o Change the "compiler" data below to reflect yourself.
 *      o In order to take advantage of the "residence" slots on the chart,
 *        use either ADDR (address) or ADDL (address living) tags in your
 *        database, e.g.:
 *
 *             1 ADDL
 *               2 DATE 20 Jun 1992
 *               2 PLAC 619 W Remington Drive, Sunnyvale, CA 94087
 *
 *        Put the full address on one line, separated by commas (,) and this
 *        program will parse the street address from the city/state/zip for
 *        the report.
 */

/*
*	Updated the filename to famtree2 after fixing problems with list vs
*	table variables. 22 September 2002. Paul Buckley.
*/

global(chart)
global(id)
global(savfam)
global(addrnode)
global(compiler)

proc main ()
{
        dayformat(2)
        monthformat(6)
        dateformat(0)
        set(addrnode, NULL)

        /*
         * Change the compiler name, address, phone, and email to reflect
         * yourself.
         */
        table(compiler)
	insert(compiler,"name", getproperty("user.fullname"))
	insert(compiler,"addr", getproperty("user.address"))
	insert(compiler,"phone", getproperty("user.phone"))
	insert(compiler,"email", getproperty("user.email"))

        getindi(indi)
        if (eq(indi, NULL)) {
            print("Individual not found...try again.")
            print(nl())
        }
        else {
            table(chart)
            print("Creating chart...")
            call buildlist(0, 1, indi)
            call doheader()
            call dochart()
            print("done.")
            print(nl())
        }
}

/*
 * Load global array (chart) with all INDI records of all direct ancestors
 * for individual, (indi + 4 generations), indexed by ah number.
 */
proc buildlist(in, ah, indi)
{
        if (par, father(indi)) {
                if (lt(ah, 16)) {
                        call buildlist(add(1,in), mul(2,ah), par)
                }
        }
/*        setel(par, ah, indi)*/
/*        setel(chart, ah, indi)*/
        insert(chart, d(ah), indi)


	if (par, mother(indi)) {
                if (lt(ah, 16)) {
                        call buildlist(add(1,in), add(1,mul(2,ah)), par)
                }
        }
}

/*
 * Find last residence of PERSON by traversing all ADDR and ADDL nodes;
 * save result in global addrnode variable.
 */
proc residence(person)
{
    if (not(person)) {
        set(addrnode, NULL)
    }
    else {
      traverse(inode(person), node, lev) {
        if (or(eq(strcmp(tag(node),"ADDR"),0),eq(strcmp(tag(node),"ADDL"),0))) {
          set(addrnode, node)
        }
      }
    }
}

/*
 * Write postscript header to output file. This is database independant
 * code. The actual genealogical data will be added below.
 */
proc doheader()
{
    "%! " nl()
    "% famtree.ps " nl()
    "% " nl()
    "% This postscript Ancestry Chart was produced by the LifeLines report " nl()
    "% program, famtree1, by James P. Jones. The orginal postscript code " nl()
    "% was written by Phil Lloyd  11th November 1986, the blanks filled in " nl()
    "% with data extracted from a LifeLines Genealogy database. " nl()
    "% ------------------------------------------------------------------- " nl()
    "% " nl()
    "% This original PostScript program was written by Phil Lloyd. " nl()
    "% It may be freely copied, distributed and used, provided these comments " nl()
    "% remain unchanged, and that no fee of any kind is charged for the " nl()
    "% software. " nl()
    "% " nl()
    "% (Except for VERY long names, it should always be possible to condense the " nl()
    "% text to fit into the boxes, without sacrificing readability [by changing " nl()
    "% the font/point size].) " nl()
    "% " nl()
    "% lloyd@prl.philips.co.uk " nl()
    " " nl()
    "% create a standard 1 point Helvetica-Bold font " nl()
    "/bold /Helvetica-Bold findfont def " nl()
    " " nl()
    "% create a standard 1 point Helvetica-Oblique font " nl()
    "/ital /Helvetica-Oblique findfont def " nl()
    " " nl()
    "% box procedure " nl()
    "% 5 arguments: x and y of middle of left-hand side " nl()
    "%              n: entry number ( 1 - 31 ) " nl()
    "%          style: font (bold or ital) " nl()
    "%          point: point size (vertical) " nl()
    "%           cond: `point size' (horizontal) " nl()
    "%              p: person's name " nl()
    "/mtrx matrix def " nl()
    "/box " nl()
    "  { /p exch def " nl()
    "    /cond exch def  /point exch def  /style exch def " nl()
    "    /n exch def  /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    " " nl()
    "    0 setlinewidth " nl()
    " " nl()
    "    /d 16 def  /str 2 string def " nl()
    "    /w 170 def  /h 14 def  /hh h 2 div def  /brad 3 def " nl()
    "    x y translate " nl()
    " " nl()
    "    n 0 ne { " nl()
    "      % black box for number " nl()
    "      newpath " nl()
    "      0.0 0.0 moveto " nl()
    "      0.0 hh  d hh  brad  arcto  4 {pop} repeat " nl()
    "      d hh  d hh neg  brad  arcto  4 {pop} repeat " nl()
    "      d hh neg  0.0 hh neg  brad  arcto  4 {pop} repeat " nl()
    "      0.0 hh neg  0.0 0.0  brad  arcto  4 {pop} repeat " nl()
    "      0.0 0.0 lineto " nl()
    "      fill " nl()
    " " nl()
    "      % white number " nl()
    "      /Helvetica findfont  10 scalefont  setfont " nl()
    "      0.0 hh neg moveto " nl()
    "      n str cvs " nl()
    "      dup stringwidth pop " nl()
    "      neg  d add  2 div  -3.5  moveto " nl()
    "      1.0 setgray  show  0.0 setgray " nl()
    "    } if " nl()
    " " nl()
    "    % box for name " nl()
    "    newpath " nl()
    "    d 0.0 moveto " nl()
    "    d hh  w hh  brad  arcto  4 {pop} repeat " nl()
    "    w hh  w hh neg  brad  arcto  4 {pop} repeat " nl()
    "    w hh neg  d hh neg  brad  arcto  4 {pop} repeat " nl()
    "    d hh neg  d 0.0  brad  arcto  4 {pop} repeat " nl()
    "    d 0.0 lineto " nl()
    "    stroke " nl()
    " " nl()
    "    % name in chosen font " nl()
    "    d 5 add hh neg 3 add moveto " nl()
    "    style [cond 0 0 point 0 0] makefont setfont " nl()
    "    p show " nl()
    "    /Helvetica findfont  10 scalefont  setfont " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% tie procedure " nl()
    "% 4 arguments: x and y of top right-hand end of tie " nl()
    "%              vh: half vertical span of the tie " nl()
    "%              bmul: multiplier to redirect central tail " nl()
    "%                    1: tail faces left " nl()
    "%                    0: tail faces right " nl()
    "/mtrx matrix def " nl()
    "/tie " nl()
    "  { /bmul exch def  /vh exch def  /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    " " nl()
    "    1 setlinewidth " nl()
    " " nl()
    "    /h 10 def  /h2 h 2 mul def  /v vh 2 mul def  /trad 4 def " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 0.0 moveto " nl()
    "    h neg 0.0  h neg vh neg  trad  arcto  4 {pop} repeat " nl()
    "    h neg vh neg  h2 neg bmul mul vh neg  trad  arcto  4 {pop} repeat " nl()
    "    h2 neg bmul mul  vh neg  lineto " nl()
    "    0.0 v neg moveto " nl()
    "    h neg v neg  h neg vh neg  trad  arcto  4 {pop} repeat " nl()
    "    h neg vh neg  h2 neg bmul mul vh neg  trad  arcto  4 {pop} repeat " nl()
    "    stroke " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% dates1 procedure " nl()
    "% 34 arguments: x and y start of first line of text " nl()
    "%               sb1, pb1, cb1: font for the following text " nl()
    "%               b1: first line of text for "born" " nl()
    "%               sb2, pb2, cb2: font for following text: " nl()
    "%               b2: second line of text for "born" " nl()
    "%               sm1, pm1, cm1: font for following text: " nl()
    "%               m1: first line of text for "married" " nl()
    "%               sm2, pm2, cm2: font for following text: " nl()
    "%               m2: second line of text for "married" " nl()
    "%               sr1, pr1, cr1: font for following text: " nl()
    "%               r1: first line of text for "resident" " nl()
    "%               sr2, pr2, cr2: font for following text: " nl()
    "%               r2: second line of text for "resident" " nl()
    "%               sd1, pd1, cd1: font for following text: " nl()
    "%               d1: first line of text for "died" " nl()
    "%               sd2, pd2, cd2: font for following text: " nl()
    "%               d2: second line of text for "died" " nl()
    "/mtrx matrix def " nl()
    "/dates1 " nl()
    "  { /d2 exch def  /cd2 exch def  /pd2 exch def  /sd2 exch def " nl()
    "    /d1 exch def  /cd1 exch def  /pd1 exch def  /sd1 exch def " nl()
    "    /r2 exch def  /cr2 exch def  /pr2 exch def  /sr2 exch def " nl()
    "    /r1 exch def  /cr1 exch def  /pr1 exch def  /sr1 exch def " nl()
    "    /m2 exch def  /cm2 exch def  /pm2 exch def  /sm2 exch def " nl()
    "    /m1 exch def  /cm1 exch def  /pm1 exch def  /sm1 exch def " nl()
    "    /b2 exch def  /cb2 exch def  /pb2 exch def  /sb2 exch def " nl()
    "    /b1 exch def  /cb1 exch def  /pb1 exch def  /sb1 exch def " nl()
    "    /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    "    /voff 23 neg def  /v 19 def  /vv 9 def  /hoff 45 def " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 voff moveto  (born) show " nl()
    "    0.0 voff v sub moveto  (married) show " nl()
    "    0.0 voff v 2 mul sub moveto  (resident) show " nl()
    "    0.0 voff v 3 mul sub moveto  (died) show " nl()
    "    sb1 [cb1 0 0 pb1 0 0] makefont setfont " nl()
    "    hoff voff moveto  b1 show " nl()
    "    sb2 [cb2 0 0 pb2 0 0] makefont setfont " nl()
    "    hoff voff vv sub moveto  b2 show " nl()
    "    sm1 [cm1 0 0 pm1 0 0] makefont setfont " nl()
    "    hoff voff v sub moveto  m1 show " nl()
    "    sm2 [cm2 0 0 pm2 0 0] makefont setfont " nl()
    "    hoff voff v sub vv sub moveto  m2 show " nl()
    "    sr1 [cr1 0 0 pr1 0 0] makefont setfont " nl()
    "    hoff voff v 2 mul sub moveto  r1 show " nl()
    "    sr2 [cr2 0 0 pr2 0 0] makefont setfont " nl()
    "    hoff voff v 2 mul sub vv sub moveto  r2 show " nl()
    "    sd1 [cd1 0 0 pd1 0 0] makefont setfont " nl()
    "    hoff voff v 3 mul sub moveto  d1 show " nl()
    "    sd2 [cd2 0 0 pd2 0 0] makefont setfont " nl()
    "    hoff voff v 3 mul sub vv sub moveto  d2 show " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% dates2 procedure " nl()
    "% 10 arguments: x and y start of first line of text " nl()
    "%               sb1, pb1, cb1: font for following text: " nl()
    "%               b1: first line of text for "born" " nl()
    "%               sb2, pb2, cb2: font for following text: " nl()
    "%               b2: second line of text for "born" " nl()
    "%               sd1, pd1, cd1: font for following text: " nl()
    "%               d1: first line of text for "died" " nl()
    "%               sd2, pd2, cd2: font for following text: " nl()
    "%               d2: second line of text for "died" " nl()
    "/mtrx matrix def " nl()
    "/dates2 " nl()
    "  { /d2 exch def  /cd2 exch def  /pd2 exch def  /sd2 exch def " nl()
    "    /d1 exch def  /cd1 exch def  /pd1 exch def  /sd1 exch def " nl()
    "    /b2 exch def  /cb2 exch def  /pb2 exch def  /sb2 exch def " nl()
    "    /b1 exch def  /cb1 exch def  /pb1 exch def  /sb1 exch def " nl()
    "    /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    "    /voff 23 neg def  /v 19 def  /vv 9 def  /hoff 30 def " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 voff moveto  (born) show " nl()
    "    0.0 voff v sub moveto  (died) show " nl()
    "    sb1 [cb1 0 0 pb1 0 0] makefont setfont " nl()
    "    hoff voff moveto  b1 show " nl()
    "    sb2 [cb2 0 0 pb2 0 0] makefont setfont " nl()
    "    hoff voff vv sub moveto  b2 show " nl()
    "    sd1 [cd1 0 0 pd1 0 0] makefont setfont " nl()
    "    hoff voff v sub moveto  d1 show " nl()
    "    sd2 [cd2 0 0 pd2 0 0] makefont setfont " nl()
    "    hoff voff v sub vv sub moveto  d2 show " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% dates3 procedure " nl()
    "% 10 arguments: x and y start of first line of text " nl()
    "%               sb1, pb1, cb1: font for following text: " nl()
    "%               b1: line of text for "born" " nl()
    "%               sm1, pm1, cm1: font for following text: " nl()
    "%               m1: line of text for "married" " nl()
    "%               sr1, pr1, cr1: font for following text: " nl()
    "%               r1: line of text for "resident" " nl()
    "%               sd1, pd1, cd1: font for following text: " nl()
    "%               d1: line of text for "died" " nl()
    "/mtrx matrix def " nl()
    "/dates3 " nl()
    "  { /d1 exch def  /cd1 exch def  /pd1 exch def  /sd1 exch def " nl()
    "    /r1 exch def  /cr1 exch def  /pr1 exch def  /sr1 exch def " nl()
    "    /m1 exch def  /cm1 exch def  /pm1 exch def  /sm1 exch def " nl()
    "    /b1 exch def  /cb1 exch def  /pb1 exch def  /sb1 exch def " nl()
    "    /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    "    /voff 17 neg def   /v 11 def  /hoff 30 def " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 voff moveto  (born) show " nl()
    "    0.0 voff v sub moveto  (mrrd) show " nl()
    "    0.0 voff v 2 mul sub moveto  (rsdnt) show " nl()
    "    0.0 voff v 3 mul sub moveto  (died) show " nl()
    "    sb1 [cb1 0 0 pb1 0 0] makefont setfont " nl()
    "    hoff voff moveto  b1 show " nl()
    "    sm1 [cm1 0 0 pm1 0 0] makefont setfont " nl()
    "    hoff voff v sub moveto  m1 show " nl()
    "    sr1 [cr1 0 0 pr1 0 0] makefont setfont " nl()
    "    hoff voff v 2 mul sub moveto  r1 show " nl()
    "    sd1 [cd1 0 0 pd1 0 0] makefont setfont " nl()
    "    hoff voff v 3 mul sub moveto  d1 show " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% dates4 procedure " nl()
    "% 6 arguments: x and y start of first line of text " nl()
    "%               sb1, pb1, cb1: font for following text: " nl()
    "%               b1: line of text for "born" " nl()
    "%               sd1, pd1, cd1: font for following text: " nl()
    "%               d1: line of text for "died" " nl()
    "/mtrx matrix def " nl()
    "/dates4 " nl()
    "  { /d1 exch def  /cd1 exch def  /pd1 exch def  /sd1 exch def " nl()
    "    /b1 exch def  /cb1 exch def  /pb1 exch def  /sb1 exch def " nl()
    "    /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    "    /voff 17 neg def   /v 11 def  /hoff 30 " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 voff moveto  (born) show " nl()
    "    0.0 voff v sub moveto  (died) show " nl()
    "    sb1 [cb1 0 0 pb1 0 0] makefont setfont " nl()
    "    hoff voff moveto  b1 show " nl()
    "    sd1 [cd1 0 0 pd1 0 0] makefont setfont " nl()
    "    hoff voff v sub moveto  d1 show " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% dates5 procedure " nl()
    "% 10 arguments: x and y start of first line of text " nl()
    "%               sb1, pb1, cb1: font for following text: " nl()
    "%               b1: text for "born" " nl()
    "%               sm1, pm1, cm1: font for following text: " nl()
    "%               m1: text for "married" " nl()
    "%               sr1, pr1, cr1: font for following text: " nl()
    "%               r1: text for "resident" " nl()
    "%               sd1, pd1, cd1: font for following text: " nl()
    "%               d1: text for "died" " nl()
    "/mtrx matrix def " nl()
    "/dates5 " nl()
    "  { /d1 exch def  /cd1 exch def  /pd1 exch def  /sd1 exch def " nl()
    "    /r1 exch def  /cr1 exch def  /pr1 exch def  /sr1 exch def " nl()
    "    /m1 exch def  /cm1 exch def  /pm1 exch def  /sm1 exch def " nl()
    "    /b1 exch def  /cb1 exch def  /pb1 exch def  /sb1 exch def " nl()
    "    /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    "    /voff 15 neg def   /v 10 def  /wh 85 def  /hoff 15 def " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 voff moveto  (b) show " nl()
    "    0.0 voff v sub moveto  (m) show " nl()
    "    wh voff v sub moveto  (r) show " nl()
    "    wh voff moveto  (d) show " nl()
    "    sb1 [cb1 0 0 pb1 0 0] makefont setfont " nl()
    "    hoff voff moveto  b1 show " nl()
    "    sm1 [cm1 0 0 pm1 0 0] makefont setfont " nl()
    "    hoff voff v sub moveto  m1 show " nl()
    "    sr1 [cr1 0 0 pr1 0 0] makefont setfont " nl()
    "    wh hoff add voff v sub moveto  r1 show " nl()
    "    sd1 [cd1 0 0 pd1 0 0] makefont setfont " nl()
    "    wh hoff add voff moveto  d1 show " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% dates6 procedure " nl()
    "% 6 arguments: x and y start of text " nl()
    "%               sb1, pb1, cb1: font for following text: " nl()
    "%               b1: text for "born" " nl()
    "%               sd1, pd1, cd1: font for following text: " nl()
    "%               d1: text for "died" " nl()
    "/mtrx matrix def " nl()
    "/dates6 " nl()
    "  { /d1 exch def  /cd1 exch def  /pd1 exch def  /sd1 exch def " nl()
    "    /b1 exch def  /cb1 exch def  /pb1 exch def  /sb1 exch def " nl()
    "    /y exch def  /x exch def " nl()
    " " nl()
    "    /savematrix mtrx currentmatrix def " nl()
    "    /voff 15 neg def   /v 10 def  /wh 85 def  /hoff 15 def " nl()
    "    x y translate " nl()
    " " nl()
    "    newpath " nl()
    "    0.0 voff moveto  (b) show " nl()
    "    wh voff moveto  (d) show " nl()
    "    sb1 [cb1 0 0 pb1 0 0] makefont setfont " nl()
    "    hoff voff moveto  b1 show " nl()
    "    sd1 [cd1 0 0 pd1 0 0] makefont setfont " nl()
    "    wh hoff add voff moveto  d1 show " nl()
    " " nl()
    "    savematrix setmatrix " nl()
    "  } def " nl()
    " " nl()
    "% coordinate transform to landscape format " nl()
    "90 rotate  0 -563 translate " nl()
    " " nl()
    "% scaling used for pocket version " nl()
    "% 0.6 0.6 scale " nl()
}

/*
 * Write the rest of the postscript code to the output file, with the
 * blanks filled in with data extracted from the user's LifeLines database.
 */
proc dochart()
{
     set(id, 1)
/*     set(person, getel(chart, id))*/
     set(person, lookup(chart, d(id)))

    "/Helvetica-Bold findfont  14 scalefont  setfont" nl()
    "30 500 moveto  (Ancestry Chart of: "
     name(person) ") show" nl() nl()

    "/Helvetica-Oblique findfont  9 scalefont  setfont" nl()
    "30 485 moveto (Compiled by: " lookup(compiler, "name") ") show" nl()
    "30 475 moveto (" lookup(compiler, "addr") ") show" nl()
    "30 465 moveto (" lookup(compiler, "phone") ") show" nl()
    "30 455 moveto (" lookup(compiler, "email") ") show" nl()

    nl()

    "/Helvetica-Oblique findfont  9 scalefont  setfont" nl()
    "30 440 moveto  (Chart dated:  "
     stddate(gettoday()) ") show" nl() nl()

    "/Helvetica findfont  14 scalefont  setfont" nl()
    "20 270 moveto  (Ancestors of:) show" nl() nl()

     /* loop through all spouse, outputing names */
     spouses(person, svar, fvar, num) {
         if (eq(num, 1)) {
             set(savfam, fvar)
            "/Helvetica findfont  12 scalefont  setfont" nl()
            "36 156 moveto  (spouse: #1) show" nl()
            "20 145 0 bold 9 9 ("
             name(svar)
            ") box" nl()
         }
         if (eq(num, 2)) {
            "/Helvetica findfont  12 scalefont  setfont" nl()
            "36 55 moveto  (other spouses:) show" nl()
            "20 44 0 bold 9 9 (#2: "
             name(svar)
            ") box" nl()
         }
         if (eq(num, 3)) {
            "/Helvetica findfont  12 scalefont  setfont" nl()
            "20 28 0 bold 9 9 (#3: "
             name(svar)
            ") box" nl()
         }
         if (eq(num, 4)) {
            "/Helvetica findfont  12 scalefont  setfont" nl()
            "20 12 0 bold 9 9 (#4: "
             name(svar)
            ") box" nl()
         }
         if (eq(num, 5)) {
            "/Helvetica findfont  12 scalefont  setfont" nl()
            "20 -4 0 bold 9 9 (#5: "
             name(svar)
            ") box" nl()
         }
     }

    "/Helvetica findfont  10 scalefont  setfont" nl() nl()

    "% individual" nl()
    "20 257  1 bold 9 9 ("
    name(person) ") box" nl()
    "20 257" nl()
    call dates1(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "% parents" nl()
    "20 393  2 bold 9 9 ("
    name(person) ") box" nl()
    "20 393" nl()
    call dates1(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "20 121  3 bold 9 9 ("
    name(person) ") box" nl()
    "20 121" nl()
    call dates2(person)
    "20 393 136 0 tie" nl() nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "% grandparents" nl()
    "210 461  4 bold 9 9 ("
    name(person) ") box" nl()
    "210 461" nl()
    call dates1(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "210 325  5 bold 9 9 ("
    name(person) ") box" nl()
    "210 325" nl()
    call dates2(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "210 189  6" nl()
    "bold 9 9 ("
    name(person) ") box" nl()
    "210 189" nl()
    call dates1(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "210 189  6" nl()
    "210  53 7 bold 9 9 ("
    name(person) ") box" nl()
    "210  53" nl()
    call dates2(person)
    "210 461  68 1 tie  210 189  68 1 tie" nl() nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "% great-grandparents" nl()
    "400 495  8 bold 9 9 ("
    name(person) ") box" nl()
    "400 495" nl()
    monthformat(4)
    call dates3(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400 427  9 bold 9 9 ("
    name(person) ") box" nl()
    "400 427" nl()
    call dates4(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400 359 10 bold 9 9 ("
    name(person) ") box" nl()
    "400 359" nl()
    call dates3(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400 291 11 bold 9 9 ("
    name(person) ") box" nl()
    "400 291" nl()
    call dates4(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400 223 12 bold 9 9 ("
    name(person) ") box" nl()
    "400 223" nl()
    call dates3(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400 155 13 bold 9 9 ("
    name(person) ") box" nl()
    "400 155" nl()
    call dates4(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400  87 14 bold 9 9 ("
    name(person) ") box" nl()
    "400  87" nl()
    call dates3(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "400  19 15 bold 9 9 ("
    name(person) ") box" nl()
    "400  19" nl()
    call dates4(person)
    "400 495  34 1 tie  400 359  34 1 tie" nl()
    "400 223  34 1 tie  400  87  34 1 tie" nl() nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "% great-great-grandparents" nl()
    "590 512 16 bold 9 9 ("
    name(person) ") box" nl()
    "590 512" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 478 17 bold 9 9 ("
    name(person) ") box" nl()
    "590 478" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 444 18 bold 9 9 ("
    name(person) ") box" nl()
    "590 444" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 410 19 bold 9 9 ("
    name(person) ") box" nl()
    "590 410" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 376 20 bold 9 9 ("
    name(person) ") box" nl()
    "590 376" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 342 21 bold 9 9 ("
    name(person) ") box" nl()
    "590 342" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 308 22 bold 9 9 ("
    name(person) ") box" nl()
    "590 308" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 274 23 bold 9 9 ("
    name(person) ") box" nl()
    "590 274" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 240 24 bold 9 9 ("
    name(person) ") box" nl()
    "590 240" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 206 25 bold 9 9 ("
    name(person) ") box" nl()
    "590 206" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 172 26 bold 9 9 ("
    name(person) ") box" nl()
    "590 172" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 138 27 bold 9 9 ("
    name(person) ") box" nl()
    "590 138" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590 104 28 bold 9 9 ("
    name(person) ") box" nl()
    "590 104" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590  70 29 bold 9 9 ("
    name(person) ") box" nl()
    "590  70" nl()
    call dates6(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590  36 30 bold 9 9 ("
    name(person) ") box" nl()
    "590  36" nl()
    call dates5(person) nl()

    set(id, add(id, 1))
    set(person, lookup(chart, d(id)))
    "590   2 31 bold 9 9 ("
    name(person) ") box" nl()
    "590   2" nl()
    call dates6(person) nl()

    "590 512  17 1 tie  590 444  17 1 tie" nl()
    "590 376  17 1 tie  590 308  17 1 tie" nl()
    "590 240  17 1 tie  590 172  17 1 tie" nl()
    "590 104  17 1 tie  590  36  17 1 tie" nl() nl()

    "showpage" nl() nl()
}

/* Style #1 for dates information...
 */
proc dates1(person)
{
    list(addrlist)

    "bold 10 9 (" stddate(birth(person)) ")" nl()
    "bold 10 9 (" place(birth(person)) ")" nl()

     set(curfam, 0)
     if (eq(id, 1)) {
         set(curfam, savfam)
     }
     else {
 /*        if (eq(mod(d(id), 2), 0)) {*/
         if (eq(mod(id, 2), 0)) {
/*             set(curfam, parents(getel(chart, div(id, 2))))*/
             set(curfam, parents(lookup(chart, d(div(id, 2)))))
         }
     }
    "bold 10 9 (" stddate(marriage(curfam)) ")" nl()
    "bold 10 9 (" place(marriage(curfam)) ")" nl()

    set(addrnode, NULL)
    call residence(person)

    if (gt(strlen(place(addrnode)), 25)) {
        extractplaces(addrnode, addrlist, n)
        set(line1, NULL)
        set(line1, dequeue(addrlist))
        set(line2, NULL)
        while (not(empty(addrlist))) {
           set(line2, concat(line2, dequeue(addrlist)))
           set(line2, concat(line2, " "))
        }
       "bold 10 9 (" line1 ")" nl()
       "bold 10 9 (" line2 ")" nl()
    }
    else {
        "bold 10 9 (" place(addrnode) ")" nl()
        "bold 10 9 (" ")" nl()
    }
    "bold 10 9 (" stddate(death(person)) ")" nl()
    "bold 10 9 (" place(death(person)) ")" nl()
    "dates1" nl()
}

/* Style #2 for dates information...
 */
proc dates2(person)
{
    "bold 10 9 (" stddate(birth(person)) ")" nl()
    "bold 10 9 (" place(birth(person)) ")" nl()
    "bold 10 9 (" stddate(death(person)) ")" nl()
    "bold 10 9 (" place(death(person)) ")"  nl()
    "dates2" nl()
}

/* Style #3 for dates information...
 */
proc dates3(person)
{
    list(addrlist)

    "bold 10 7.6 (" short(birth(person)) ")" nl()

     set(curfam, 0)
/*     if (eq(mod(d(id), 2), 0)) {*/
     if (eq(mod(id, 2), 0)) {
/*         set(curfam, parents(getel(chart, div(id, 2))))*/
         set(curfam, parents(lookup(chart, d(div(id, 2)))))
     }
    "bold 10 7.6 (" short(marriage(curfam)) ")" nl()
    "bold 10 7.6 "

    set(addrnode, NULL)
    call residence(person)

    if (gt(strlen(place(addrnode)), 25)) {
        extractplaces(addrnode, addrlist, n)
        set(line1, NULL)
        set(line1, dequeue(addrlist))
        set(line2, NULL)
        while (not(empty(addrlist))) {
           set(line2, concat(line2, dequeue(addrlist)))
           set(line2, concat(line2, " "))
        }
       " (" line2 ")" nl()
    }
    else {
        " (" place(addrnode) ")" nl()
    }

    "bold 10 7.6 (" short(death(person)) ")" nl()
    "dates3" nl()
}

/* Style #4 for dates information...
 */
proc dates4(person)
{
    "bold 10 7.6 (" short(birth(person)) ")" nl()
    "bold 10 7.6 (" short(death(person)) ")" nl()
    "dates4" nl()
}

/* Style #5 for dates information...
 */
proc dates5(person)
{
    list(addrlist)

    "bold 10 5 (" short(birth(person)) ")" nl()
     set(curfam, 0)
 /*    if (eq(mod(d(id), 2), 0)) {*/
     if (eq(mod(id, 2), 0)) {
/*         set(curfam, parents(getel(chart, div(id, 2))))*/
         set(curfam, parents(lookup(chart, d(div(id, 2)))))
     }
    "bold 10 5 (" short(marriage(curfam)) ")" nl()

    set(addrnode, NULL)
    call residence(person)

    if (gt(strlen(place(addrnode)), 25)) {
        extractplaces(addrnode, addrlist, n)
        set(line1, NULL)
        set(line1, dequeue(addrlist))
        set(line2, NULL)
        while (not(empty(addrlist))) {
           set(line2, concat(line2, dequeue(addrlist)))
           set(line2, concat(line2, " "))
        }
       "bold 10 5 (" line2 ")" nl()
    }
    else {
        "bold 10 5 (" place(addrnode) ")" nl()
    }
    "bold 10 5 (" short(death(person)) ")" nl()
    "dates5" nl()
}

/* Style #6 for dates information...
 */
proc dates6(person)
{
    "bold 10 5 (" short(birth(person)) ")" nl()
    "bold 10 5 (" short(death(person)) ")" nl()
    "dates6" nl()
}
