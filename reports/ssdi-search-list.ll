/*
 * @progname       ssdi-search-list.ll
 * @version        1.0
 * @author         Larry Soule (lsoule@ikos.com)
 * @category       
 * @output         Text
 * @description
 *
 * This LifeLines report program searches for individuals in the database
 * that are missing some birth or death information that may be in the
 * social security death index (SSDI).  Right now this searches for:
 *      1. Deaths after 1960 that do not have locations
 *      2. Births after 1880 with no death event
 *
 * These two sets of people are sorted by name and printed out in the
 * report in ASCII.
 *
 * The first set of people, those with deaths after 1960 that do not
 * have locations, is the most promising to search for.  The second set
 * right now contains many living people but also other possible
 * entries in the SSDI.
 *
 * The social security death index is available at your local Family History
 * Library or on-line at http://www.ancestry.com/ssdi/
 *
 * Version 1.0 - November 1996, Larry Soule (lsoule@ikos.com)
 *
 * Sample report output (note: all spouses are listed for female individuals
 * since they may be listed under their maiden name, or any other married name)
 *

2207 individuals in the database.
52 have known death dates but not locations.
331 have known birth dates but no death dates or locations.

**** List of individuals with death dates but not locations
Charles Edwin ALBRIDGE        b. 27 NOV 1915     Pennsylvania
                              d. 03 DEC 1981

Evelyn Carter ALBRIDGE        b. 26 MAR 1905     Pennsylvania
                              d. 06 OCT 1982
    Married to Chester Goy RAVER
...

**** List of individuals with birth dates but not death dates or location
Alice Alamanda ALBRIDGE       b. 04 FEB 1902     Easton, Northampton Co., PA
                              d.
...

 */

/* These two sets are built up */
global(missingDeathPlaceSet)
global(missingDeathEventSet)

proc main() {
    /* Generate the two sets of people */
    call generateSetToSearch()

    /* Now print the two sets */
    "**** List of individuals with death dates but not locations" nl()
    call printSet(missingDeathPlaceSet)

    nl() nl()
    "**** List of individuals with birth dates but not death dates or location" nl()
    call printSet(missingDeathEventSet)
}

/*
 * Generate the two sets of individuals
 */
proc generateSetToSearch() {
    indiset(missingDeathPlaceSet)
    indiset(missingDeathEventSet)

    forindi(indi_v, count_v) {
        set(deathEv, death(indi_v))
        set(birthEv, birth(indi_v))
        if (deathEV, death(indi_v)) {
          /*
           * A death record exists - see if the location is empty and
           * the date is after 1960
           */
            if (and(eq(0, strlen(place(deathEv))),
                    gt(atoi(year(deathEv)), 1960))) {
                addtoset(missingDeathPlaceSet, indi_v, 0)
            }
        } else {
          /*
           * No death record exists - see if the birth year
           * is after 1880
           */
            if (birthEV, birth(indi_v)){
              if (gt(atoi(year(birthEv)), 1880)) {
                addtoset(missingDeathEventSet, indi_v, 0)
              }
            }
        }
    }

    /* Output some statistics */
    d(count_v) " individuals in the database." nl()
    d(lengthset(missingDeathPlaceSet)) " have known death dates but not locations." nl()
    d(lengthset(missingDeathEventSet)) " have known birth dates but no death dates or locations." nl() nl()

    /* Sort the two sets by name */
    namesort(missingDeathPlaceSet)
    namesort(missingDeathEventSet)
}

/*
 * Print the set of individuals passed in the argument printSet.
 * This uses a simple name, birth, death format, followed by a list
 * of spouses for females
 */
proc printSet(printSet) {
    forindiset(printSet, personIndi, personValue, iteration) {
        set(birthEv, birth(personIndi))
        set(deathEv, death(personIndi))
        fullname(personIndi, 1, 1, 30) col(30) " b. " date(birthEv) col(50) place(birthEv)
        nl() col(30) " d. " date(deathEv) col(50) place(deathEv) nl()
        if (female(personIndi)) {
            if (gt(nspouses(personIndi), 0)) {
                spouses(personIndi, spouse_v, fam_v, count) {
                    "    Married to " name(spouse_v) nl()
                }
            }
        }
        nl()
    }
}
