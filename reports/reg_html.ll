/*
 * @progname       reg_html.ll
 * @version        none
 * @author         Wetmore, Prinke
 * @category       
 * @output         HTML
 * @description
 *
 *  The output produces a HTML-marked file (without header) with
 *  one family group per line so that it is displayed on WWW when
 *  found with grep and properly formatted.
 *  Continental European genealogical symbols are used:
 *      * = born     + = died    x = married
 *
 *   Original code by Tom Wetmore, ttw@cbnewsl.att.com, 1990
 *   Modified for HTML/WWW by Rafal Prinke, rafalp@plpuam11.bitnet, 1995
 *
 *   This program is based on regvital by Tom Wetmore. I deleted
 *   all nroff output code and indexing code, and made some other
 *   modifications.
 *
 *
 *  The CGI script I wrote for searching the file and then navigating
 *  through it using the personal key numbers in angle brackets:
 *
 * #!/bin/sh
 * echo Content-type: text/html
 * echo
 * if [ $# = 0 ]
 * then
 *    echo "<HEAD>"
 *    echo "<TITLE>Surname search</TITLE>"
 *    echo "<ISINDEX>"
 *    echo "</HEAD>"
 *    echo "<BODY>"
 *    echo "<H1>Give the surname of the family</H1>"
 *    echo "Regular expressions allowed<P>"
 *    echo "</BODY>"
 * else
 *    echo "<HEAD>"
 *    echo "<TITLE>Search results</TITLE>"
 *    echo "<ISINDEX>"
 *    echo "</HEAD>"
 *    echo "<BODY>"
 *    echo "<H1>Now you can jump to any person displayed</H1>"
 *    echo "type the person's number in angle brackets (lesser/greater)<P>"
 *    grep -i "<ST>$*" <<here goes the path+filename>>
 *    echo "</BODY>"
 * fi
 *
 *
 *
 */

proc main ()
{
    monthformat(2)
    dateformat(5)
    forindi(indi,n) {
        print(" ") print(name(indi)) print(nl())
        call longvitals(indi)
        set(j,1)
        families(indi,fam,spouse,nfam) {
            if (eq(1,nspouses(indi))) {
               "<P><DL><DT>x " }
            else { "<P><DL><DT>x " d(j) ") "
                set(j,add(j,1)) }
            if (eq(0,nchildren(fam))) {
                call spousevitals(spouse,fam)
                ", children not recorded [" key(spouse,1) "]" }
            else {
                call spousevitals(spouse,fam)
                " [" key(spouse,1) "]"
                children(fam,child,nchl) {
                        "<DD>" d(nchl) ". "
                        name(child) " [" key(child,1) "]"
                    }
                }
                "</DL>"
            }
        }
    }

proc longvitals(i)
{
        set(father,father(i))
        set(mother,mother(i))
        nl() "-------------------------<P>"
        if (or(father,mother)) {
        "Parents: "
        if (father)          { name(father) }
        if (and(father,mother)) { " & " }
        if (mother)          { name(mother) }
        }
        "<H2><ST><" key(i,1) ">" givens(i) " <ST>" surname(i) "</ST></H2></ST>"
        set(e,birth(i))
        if(or(date(e),place(e))) { " * " }
        if(date(e)) { stddate(e) ", " }
        if(place(e)) { place(e) ", " }
        set(e,death(i))
        if(or(date(e),place(e))) { " + " }
        if(date(e)) { stddate(e) ", " }
        if(place(e)) { place(e) ", " }
        fornodes(inode(i), node) {
                if (eq(0,strcmp("OCCU", tag(node)))) {
                       value(node) ", "
                }
        }
        fornodes(inode(i), node) {
                if (eq(0,strcmp("NOTE", tag(node)))) {
                        value(node)
                        fornodes(node, subnode) {
                        if (or(eqstr("CONT",tag(subnode)),
			       eqstr("CONC",tag(subnode)))) {
                                        " " value(subnode)
                                }
                        }
                }
        }
}

proc spousevitals (sp,fam)
{
        set(e,marriage(fam))
        if(date(e)) { stddate(e) ", " }
        if(place(e)) { place(e) ", " }
        name(sp)
}
