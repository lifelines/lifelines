/*
 * @progname       src-rtf.ll
 * @version        none
 * @author         Paul Buckely <cu-chulain@home.net>
 * @category       
 * @output         RTF
 * @description
 *
 * List sources in RTF, a modification of src.ll .
 *
 */

/*include ("util.ll") for using nodetag(), which is much slower*/

proc main ()
{
    set(i, 1)
    set(errcnt, 1)
    newfile(strconcat(database(),".src.rtf"),0)
    dayformat(0)
    monthformat(4)
    dateformat(8) /* this is used so I can sort by numeric date*/
    "{\\rtf1\\ansicpg1000{\\fonttbl\\f0\\fnil Times-Roman;}\n"
    "\\margl720\\margr720\\margt720\\margb720\\viewkind1\n"
    "\\pard\\tx560\\tx2700\\tx4140\\tx5020\\f0\\b0\\i0\\fs20\\fi-5020\\li5020\\fc0\\cf0\ "
    "\\ul Ref#\\ulnone \t\n"
    "\\ul Key\\ulnone \t\n"
    "\\ul Entered\\ulnone \t\n"
    "\\ul Order\\ulnone \t\n"
    "\\ul Title\\ulnone \\\n"
    while(le(errcnt,100)) {
    set(skey, concat("@S",d(i),"@"))
    if(snode, dereference(skey)) {
	set(mytitle, "")
	set(myrefn, "")
	set(mydate, "")
	set(order, "")
	fornodes(snode, anode) {
	    if(eqstr(tag(anode),"TITL")) {
		set(mytitle, save(value(anode))) }
		elsif(eqstr(tag(anode),"REFN")) {
		set(myrefn, save(value(anode)))  }
    /*set(myrefn, nodetag(snode, "REFN")) this works but it's much slower*/
	    set(mydate, stddate(snode))
	    extractdate(snode, dy, mo, yr)
	    set(order, add(mul(100,mo),add(dy,mul(2,yr))))
	}
	d(i) "\t" myrefn "\t" mydate "\t" d(order) "\t" mytitle"\\\n"
	}
	else {
	    set(errcnt, add(errcnt,1))
	}
	set(i, add(i,1))
    }
    "\\\n\\\n"
    "References generated "
	date(gettoday())
	" from "
	concat(database(),".gedcom")
    " using LifeLines genealogy software.\\\n"
    nl() "}"
}
