/*
 * @progname       d-rtf.ll
 * @version        1.1 of 2000-06-11
 * @author         Paul Buckley
 * @category       
 * @output         RTF
 * @description
 *	
 *  This report will produce a document in Rich Text Format modeled after some
 *  typewritten and typeset Henry format genealogies I've seen.
 *  All descendants of a selected person, their spouses and their spouses
 *  parents, event dates, and NOTEs are included. I generally reserve TEXT
 *  items for comments I would prefer not to export.
 *
 *  This version requires shorten.li, a library with a lookup table
 *  to abbreviate the elements of the long placenames I tend to use
 *  (town, county, state, country).
 *  If you remove the calls to "shorten()" and just use the output of
 *  long() it should work fine without the library.
 *
 * Added support for printing reference numbers after data.
 *  Set "refs" to 0 to omit references.
 *
 * Written by: Paul Buckley, 11 Jun 2000, contact via LifeLines mail list
 * (with a lot of help from the archives)
 */

global(rtfH)		/* string, RTF header and font info */
global(rulI)            /* string, index person ruler */
global(rulS)            /* string, spouse ruler */
global(rulC)            /* string, children list ruler*/
global(rulN)            /* string, notes ruler (same as spouse)*/
global(font_name)       /* string, name of font */
global(font_size)	/* int, font size in RTF values (2 x points) */
global(big_font)	/* int, ~1.3 times font_size */
global(sml_font)	/* int, ~2/3 times font_size */

include("shorten.li")

proc main ()
{
    set(refs,1)		/*set this to have reference numbers printed*/
    indiset(sibs)
    indiset(nextgen)
    table(abbvtab)
    call setupabbvtab()
    
    getindi(p)
    newfile(concat(database(), ".", key(p), "-d.rtf"),0)
    
    call GetUserOptions()
    set(genN,1)
    set(Icnt,1)
    addtoset(sibs,p,Icnt)
    set(l,1)
/*   set(mark,concat("\\fs", d(div(font_size,2)), "\\up4 +\\up0\\fs", d(font_size))) */
    set(mark,"*")
    rtfH
    "\\pard\\fs"
    d(big_font)
    "The Descendants of " upper(name(p)) "."
    "\\ql\\ulnone\\\n\\fs"
    d(font_size)
    "\\\n"
    
    while(l) {
        "\\pard\\qc\\i1\\fs"
        d(big_font) " " capitalize(ord(genN))" Generation \\i0\\ql\\fs"
        d(font_size)
        "\\\n\\\n"
        forindiset(sibs,person,var,i) {
            rulI
            upper(alpha(genN)) "-" d(var) "\t" name(person)
            if(or(date(birth(person)),place(birth(person)))) {
                            ", b. " shorten(long(birth(person)))
                            if(refs) {call refRTF(birth(person)) }
            }
            if(or(date(baptism(person)),place(baptism(person)))) {
                            ", bt. " shorten(long(baptism(person)))
                            if(ifwitn(root(person))) {". "}
                            if(refs) {call refRTF(baptism(person)) }
            }
            if(or(place(death(person)),date(death(person)))) {
                            ", d. " shorten(long(death(person)))
                            if(refs) {call refRTF(death(person)) }
            }
            ".\\\n"
            traverse(root(person),node,cnt) {
            if (not(strcmp(tag(node),"NOTE"))) {
                            rulN value(node) call refRTF(node) " \n"
                            }
            }
            families(person,family,spouse,j) {
                rulS
                if(spouse) {
                    givens(person) " married " name(spouse)
                    if(date(marriage(family))) {
                        " " shorten(long(marriage(family)))
                    }
                    ". "
    /*if(refs) {call refRTF(marriage(family))}*/
                    "\n"
                    if(ifwitn(root(family))) {". "}
                    set(comma,0)
                    if(or(place(birth(spouse)),date(birth(spouse)))) {
                        set(comma,1)
                        pn(spouse,0) " was born " shorten(long(birth(spouse)))
                    }elsif(parents(spouse)) {
                        pn(spouse,0) " was born" shorten(long(birth(spouse)))
                    }
                    if(parents(spouse)) {
                        " to "
                        if(father(spouse)) {
                            set(comma,1)
                            name(father(spouse))
                            if(mother(spouse)) {
                                set(comma,1)
                                " and " name(mother(spouse))
                            }
                        }elsif(mother(spouse)) {
                            set(comma,1)
                            name(mother(spouse))
                        }
                    }
                    if(or(date(death(spouse)),place(death(spouse)))) {
                        if(comma) {", "}
                        else { pn(spouse,0) " "}
                        "died " shorten(long(death(spouse)))
                    } 
                    if(comma) {". "}
                    traverse(root(spouse),node,cnt) {
                    if (not(strcmp(tag(node),"NOTE"))) {
                        "\\\n" rulN value(node) call refRTF(node)
                    }
                }
                "\\\n"
            } 
            else {"Spouse unknown.\\\n"}
            children(family,kid,k) {
            if(kid) {
                set(genNx,add(genN,1))
                rulC
                if(nfamilies(kid)) {
                    addtoset(nextgen,kid,Icnt)
                    upper(alpha(genNx)) "-" d(Icnt) mark "\t" name(kid)
                    if(date(birth(kid))) {
                    ", b. " shorten(date(birth(kid)))
                }
                if(date(baptism(kid))) {
                    ", bt. " shorten(date(baptism(kid)))
                }
                if(date(death(kid))) {
                    ", d. " shorten(date(death(kid)))
                }
                ".\\\n"
                }else {
                    upper(alpha(genNx)) "-" d(Icnt) "\t" name(kid)
                    if(or(date(birth(kid)),place(birth(kid)))) {
                        ", b. " shorten(long(birth(kid)))
                        if(refs) {call refRTF(birth(kid)) }
                    }
                    if(or(date(baptism(kid)),place(baptism(kid)))) {
                        ", bt. " shorten(long(baptism(kid))) 
                        if(ifwitn(root(kid))) {""}
                        if(refs) {call refRTF(baptism(kid)) }
                    }
                    if(or(date(death(kid)),place(death(kid)))) {
                        ", d. " shorten(long(death(kid)))
                        if(refs) {call refRTF(death(kid)) }
                    }
                    ".\\\n"
                    traverse(root(kid),node,cnt) {
                        if (not(strcmp(tag(node),"NOTE"))) {
                            "\t" value(node) call refRTF(node) "\\\n"
                        }
                    }
                }
                set(Icnt,add(Icnt,1))
            }
        }
            traverse(root(family),node,cnt) {
            if (not(strcmp(tag(node),"NOTE"))) {
            rulN value(node) call refRTF(node) " "
            }
            }
            }
            "\\\n"
        }
        set(l, lengthset(nextgen))
        indiset(sibs)
        set(sibs,nextgen)
        
        indiset(nextgen)
        set(genN,add(genN,1))
        set(Icnt,1)
    }
    rulI
    "\\\n Generated "
	date(gettoday())
	" from "
	concat(database(),".gedcom")
/*	" by YOU " */
	" using LifeLines genealogy software"
	". \\\n } "
}

