/*
 * @progname       relink.ll
 * @version        1995-06
 * @author         J.F. Chandler
 * @category       
 * @output         GEDCOM
 * @description

LifeLines program to reconstruct pointers from persons to families when
these pointers are missing, but can be deduced from the corresponding
pointers from families to persons.  Do this only for persons with no
pointers to families at all.  Similarly, reconstruct pointers from
families to persons where necessary.

The output is a GEDCOM file which includes only the individual and family
records from the database.  Other record types must be recovered separately
because there is no iterator in the language for those record types.

relink - J.F. Chandler - 1995 Jun
*/

proc main() {
"0 HEAD\n1 SOUR RELINK\n1 DEST ANY\n"
forindi(i,n) {
        traverse(inode(i),node,level) {
                d(level) " "
                if(eq(level,0)) { "@" key(i) "@ " }
                tag(node)
                if(v,value(node)) { " " v }
                nl()
        }
        if(not(or(nfamilies(i),parents(i)))) {
                set(indk,save(key(i)))
                forfam(f,k) {
                        if(or(eq(0,strcmp(indk,key(wife(f)))),
                                eq(0,strcmp(indk,key(husband(f)))))) {
                                "1 FAMS @" key(f) "@\n"
                        } elsif(nchildren(f)) {
                                children(f,child,l) {
                                        if(eq(0,strcmp(indk,key(child)))) {
                                                "1 FAMC @" key(f) "@\n"
                                                break()
                                        }
                                }
                        }
                }
        }
}
forfam(f,k) {
        traverse(fnode(f),node,level) {
                d(level) " "
                if(eq(level,0)) { "@" key(f) "@ " }
                tag(node)
                if(v,value(node)) { " " v }
                nl()
        }
        if(not(or(husband(f),wife(f),nchildren(f)))) {
                set(famk,save(key(f)))
                forindi(i,n) {
                        families(i,fam,spo,l) {
                                if(eq(0,strcmp(famk,key(fam)))) {
                                        if(male(i)) {"1 HUSB @" key(i) "@\n"}
                                        else {"1 WIFE @" key(i) "@\n"}
                                        break()
                                }
                        }
                        if(eq(0,strcmp(famk,key(parents(i))))) {
                                "1 CHIL @" key(i) "@\n"
                        }
                }
        }
}
"0 TRLR\n"
}
