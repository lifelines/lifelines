/*
 * @progname       ll2visio.ll
 * @version        1 of 1999-04-02
 * @author         Rafal T. Prinke (rafalp@amu.edu.pl)
 * @category       
 * @output         VISIO 5 diagram
 * @description
 *
 *  This program generates a text file with male line descendants
 *  which can be imported by VISIO 5 and converted to a diagram.
 *
 *  The main procedure is based on Tom Wetmore's _pdesc2_ report.
 *  The procedures "longvitals" and "spousevitals" were originally
 *  based on the same in Tom Wetmore's _register1_ report but
 *  have been changed beyond recognition.
 *
 *  The included procedures "upl2.ll" were written by Paul McBride.
 *
 *  The text file describing a diagram to be constructed in VISIO 5
 *  consists of records (lines) and data fields separated with a comma (.csv)
 *  or a tab (.txt). It can also use an arbitrary delimiter but this report
 *  uses a comma so the output file should be given a .CSV extension.
 *  The CR (hard return) character is not treated as end of record if
 *  enclosed in quotation marks. Comment lines are escaped with a semicolon.
 *  All measurements are in inches.
 *
 *  Here is a quick reference to record types. For one field records
 *  I have added possible values [with explanations in brackets].
 *
 *     <record type,> <field,field,field...>
 *
 *     Master,MasterID,StencilName
 *
 *     PlacementStyle,Style     (0|1|2) [Radial|TopToBottom|LeftToRight]
 *     RoutingStyle,Style       (1|2|3|4|5|) [RightAngle||||Flowchart]
 *
 *     NodeToLineClearance,Horizontal,Vertical
 *     LineToLineClearance,Horizontal,Vertical
 *
 *     Gridding,UseGrid                 (0|1)   [no|yes]
 *     BlockSize,Width,Length
 *     AvenueSize,Width,Height
 *
 *     Template,FileName
 *     Property,Master,RowName,Label,Prompt,Type,Format,Value,Hidden,Ask
 *
 *     Shape,ID,Master,Text,X,Y,Width,Height,Property
 *     Link,ID,Master,Text,From,To
 *
 */

include("upl2.ll")

global(persons)
global(mens)
global(ind1)
global(first)
global(maxgen)
global(upl_tag_list)
global(upl_before_list)
global(upl_after_list)
global(upl_level_list)
global(upl_process_list)
global(upl_out_type)    /* 0 = both, 1 = screen, 2 = file */

proc main () {

        list(persons)
        table(mens)
        call upl_build()
        set(upl_out_type, 2)  /* output to file */

        while(not(indi)) {
             getindi(indi)
        }
        getintmsg(maxgen,"How many generations? ")

        call head(indi)
        call mensae(indi)
        call pout(0, indi)
        set(first,key(indi))

        while(not(empty(persons))) {
             set(p,pop(persons))
             call longvitals(p)
        }
}

/* This procedure produces a list of individuals which is then
 * popped for final output. The reason is that VISIO 5 seems to
 * lay out the shapes from top to bottom and from right to left,
 * so the older children would be on the right and younger on the left,
 * while in traditional genealogical tables the older children are
 * on the left.
 */

proc pout(gen, indi)
{
        push(persons, indi)
        set(next, add(1, gen))
        families(indi,fam,sp,num) {
                if (lt(next,maxgen))  {

/*
    remove the condition below to include all descendants
    (this may not work well yet - father/mother functions need modification)
*/

                        if(eqstr(sex(indi),"M")) {
                                children(fam, child, no) {
                                        call pout(next, child)
                                }
                        }
                }
        }
}

/*
 * The following is the procedure outputting the header of the .csv
 * file for import to VISIO 5. The records field and allowed values
 * can be altered to give intended results.
 */

proc head(u)
{

      "Master,AUTO,Auto-height Box,Basic Flowchart Shapes.VSS\n"
      "Master,CON,Bottom to top variable,Connectors and Callouts.VSS\n"
      "PlacementStyle,1\n"
      "RoutingStyle,1\n"
      ";RoutingStyle,7\n"
      "AvenueSize,0.15,0.15\n"
      "LineToLineClearance,0.1,0.1\n"
      "NodeToLineClearance,0.1,0.1\n"
      "BlockSize,0.2,0.2\n"
      "Gridding,0\n"
      ";Property,AUTO,,,,\n"
      ";Template,\n"
      "\n"
}

/*
 * The remaining procedures define what information goes into
 * one person box. This is rather specific to my files and uses
 * Polish terms so should be adapted to one's needs.
 */


