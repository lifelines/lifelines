/*
* @progname       src.ll
* @version        2.0
* @author         McBride
* @category       sample
* @output         text
* @description
Here is a report program to list SOURces. The REFN and TITL
values are shown. Other tags can be added by duplicating
the lines that containing "myrefn" or "REFN" and replacing them
with the tag you want.

To process tags that have CONTinuation lines, or tags with no
values you need something more complicated.

"P. McBride" <pbmcbride@RCN.COM>
*/

proc main ()
{
   forsour(snode, i) {
     set(mytitle, "")
     set(myrefn, "")
     fornodes(root(snode), anode) {
       if(eqstr(tag(anode),"TITL")) { set(mytitle, save(value(anode))) }
       elsif(eqstr(tag(anode),"REFN")) {
         set(myrefn, save(value(anode)))
       }
     }
     myrefn  "\t"  key(snode)  "\t"  mytitle  nl()
   }
}
