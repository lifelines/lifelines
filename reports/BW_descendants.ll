/*
 * @progname    BW_descendants.ll
 * @version     1.00
 * @author      Birger Wathne
 * @category
 * @output      Text
 * @description
 *              List successors with notes

BW_descendants - a LifeLines report by Birger Wathne (Birger.Wathne@sdata.no)

Version 1.00
This program partially based on code by Brad Frecker and Dick Knowles.
Suggestions and comments welcome.


Output sample at bottom of this source file.

This report generates a list of successors of the given person. For each
successor and its spouse(s), all level 1 notes are listed.

The report asks for some parameters:
- Number of generations to include (0 gives all)
- Amount of output for spouse's other families
        0 - Only list the main person's marriages. Don't even mention
            the fact that the spouse(s) have had other relationships.
        1 - Print a one-line summary for each of the spouse's other
            relationships (number of children, spouses name, etc)
        2 - Print the one-liner from option 1, plus a full listing of
            all stepchildren (not recursive)
- Output type
        0 - Text. Plain ascii output (like the sample)
        1 - roff. Not finished, as I don't have an 8-bit clean roff.
            If someone wants this, please finish it, and send me the code.
        2 - HTML. This output uses <TABLE> tags, so you need HTML 3.0 support.
            Uses bold fonts, etc. Nice....
- Language for generated text. The header, and all those small words
  used in the output can be generated in the language you have your
  data. Makes it look more natural. If you add new languages, please tell me.
*/

global(strings)
global(outputtype)
global(true)
global(false)

func init_strings(language) {
        if ( eq( language, 0)) {
                insert(strings, "Header", "Successors of")
                insert(strings, "Headerdate", "Date")
                insert(strings, "Born", "Born")
                insert(strings, "Dead", "Dead")
                insert(strings, "Married", "Married")
                insert(strings, "Relationship", "Relationship")
                insert(strings, "with", "with")
                insert(strings, "unknownspouse", "unknown spouse")
                insert(strings, "descendants", "descendants")
                insert(strings, "generations", "generations")
                insert(strings, "children", "children")
                insert(strings, "had", "had")
                insert(strings, "all", "all")
                insert(strings, "Notesfor", "Notes for")

                return(0)
        }

        if ( eq( language, 1)) {
                insert(strings, "Header", "Etterkommere etter")
                insert(strings, "Headerdate", "Dato")
                insert(strings, "Born", "F輐t")
                insert(strings, "Dead", "D輐")
                insert(strings, "Married", "Gift")
                insert(strings, "Relationship", "Forhold")
                insert(strings, "with", "med")
                insert(strings, "unknownspouse", "ukjent ektefelle")
                insert(strings, "descendants", "etterkommere")
                insert(strings, "generations", "generasjoner")
                insert(strings, "children", "barn")
                insert(strings, "had", "hadde")
                insert(strings, "all", "alle")
                insert(strings, "Notesfor", "Notater for")

                return(0)
        }

        return(1)
}


proc main () {
        table(strings)

        set(true, 1)
        set(false, 0)

        dayformat(0)
        monthformat(4)
        dateformat(0)

        getindi(indi)

        getintmsg (generation_count, "How many generations (0 for all)?")
        if ( lt( generation_count, 0)) {
                print("Illegal number of generations")
                return()
        }

        getintmsg (
                spousefamilies,
                "How much output for spouse's other families (0=none, 1=summary, 2=list children)?"
        )
        if ( or( lt( spousefamilies, 0), gt( spousefamilies, 2))) {
                print("Illegal answer")
                return()
        }

        getintmsg(outputtype, "Output type (0=TEXT, 1=ROFF, 2=HTML)?")
        if ( or( lt( outputtype, 0), gt( outputtype, 2))) {
                print("Illegal output type")
                return()
        }


        getintmsg(
                language,
                "Language for generated text (0=English, 1=Norwegian)?"
        )
        if ( ne( init_strings(language), 0)) {
                print("Couldn't initialize string table to selected language")
                return()
        }

        output_init()
        /* Headers */
        if ( eq( generation_count, 0)) {
                output_header1 (
                        concat(
                                lookup( strings, "Header")," ",
                                name(indi), " ",
                                lookup( strings, "with"), " ",
                                lookup( strings, "all"), " ",
                                lookup( strings, "descendants")
                        )
                )
        } else {
                if ( eq( generation_count, 1)) {
                        output_header1 (
                                concat(
                                        lookup( strings, "Header"), " ",
                                        name(indi), " "
                                )
                        )
                } else {
                        output_header1 (
                                concat(
                                        lookup( strings, "Header"), " ",
                                        name(indi), " ",
                                        lookup( strings, "with"), " ",
                                        d(sub(generation_count,1)), " ",
                                        lookup( strings, "generations"), " ",
                                        lookup( strings, "descendants")
                                )
                        )
                }
        }
        nl() nl()

        output_header2(
                concat(
                        lookup( strings, "Headerdate"), ": ",
                        stddate(gettoday())
                )
        )
        nl() nl()

        call descendants(indi, "1", generation_count, spousefamilies)
        output_terminate()
}



