proc main ()
{
        "THE FOLLOWING PERSONS ARE 'ISOLATED' IN YOUR DATABASE" nl() nl()
        forindi(indi, num) {
                if (and(not(parents(indi)), eq(0,nfamilies(indi)))) {
                        name(indi) " (" key(indi) ")" nl()
                }
        }
}

