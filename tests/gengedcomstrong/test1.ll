/*
@progname test1.ll
@author Perry
@description Test gengedcom on sparse db
*/
proc main ()
{
        getindi(indi)
        indiset(ilist)
        addtoset(ilist, indi, 0)
	set (curlen,-3)
        set (silly, lengthset(ilist))
	while (ne(curlen, lengthset(ilist)))
	{
	    set(curlen, lengthset(ilist))
	    set(ilist, union(ilist, ancestorset(ilist)))
	    set(ilist, union(ilist, descendentset(ilist)))
	    set(ilist, union(ilist, spouseset(ilist)))
	}
	gengedcomstrong(ilist)
}

