/*
 * @progname       gedlod.ll
 * @version        2000-02-15
 * @author         Paul B. McBride (pbmcbride@rcn.com)
 * @category       
 * @output         GEDCOM
 * @description
 *
 * Generate a GEDCOM file of a person's descent from an ancestor.
 * The GEDCOM file will contain the following:
 * all descendents of the ancestor who are ancestors of descendant, 
 * as well as the ancestor and descendant themselves.
 *
 * 15 Feb 2000 Paul B. McBride (pbmcbride@rcn.com)
 */

include("prompt.li")
include("ged_write.li")

proc main ()
{
      indiset(iset)
      indiset(dset)
      indiset(tset)
      indiset(uset)
      indiset(aset)
      indiset(gset)

      getindimsg(descendant,"Identify the descendant")
      if(descendant) {
        set(i, 1)
        while(1) {
          getindimsg(ancestor, concat("Identify ", ord(i), " ancestor"))
          if(ancestor) {
	    set(i, add(i,1))
	    addtoset(iset, ancestor, 0)
      	  }
	  else {
	    break()
	  }
        }
      }
      if(and(gt(lengthset(iset), 0),ne(descendant,0))) {
      	  set(addspouses, askyn("Include spouses"))
      	  set(addchildren, askyn("Include children"))

	  /* find all the people of interest */
	  print("Finding Ancestors... ")
	  addtoset(dset, descendant, 0)
	  set(tset, ancestorset(dset))
	  print(d(lengthset(tset)), nl())

	  print("Finding Descendants... ")
	  set(uset, descendantset(iset))
	  print(d(lengthset(uset)), nl())

	  set(aset, intersect(tset, uset))
	  set(aset, union(aset, iset))	/* add in ancestors */
	  addtoset(aset, descendant, 0)	/* add in descendant */

	  if(addspouses) {
	    set(tset, spouseset(aset))	/* add their spouses */
            set(aset, union(aset, tset))
	  }

	  if(addchildren) {
	    set(tset, childset(aset))	/* find everyone's children */
            set(aset, union(aset, tset))
	  }
	  call ged_write(aset)		/* write out GEDCOM file */
      }
}
