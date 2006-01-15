/*
 * @progname       register1-dot
 * @version        1.0 (14-May-2004)
 * @author         Marc Nozell (marc@nozell.com)
 * @category       
 * @output         dot format
 * @description    Use graphviz's dot to product multipage 
 *	           directed graphs of descendants.
 *
 *                 (dot is available from www.graphviz.com)
 *                 $ dot -Tps -ofamily.ps family.dot
 */


proc main ()
{
	getindi(indi)
	list(ilist)
 	list(glist)
 	table(stab)
 	indiset(idex)
 	enqueue(ilist,indi)
 	enqueue(glist,1)
 	set(curgen,0)  set(out,1)  set(in,2)

	"digraph \"nozell family\" {" nl()
	"ranksep=.75; "nl()
	"page = \"8,5\";"nl()
/*
	"size = \"10.5,8\";"nl()
*/
/*  If you want landscape mode.
	"rotate = 90;"nl()
*/
	"\"" key(indi) "\" [label=\"" name(indi)  "\"];" nl()
 	while (indi,dequeue(ilist)) {
 		print("OUT: ") print(d(out))
 		print("# ") print(name(indi)) print(nl())
 		set(thisgen,dequeue(glist))

 		insert(stab,save(key(indi)),out)

 		addtoset(idex,indi,0)
 		set(out,add(out,1))

		families(indi,fam,spouse,nfam) {

			if (spouse) { set(sname, save(name(spouse))) set (spousekey, save(key(spouse))) }
			else        { set(sname, "_____") set (spousekey, "IUNKNOWN") }

			if (eq(0,nchildren(fam))) {
				nl()
			}
			elsif (and(spouse,lookup(stab,key(spouse)))) {
				nl()
			}
			else {
				"#Children of " name(indi) " and " sname":" nl()

				if (male(spouse)) { set(spousesexstyle, " ,shape=box,color=slateblue1 ") }
				elsif (female(spouse)) { set(spousesexstyle, " ,shape=diamond,color=pink ")}
				else            { set(spousesexstyle, " ,shape=hexagon,color=yellow ") }


				/* define the spouse... */
				"\"" spousekey "\" [label=\"" sname "\"" spousesexstyle "];" nl()
	
				/* Show the marriage by a different
				   arrow type, a higher weight and set
				   them at the same level */

				"\"" key(indi) "\" -> \"" spousekey "\" [weight=10, arrowhead=dot, arrowtail=dot];" nl()
				"\"" spousekey "\" -> \"" key(indi) "\" [weight=10, arrowhead=dot, arrowtail=dot];" nl()

				"{ rank = same; " key(indi) "; " spousekey "; }" nl()

				children(fam,child,nchl) {
					set(haschild,0)
					families(child,cfam,cspou,ncf) {
						if (ne(0,nchildren(cfam))) { set(haschild,1) }
					}

					if (male(child)) { set(sexstyle, " ,shape=box,color=slateblue1 ") }
					elsif (female(child)) { set(sexstyle, " ,shape=diamond,color=pink ")}
					else            { set(sexstyle, " ,shape=hexagon,color=yellow ") }


					/* define the child and their relationship to the parents */

					"# KEYDEF \"" key(child) "\" [label=\"" name(child)  "\"" sexstyle "];" nl()
					"\"" key(child) "\" [label=\"" name(child)  "\" sexstyle];" nl()
					"\"" key(indi) "\""  " -> " "\"" key(child) "\";" nl()
					"\"" spousekey "\"" " -> " "\"" key(child) "\";" nl()

					if (haschild) {
						print("IN:  ") print(d(in))
						print(" ") print(name(child)) print(nl())
						enqueue(ilist,child)
						enqueue(glist,add(1,curgen))
					}
					else {
 						addtoset(idex,child,0)
					}


				}
			}
		}
	}
	"}"
}
