/*
 * @progname       mc_llexec1
 * @version        1.0
 * @author         Stephen Dum
 * @category       test program
 * @output         text
 * @description
 * This program tests the behavior of menuchoice(list,prompt)
 * It was designed to exercise a number of bugs found prior to 
 * lifelines V. 3.0.2 to make it closer to the llines behavior
 * 1.  The prompt is printed, and at end of prompt is printed
 *     (n,m)  where n is index of first listed item, and m is last
 *            possible item (last index in list)
 *     (this wasn't printed prior to lifelines llexec v. 3.0.2)
 * 2. llexec prints index numbers that can be selected. it lists
 *    items 1..9 initially and user an scroll down to 10..19, 20..29 etc
 *    The indexes listed are 1..9 to start and 0..9 for options whose
 *    index is > 9. Essentially what's listed is index % 10.
 *    but full value is returned. (the current range start is listed
 *    at the end of the prompt string.
 *    using the u can d key strokes. menuchoice scrolls the index values
 *    listed. one of 1..m based on your selection
 *    prior to lifelines 3.0.2 the numbers were listed 0..(m-1)
 *    but 1..m was returned.
 * 3. llexec requires choices to be entered as <char><cr> i.e. one per line. 
 * 4. if llexec runs out of input looking for menuchoice selection info,
 *    menuchoice will return 0 (as it does for an empty options list)
 *    prior to 3.0.2 llexec went into a loop waiting for input
 *    test mc_llexec1 tests for this behavior.
 * 5. Prior to 3.0.2 the  Command: list included i to select an item,
 *    but this is not possible as the only selection mechanism is to
 *    enter a digit 0 thru 9.
 *
 */
proc main () 
{
    list(options)
    setel(options,1,"Are you happy today")
    setel(options,2,"or is it a sad day for you")
    set(resp, menuchoose(options,"select your mood - menu 1:"))
    "response was " d(resp) "\n"

    set(resp, menuchoose(options,"select your mood - menu 2:"))
    "response was " d(resp) "\n"

    set(resp, menuchoose(options,"select your mood - menu 3:"))
    "response was " d(resp) "\n"

    set(resp, menuchoose(options,"select your mood - menu 4:"))
    "response was " d(resp) "\n"

    list(options)
    setel(options,1,"Are you happy today")
    setel(options,3,"or is it a sad day for you")
    set(resp, menuchoose(options,"select your mood - menu 5:"))

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
    set(resp, menuchoose(options,"select your mood - menu 1:"))
    "response was " d(resp) "\n"
}