proc longvitals(i)
{
     set(ikey,key(i))
     set(ind1,i)

/*
 * The following line is the beginning of an individual shape (or box)
 * definition, and then the text inside it is constructed.
 * NOTE: if there are quotation marks in the data, there may be problems.
 */

     "Shape," ikey ",AUTO," "\""

/* The following fragment is intended to overcome the "unknown mother"
 * problem when numbering marriages. It is assumed that a dummy FAM record
 * (a) has no WIFE, (b) has the word "ANY" as the wife's name, or (c) has two
 * or more numbers separated with the bar character as her name (e.g. "2|3")
 * when only some wives are possible mothers.
 */


if(nestr(ikey,first)) {
     if (ne(nfamilies(father(i)),1)) {
          if (eqstr(name(mother),"ANY")) {
               "(n) "
          }
          elsif (index(name(mother(i)),"|",1)) {
               "(" name(mother) ") "
          }
          else {
               set(wnm,0)
               families(father(i),a,b,c) {
                       if (wife(parents(i))) {
                            set(wnm,add(wnm,1))
                       }
                       if (eqstr(key(a),key(parents(i)))) {
                            if (wife(parents(i))) {
                                    "(" d(wnm) ") " }
                            else { "(n) " }
                       }
               }
          }
     }
}

/* biographical data report on person is called */

call upl_report(i)

     if (eq(1,nspouses(i))) {
            spouses(i,s,f,n) {
                "; x "
                call spousevitals(s,f)
            }
     } else {
            spouses(i,s,f,n) {
                "; x (" d(n) ") "
                call spousevitals(s,f)
            }
     }

     "\"\n"

/*
 * The following line is the definition of a link between two boxes.
 * The condition excludes up-link from the first person.
 */

if(nestr(ikey,first)) { "Link,,CON,," key(father(i)) "," key(i) "\n" }

}

proc spousevitals (spouse,fam)
{
     set(e,marriage(fam))
     if (and(e,long(e))) { mylong(e) ", " }
     roz(fam)

/* biographical data report on spouse is called */

call upl_report(spouse)

     set(dad,father(spouse))
     set(mom,mother(spouse))
     if (or(dad,mom)) {
            ", "
            if (male(spouse))      { "s. " }
            elsif (female(spouse)) { "dau. " }
            else                   { "ch. " }
     }

     if (dad)          { givens(dad)    /*  "==a"  */
            fornodes(inode(dad), ok) {
                  if (eqstr(tag(ok),"OCCU")) {
                       ", " value(ok)
                  }
            }
     }

/* Other marriages of the spouse. */

     set(srd,0)
     if (gt(nspouses(spouse),1)) { " ["
            spouses(spouse,ind2,fm,nsp) {
                  if (ne(ind2,ind1)) {
                        if (srd) { "; " }
                        " x (" d(nsp) ") "
                        set (es,marriage(fm))
                        if (and(es,long(es))) { mylong(es) " " }
                        roz(fm)
                        name(ind2,0)
                        set(srd,1)
                  }
            }
     "]"
     }
}

func mylong(ev)
{
     list(datum)
     extracttokens(date(ev),datum,n," ")
     forlist(datum,q,n) {
            if (lookup(mens,q)) { lookup(mens,q) }
            else { q }
     }
     if(place(ev)) {
            " (" place(ev) ")"
     }
}


func roz(fx)
{
     fornodes(root(fx), ok) {
            if (eqstr(tag(ok),"DIV")) {
                 ", div."
                 fornodes(ok, dt) {
                       if (eqstr(tag(dt),"DATE")) {
                            " "  mylong(dt)
                       }
                 }
            }
     }
}


/* Table of date tokens follows. */

proc mensae(w)
{
insert(mens,"JAN",".I.")
insert(mens,"FEB",".II.")
insert(mens,"MAR",".III.")
insert(mens,"APR",".IV.")
insert(mens,"MAY",".V.")
insert(mens,"JUN",".VI.")
insert(mens,"JUL",".VII.")
insert(mens,"AUG",".VIII.")
insert(mens,"SEP",".IX.")
insert(mens,"OCT",".X.")
insert(mens,"NOV",".XI.")
insert(mens,"DEC",".XII.")
insert(mens,"BEF","a")
insert(mens,"AFT","p")
insert(mens,"ABT","c")
insert(mens,"CIR","c")
insert(mens,"BET","")
insert(mens,"AND","/")
}

