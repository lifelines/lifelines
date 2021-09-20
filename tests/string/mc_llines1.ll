/*
 * @progname       mc_llines1
 * @version        1.0
 * @author         Stephen Dum
 * @category       test program
 * @output         text
 * @description
 * This program tests the behavior of menuchoice(list,prompt)
 * it is basically a reference behavior on how llines behaves.
 * The tests mc_llexec and mc_llexec1 test the non gui implementation
 * see those tests for details.
 *
 * This test shows an (? undocumented) use of input characters [ and ]
 * basically typing a [ usually pops up sub window to the menuchoice window
 * showing current selection and other info. Entering a second [ (? or ])
 * will cause a memory fault.
 * free(): double free detected in tcache 2
 * src/liflines/screen.c:462 *  ( stdfree((STRING)uiwin->name);
 * 
 */
proc main () 
{
    list(options)
    setel(options,1,"Are you happy today")
    setel(options,2,"or is it a sad day for you")
    set(resp, menuchoose(options,"select your mood - menu 1:"))
    "Menu 1, response was " d(resp) "\n"

    set(resp, menuchoose(options,"select your mood - menu 2:"))
    "Menu 2, response was " d(resp) "\n"

    set(resp, menuchoose(options,"select your mood - menu 3:"))
    "Menu 3, response was " d(resp) "\n"

    set(resp, menuchoose(options,"select your mood - menu 4:"))
    "Menu 4, response was " d(resp) "\n"

    list(options)
    setel(options,1,"Are you happy today")
    setel(options,3,"or is it a sad day for you")
    set(resp, menuchoose(options,"select your mood - menu 5:"))
    "Menu 5, response was " d(resp) "\n"

    list(options)
    push(options,"Option 1")
    push(options,"Option 2")
    push(options,"Option 3")
    push(options,"Option 4")
    push(options,"Option 5")
    push(options,"Option 6")
    push(options,"Option 7")
    push(options,"Option 8")
    push(options,"Option 9")
    push(options,"Option 10")
    push(options,"Option 11")
    push(options,"Option 12")
    push(options,"Option 13")
    push(options,"Option 14")
    push(options,"Option 15")
    push(options,"Option 16")
    push(options,"Option 17")
    push(options,"Option 18")
    push(options,"Option 19")
    push(options,"Option 20")
    push(options,"Option 21")
    push(options,"Option 22")
    push(options,"Option 23")
    push(options,"Option 24")
    push(options,"Option 25")
    set(resp, menuchoose(options,"select your mood - menu 6:"))
    "Menu 6, response was " d(resp) "\n"
}