proc descendants (indi, number, generation_count, spousefamilies) {
        output_startpara()
        number output_linebreak()
        call print_person(indi, 1)

        call write_notes(indi)

        set(childnumber, 0)

        families(indi, family, spouse, i) {
                if(e, marriage(family)) {
                        lookup( strings, "Married") " " stddate(e)
                } else {
                        lookup( strings, "Relationship")
                }

                if ( ne( spouse, null)) {
                        " " lookup( strings, "with") " "
                        call print_person(spouse, 1)
                } else {
                        " " lookup( strings, "with") " "
                        lookup( strings, "unknownspouse")
                }

                if ( ne(spousefamilies, 0)) {
                        families(spouse, spfamily, spspouse, j) {
                                if (ne(family, spfamily)) {
                                        name(spouse) " "
                                        lookup( strings, "had") " "
                                        d(nchildren(spfamily)) " "
                                        lookup( strings, "children")
                                        if ( ne( spspouse, null)) {
                                                " " lookup( strings, "with") " "
                                                call print_person(spspouse, 0)
                                        } else {
                                                " " lookup( strings, "with") " "
                                                lookup( strings, "unknownspouse")
                                        }
                                        if( eq(spousefamilies, 2)) {
                                                output_leftin()
                                                children(spfamily, child, k) {
                                                        "\t"
                                                        call print_person(child, 0)
                                                }
                                                output_leftout()
                                        }
                                }
                        }
                }

                call write_notes(spouse)

                output_leftin()
                children(family, child, j) {
                        "\t" number "." d(add(j, childnumber)) nl() "\t"
                        call print_person(child, 1)
                }
                output_leftout()
                set(childnumber, add(childnumber, j))
        }

        output_endpara()

        set(childnumber, 0)

        families(indi, family, spouse, i) {
                if (ne(1, generation_count)) {
                        if ( gt(generation_count, 1)) {
                                decr(generation_count)
                        }
                        children(family, child, j) {
                                call descendants (
                                        child,
                                        strconcat(
                                                number, ".",
                                                d(add(j, childnumber))
                                        ),
                                        generation_count, spousefamilies)
                        }
                        set(childnumber, add(childnumber, j))
                }
        }

}



proc write_notes(indi) {
        set(done_header, 0)
        fornodes(inode(indi), node) {
                if (eq(0,strcmp("FILE", tag(node)))) {
                        if ( eq(done_header, 0) ) {
                                lookup( strings, "Notesfor") " "
                                name(indi) ":" output_linebreak()
                                incr(done_header)
                        }
                        copyfile(value(node))
                } elsif (eq(0,strcmp("NOTE", tag(node)))) {
                        if ( eq(done_header, 0) ) {
                                lookup( strings, "Notesfor") " "
                                name(indi) ":" output_linebreak()
                                incr(done_header)
                        }
                        value(node)
                        fornodes(node, subnode) {
                                if (eq(0,strcmp("CONT", tag(subnode)))) {
                                        nl() value(subnode)
                                }
                        }
                        output_linebreak()
                }
        }
}


proc print_person (indi, bold) {
        if(bold) {
                output_bold( name(indi))
        } else {
                name(indi)
        }
        if (e, stddate(birth(indi))) {
                ", " lookup( strings, "Born") " " e
        }
        if(e, stddate(death(indi))) {
                ", " lookup( strings, "Dead") " " e
        }
        "." output_linebreak()
}


func output_header1 (string) {
        if ( eq( outputtype, 1)) {
                return(concat( ".(b C", nl(), ".ps 16", nl(), "\\fB",
                        split(string), "\\fP", nl(),
                        ".ps 8", nl(), ".)b", nl()))
        }
        if ( eq( outputtype, 2)) {
                return(concat( "<H1>", string, "</H1>"))
        }
        return(string)
}

