/*
@programe Analyze.ll
@author Stephen Dum
@description This test is attempting to call as many different iterators
             as it can for indi's, fam's, and children.
*/
proc main ()
{
    "This test tries running various script constructs to check for\n"
    "basic functionality.\n"
    "In the process it generates some interesting statistics about the data\n\n"
    set(verbose,1)
    set(verbose,0)
    set(np,0)    /* count of indi with no parents */
    set(mmp,0)   /* mis matched parents */

    list(lspouses)
    list(lispouses)
    list(lfamilies)
    list(lchildren)
    set(indicnt,0)
    "Running INDI related constructs \n"
    "   fathers, mothers, Parents, families function: nspouses\n"
    forindi(ind, cnt) {
        incr(indicnt)
        set(f,0)
        set(m,0)
        set(p,0)
        fathers(ind,fath,ffam,fcnt) {
            incr(f)
        }
        mothers(ind,fath,ffam,fcnt) {
            incr(m)
        }
        Parents(ind,fam,pcnt) {
            incr(p)
        }
        if (sum,add(f,m)) {
            if (or(ne(p,f),ne(p,m))) {
                if (verbose) {
                    "Missing Parents  " name(ind) " s=" d(sum) " p=" d(p) " f=" 
                    d(f) " m=" d(m) nl()
                }
                incr(mmp)
            }
        }   else {
            if (verbose) {
                "No Parents " name(ind) " s=" d(sum) " p=" d(p) " f=" 
                d(f) " m=" d(m) nl()
            }
            incr(np)
        }
        set(fcnt,0)
        families(ind,afam,ind1,cntf) {
            incr(fcnt)
        }
        setel(lfamilies,fcnt,add(getel(lfamilies,fcnt),1))
        if (ne(fcnt,nfamilies(ind))) {
            "Error, nfamilies disagrees with counting 'families'\n"
        }
        setel(lispouses,nspouses(ind),add(1,getel(lispouses,nspouses(ind))))
    }   /* end of forindi */
    /* call routines based on fams */
    "Running FAM related constructs \n"
    "   forfam, spouses, children, function: nchildren \n"
    set(famcnt,0)
    forfam(afam,cntf) {
        incr(famcnt)
        set(ns,0)
        spouses(afam,ai,icnt) {
            incr(ns)
        }
        setel(lspouses,ns,add(getel(lspouses,ns),1))
        /* for each fam, see how many children */
        set(childcnt,0)
        children(afam,ind,ccnt) {
            incr(childcnt)
        }
        setel(lchildren,childcnt,add(getel(lchildren,childcnt),1))
        if (ne(childcnt,nchildren(afam))) {
            "childcnt = " pvalue(childcnt)
            " nchildren(afam) = " pvalue(nchildren(afam)) "\n"
            "Error, childcnt(" d(childcnt) 
            ") and count from iterating over children(" d(nchildren(afam)) ") differ\n"
        }
        /*#firstchild(fam)
        lastchild9fam)
        */
    }
    "Misc. iterators\n"
    "   firstfam, nextfam, prevfam, lastfam \n"
    set(famcnt1,0)
    if(f,firstfam()) {
        incr(famcnt1)
        while (f,nextfam(f)) {
            incr(famcnt1)
        }
    }
    if (ne(famcnt1,famcnt)) {
        "forfam got " d(famcnt) " fam's, nextfam got " d(famcnt1) nl()
    }
    set(famcnt1,0)
    if(f,lastfam()) {
        incr(famcnt1)
        while (f,prevfam(f)) {
            incr(famcnt1)
        }
    }
    if (ne(famcnt1,famcnt)) {
        "forfam got " d(famcnt) " fam's, prevfam got " d(famcnt1) nl()
    }

    "   forsour, foreven, forothr\n"
    set(sourcnt,0)
    forsour(asour,fcnt) {
        incr(sourcnt)
    }
    set(evencnt,0)
    foreven(aeven,fcnt) {
        incr(evencnt)
    }
    set(othrcnt,0)
    forothr(aothr,fcnt) {
        incr(othrcnt)
    }

    "   firstindi, nextindi, previndi, lastindi " nl()

    set(i,firstindi())
    set(j,firstindi())
    "first indi is " key(i) nl()
    if (verbose) {
        "start indi is " key(i) nl()
    }
    set(cnt,0)
    while(j) {
        set(j,nextindi(j))
        if (verbose) {
            "next indi is " key(j) nl()
        }
        incr(cnt)
    }
    set(cnt1,0)
    set(j,lastindi())
    "last indi is " key(j) nl()
    if (verbose) {
        nl()
        "last indi is " key(j) nl()
    }
    while (j) {
        set(j,previndi(j))
        if (verbose) {
            "prev indi is " key(j) nl()
        }
        incr(cnt1)
    }
    if (ne(cnt,cnt1)) {
        "Error, with nextindi,previndi\n"
    }
set(verbose,1)
set(n,indi("I3"))
set(ncnt,0)
traverse(n,anode,cnt) {
    incr(ncnt)
}
"traverse found " d(ncnt) " nodes decendents of I3\n"
set(ncnt,0)
fornodes(n,anode) {
    incr(ncnt)
}
"fornodes found " d(ncnt) " nodes decendents of I3\n"
/* need 
   forindiset
*/

    "\nSummary\n"
    d(indicnt) " individuals\n"
    d(famcnt) " families\n"
    d(sourcnt) " sources\n"
    d(evencnt) " events\n"
    d(othrcnt) " Other\n"
    d(np) " Indi's with no Parents\n"
    d(mmp) " fam's with different number of fathers and mothers\n"
    "Histogram of number of families for each individual\n"
    forlist(lfamilies,val,ind) {
        d(ind) " -> " d(val) "\n"
    }
    "Histogram of number of spouses for each family\n"
    forlist(lspouses,val,ind) {
        d(ind) " -> " d(val) "\n"
    }
    "Histogram of number of spouses for each individual\n"
    forlist(lispouses,val,ind) {
        d(ind) " -> " d(val) "\n"
    }
    "Histogram of number of children in each family\n"
    forlist(lchildren,val,ind) {
        d(ind) " -> " d(val) "\n"
    }
}


