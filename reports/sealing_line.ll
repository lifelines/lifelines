/*
 * @progname       sealing_line.ll
 * @version        none
 * @author         Tom Wetmore
 * @category       
 * @output         function, and driver writing Text
 * @description

function sealing_line().
You pass it a person, and it returns the person's
sealing line (if there is one) or nothing (if there isn't).  The main
program is only used here to test it.  You would call "sealing_line" in the
place you need it in your own program.  Yes, it is a little complicated,
but that's why we have modules.  Write it, stick it in some library
somewhere, and just call it when you need the info.

Tom Wetmore
 */

proc main ()
{
        getindi(i)
        if (not(i)) { return() }
        if (l, sealing_line(i)) {
                print("yes\n")
                print(tag(l), " ", value(l), "\n")
        } else {
                print("no")
        }
}

func sealing_line (i)
{
        set(f, parents(i))
        if (not(f)) { return(0) }
        set(ir, inode(i)) set(fr, fnode(f))
        fornodes(fr, s) {
                if(and(eqstr("CHIL", tag(s)), eqstr(xref(ir), value(s)))) {
                        fornodes(s, ss) {
                                if(eqstr("SLGC", tag(ss))) { return(ss) }
                        }
                        return(0)
                }
        }
        return (0)
}