func output_header2 (string) {
        if ( eq( outputtype, 1)) {
                return(concat( ".(b C", nl(), ".ps 12", nl(), "\\fB",
                        string, "\\fP", nl(),
                        ".ps 8", nl(), ".)b", nl()))
        }
        if ( eq( outputtype, 2)) {
                return(concat( "<H2>", string, "</H2>"))
        }
        return(string)
}


func output_init () {
        if ( eq( outputtype, 1)) {
                return(concat( ".po 0.8i", nl(), ".ll 6.8i", nl(),
                        ".pl +1.5i", nl(), ".nf", nl(), ".ps 8", nl()))
        }
        if ( eq( outputtype, 2)) {
                return(concat("<HTML><HEAD><TITLE>Descendant chart</TITLE></HEAD><BODY>", nl() ))
        }
}


func output_terminate () {
        if ( eq( outputtype, 2)) {
                return("</BODY></HTML>")
        }
}


func output_linebreak () {
        if ( eq( outputtype, 1)) {
                return(nl())
        }
        if ( eq( outputtype, 2)) {
                return("<BR>")
        }
        return(nl())
}


func output_startpara () {
        if ( eq( outputtype, 2)) {
                return("<P>")
        }
}


func output_endpara () {
        if ( eq( outputtype, 2)) {
                return("</P>")
        }
        return(concat( output_linebreak(), output_linebreak()))
}

func output_bold (string) {
        if ( eq( outputtype, 1)) {
                return(concat( "\\fB", string, "\\fP"))
        }
        if ( eq( outputtype, 2)) {
                return(concat( "<B>", string, "</B>"))
        }
        return(string)
}


func output_leftin () {
        if ( eq( outputtype, 2)) {
                return(concat( "<TABLE WIDTH=100%><TR><TD WIDTH=5%></TD><TD>"))
        }
}


func output_leftout () {
        if ( eq( outputtype, 2)) {
                return(concat( "</TD></TR></TABLE>"))
        }
}


func split(string) {
        set(i, 1)
        set(tmpstr, "")
        while( ne( i, strlen(string))) {
                if ( nestr( substring( string, i, i), " ")) {
                        set(tmpstr, concat(tmpstr, substring( string, i, i)))
                } else {
                        set(tmpstr, concat(tmpstr, "\\0"))
                }
                incr(i)
        }
        return (tmpstr)
}


/* Sample output:


Successors of N.N. Helgesdtr. 又E with 2 generations descendants

Date: 14 Jun 1995

1
N.N. Helgesdtr. 又E.
Relationship with Tjeran Hallvardson (Halldors.) VASSHUS, Born        1610.
Notes for Tjeran Hallvardson (Halldors.) VASSHUS:
Er nevnt som leilending i 1635 sammen med Helge 山e som sannsynligvis var
hans svigerfar.
Tjeran og broren Rasmus stevnet stefaren Laurits Asserson for odelsgods i
Kluge, Gjesdal som deres mor eide.
        1.1
        Lars Tjeranson 又E, Born        1643, Dead  9 Jun 1702.
        1.2
        Marite Tjerandsdtr. 又E, Dead        1691.
        1.3
        Hallvard TJERANSON, Born        1651.
        1.4
        Helge TJERANSON, Born        1654.


1.1
Lars Tjeranson 又E, Born        1643, Dead  9 Jun 1702.
Notes for Lars Tjeranson 又E:
Gift I med Kirsti Olsdtr. Malmeim (f.1665 d.12.04.1695).
II med Johanna Gunnarsdtr. Sveinsvoll (d.1741).
Lars hadde v喣t soldat i 10 繢. 2 barn kjent.


1.2
Marite Tjerandsdtr. 又E, Dead        1691.
Relationship with Ola Olson KJOSAVIK, Born        1623, Dead  9     1702.
Ola Olson KJOSAVIK had 7 children with KariI Pedersdtr., Born 16     , Dead        1674.
Ola Olson KJOSAVIK had 1 children with KariII Torkellsdtr. ALSNES, Born        1661, Dead 30 Mar 1705.
Notes for Ola Olson KJOSAVIK:
7 barn av f酺ste ekteskap, 3 av andre og 1 av tredje ekteskap.
        1.2.1
        Berit Olsdtr. KJOSAVIK, Born        1674, Dead        1746.
        1.2.2
        Kristoffer Olson KJOSAVIK, Born        1677.
        1.2.3
        Ola O. KJOSAVIK, Born        1681, Dead 23     1695.

.
.
.

*/