proc GetUserOptions ()
{
/*
**  QUESTION: What font should be used?
**
**  Because it is such a pain to enter a font name, and a spelling mistake
**  will get you an ugly default font, this should be set to a default.  I
**  suggest one of: Times-Roman, NewCenturySchlbk-Roman, or ZapfChancery.
**  This is a modification of code from the original psanc uing NeXT fonts
**  -PB
**
*/
    
    if (0)  {
        list(options)
        setel(options, 1, "Roman")
        setel(options, 2, "Italic")
        set(ff, menuchoose(options, "Select font face: "))
        if (eq(1,ff)) {
            list(options)
            setel(options,1,"Times")
            setel(options,2,"New Century Schoolbook")
            setel(options,3,"Garamond")
            set(mc, menuchoose(options, "Select font family: "))
            if (eq(3,mc)) {
                set (font_name, "AGaramond-Regular")
            } elsif (eq(2,mc)) {
                set (font_name, "NewCenturySchlbk-Roman")
            } else {
                set (font_name, "Times-Roman")
            }
        }else {
            setel(options,1,"Times")
            setel(options,2,"New Century Schoolbook")
            setel(options,3,"Garamond")
            setel(options,4,"ZapfChancery")
            set(mc, menuchoose(options, "Select font: "))
            if (eq(1,mc)) {
                set (font_name, "Times-Italic")
            } elsif (eq(2,mc)) {
                set (font_name, "NewCenturySchlbk-Italic")
            } elsif (eq(3,mc)) {
                set (font_name, "AGaramond-Italic")
            } elsif (eq(4,mc)) {
                set (font_name, "ZapfChancery-MediumItalic")
            }
        }
    } else { set (font_name, "Times-Roman") }

/*
**  QUESTION: What font size should be used?
**
**  I set this to 20 by default, which is about 10pt.
**  A title font is generated about 1/3 bigger (dividing integers here)
**  -PB
**
*/
    if(0) {
        getintmsg (font_size, "Enter the font size in points.")
        set(font_size, mul(font_size,2))
    } else {
        set(font_size, 20)
    }
    set(big_font,add(font_size,div(font_size,3)))
    set(sml_font,sub(font_size,div(font_size,3)))
    
/* 
* Set RTF defaults. Modifed for Mac OS X TextEdit.app.
* Don't forget the terminal space character.
*/

    set(rtfH, concat("{\\rtf1\\ansicpg1000{\\fonttbl\\f0\\fnil ", concat(font_name, ";}")))			
    set(rtfH, concat(rtfH, "\n\\margl720\\margr720\\margt720\\margb720\\viewkind1"))
    set(rtfH, concat(rtfH, "\n\\f0\\b0\\i0\\ulnone\\ql\\fs"))
    set(rtfH, concat(rtfH, d(font_size)))
    set(rtfH, concat(rtfH, "\\fi0\\li0"))
    set(rulI, "\\pard\\tx720\\fi-720\\li720 ")
    set(rulS, "\\pard\\fi-180\\li1080 ")
    set(rulC, "\\pard\\tx1800\\fi-720\\li1800 ")
    set(rulN, "\\pard\\fi-180\\li1080 ")
}
    
func ifwitn (thisnode)
{
    set(needand,0)
    set(amdone,0)
    traverse(thisnode,x,y) {
        if (not(strcmp(tag(x),"WITN"))) {
            if(needand) {" and"}
            " " value(x)
            set(needand,1)
            set(amdone,1)
        } else {set(needand,0)}
    }
    if(amdone) {" witnessed"}
    else {""}
    return(amdone)
}

proc refRTF (i) {
    fornodes(i,node) {
        if (not(strcmp(tag(node),"SOUR"))) {
            set(text,strsave(value(node)))
            if (index(text,"@",2)) {
                set(text,substring(value(node),3,sub(strlen(text),1)))
            }
            "\\fs" d(sml_font)
            "\\up" d(div(font_size,4))
            "(" text ")"
            "\\fs" d(font_size) "\\up0"
        }
    }
}
