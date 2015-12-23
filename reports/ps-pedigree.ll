/*
 * @progname    ps-pedigree.ll
 * @version     1.1.0
 * @author      Stephen Woodbridge,  woodbri@swoodbridge.com
 * @category
 * @output      PostScript
 * @description
 *
 *   This report generates Multiple linked Pedigree Charts
 *   Each chart is 7 or 8 generations and as a line moves off
 *   a chart the new chart number is referenced. The output
 *   of this report is a POSTSCRIPT file. The text size is very
 *   small but readable (it seams less readable as I age!) on
 *   8.5x11 paper with 8 generations and larger but somewhat
 *   compressed at 7 generations per chart. And an index of all
 *   persons on the charts is also created.
 *
 *   Code by Stephen Woodbridge, woodbri@swoodbridge.com
 *   Copyright 1992 by Stephen Woodbridge
 *
 *   Version one of this report was written in XLISP and this is a
 *   direct translation of that Lisp code.
 *
 *   --- Version control info ---
 *
 *   10/22/92 - First Release 1.0.0
 *   10/28/92 - changed box width to expand the text font
 *              added CENTER_LAST global to center names in last boxes
 *   11/05/92 - Release 1.1.0 Added name sorted index and misc. other
 *              features and enhancements.
 *   11/03/03 - Included into LifeLines standard distribution.
 *              ps-pedi.ps renamed to ps-pedigree.ps
 *
 *   --- Comments about the program ---
 *
 *   There are lots of global flags that control whether or not aspects
 *   of the output are generated. These are set in "init_globals" and
 *   the comments there will explain them.  The title string for the
 *   index is also set here. The program will also generate an index of
 *   just the people in the pedigree OR all people in the database. This
 *   is controlled by the flag INDEX_ALL.
 *
 *   All global are in capitals. Global constants are set in
 *   init_globals  and are not changed as the program runs. The global
 *   variables are used throughout the execution.
 *   There is a global TRACE which will print most proc names as they
 *   are executed. This is helpful in tracking down SEGV crashes. There
 *   is a global LIST which will print the name of each person or a "."
 *   as it is processed. The enqueueing of people to be processed is
 *   done in plot_me.
 *
 *   You can adjust the margins on the paper. This has the effect of
 *   pushing the plot off the top/bottom/left/right. See M_TOP/M_BOT/
 *   M_LEFT/M_RIGHT in  init_globals.  The current setting leaves a
 *   margin at the top for three-hole punching or binding.
 *
 *   --- Comments about the PostScript output ---
 *
 *   You can change the paper size without regenerating the output.
 *   The plot will scale to fit the paper. A ledger size paper makes
 *   the plots much easier to read. This can be done by editing line
 *   66 in the output file. Just above this line are definitions for
 *   "a-size","a4-size" and "b4-size" paper. You can add your own paper
 *   sizes  and reference them on line 66.
 *
 *   Changing the small text font size will not nessasarily change the
 *   output on the paper because I compute an x and y scale factor the
 *   forces the chart into the bounds of the paper. Feel free to
 *   experiment and let me know if you get a good combination.
 *
 */

/* global variables */

global(RVAL)        /*  stack used to return values from procs */
global(ILIST)        /*  indi's to be done in next depth of charts */
global(NLIST)        /*  chart num of indi's above */
global(WHICH_CHART)    /*  table xrefs of indi to chart number */
global(FROM_CHART)
global(INDXSET)

global(CHART_NO)
global(CURRENT_CHART_NO)
global(PAGE)        /*  postscript page number being outputed */
global(PAGE_INDX)

/* global constants */

global(M_BOT)
global(M_LEFT)
global(M_RIGHT)
global(M_TOP)

global(LF_HGT)
global(LF_WDT)
global(SF_HGT)
global(SF_WDT)

global(BOX_H)
global(BOX_DH)

global(BOX_NC_1)
global(BOX_NC_2)

global(BOX_W)
global(BOX_WW)
global(BOX_SP)
global(BOX_DW)

global(CHART_PREFIX)
global(LEN_CHART_PREFIX)

global(TEXT_HGT)
global(TEXT_WDT)

global(INDEX_SIZE)
global(INDEX_LPP)
global(HEADER_SIZE)

global(LINE_COUNT)

global(PLOT_INUMS)
global(PLOT_DATE)
global(CENTER_LAST)
global(INDEX_ALL)
global(TITLE)

global(TRACE)
global(LIST)
global(PS_HDR_FILE)

/*
 *--------------------------------------------------------*
 */

proc main ()
{
    set(TRACE, 0)    /* trace proc calling sequence to trace down
                        SEGV: signal 11 crashes */

    set(LIST, 0)    /* list names as they are processed */

    call init_globals()

    list(RVAL)
    list(ILIST)
    list(NLIST)
    table(WHICH_CHART)
    table(FROM_CHART)
    indiset(INDXSET)

    getindi(me)

    /*
     *  The program can make 3 thru n generation charts
     *  but only the 7 and 8 have good aspect ratios that
     *  make them usable.
     */
    getintmsg(max, "Enter max generations per chart [7 or 8]")
    if (or( eq(max, 7), eq(max, 8)))
    {
        getintmsg(dmax, "Enter max depth of charts:")

        enqueue(ILIST, me)
        enqueue(NLIST, 1)

        call plot_init(max, TITLE)
        set(i, 1)
        while(le(i, dmax))
        {
            set (jlist, ILIST)
            set (mlist, NLIST)
            list(ILIST)
            list(NLIST)
            while (me, dequeue (jlist))
            {
                set(cno, dequeue(mlist))
                set(CURRENT_CHART_NO, cno)
                call new_plot_page(cno)
                call do_ancestors(me, 1, 0, max)
                call title_chart(cno, me, max)
            }
            set(i, add(i, 1))
        }
        call plot_fini()

        call do_index()
        call index_fini()
    }
}

proc init_globals()
{
    /* initialize global constants */

            /* Paper margins for output in points */
    set(M_TOP, 27)    /* 0.375in*72points/in */
    set(M_BOT, 0)
    set(M_LEFT, 0)
    set(M_RIGHT, 0)

            /* Large and small font sizes in points */
    set(LF_HGT, 18)
    set(LF_WDT, 12)
    set(SF_HGT, 5)
    set(SF_WDT, 4)

            /* Size of text in boxes */
    set(TEXT_HGT, SF_HGT)
    set(TEXT_WDT, SF_WDT)

            /* height of box and vertical spacing */
    set(BOX_H, add(1, TEXT_HGT))
    set(BOX_DH, add(1, BOX_H))

            /*  width of boxes in number of characters  */
    set(BOX_NC_1, 42)
    set(BOX_NC_2, 30)

            /*  width of boxes and horizontal spacing  */
    set(BOX_W, mul(BOX_NC_2, TEXT_WDT))
    set(BOX_WW, mul(BOX_NC_1, TEXT_WDT))
    set(BOX_SP, div( mul(BOX_W, 3), 20))    /* BOX_W*0.15 */
    set(BOX_DW, add(BOX_W, BOX_SP))

            /*  controls for the index  */
    set(INDEX_SIZE, 8)
    set(INDEX_LPP, 80)
    set(HEADER_SIZE, 10)

            /*  controls for what and how the charts appear */
    set(CHART_PREFIX, "")    /* if CHART_PREFIX=0 then don't number charts */
    set(LEN_CHART_PREFIX, 0)
    set(PLOT_INUMS, 1)    /* bool 0=don't plot inums, 1=plot inums */
    set(PLOT_DATE, 1)     /* bool 0=don't date charts, 1=date charts */
    set(CENTER_LAST, 1)   /* bool 0=don't center names in last column,
                                  1=center names */
    set(INDEX_ALL, 0)     /* bool 0=only index names on charts,
                                  1=index all names in database */

    /* global variables used to keep track of which chart */

    set(CHART_NO, 1)
    set(CURRENT_CHART_NO, 0)
    set(PAGE, 0)
    set(PAGE_INDX, 1)

    set(PS_HDR_FILE, "ps-pedigree.ps")   /* PostScript Header file name */

    set(TITLE, "Pedigree Index")     /* Title string for Index pages */

    dayformat(0)
    monthformat(3)
    dateformat(0)
}

proc do_ancestors (me, depth, width, max)
{
if (TRACE) { print("do_ancestors ") }
    if (me)
    {
        if (LIST) {
            print(fullname(me,1,0,40)) print(" -")
            print(key(me)) print(sp()) print(d(depth))
            print(sp()) print(d(width)) print(nl())
        } else
            { print(".") }

        set(my_tag, lookup(WHICH_CHART, key(me)))
        call plot_me(me, depth, width, max)
        if ( and( or( eq(1, depth), not(my_tag)), lt(depth, max)))
        {
            if (dad, father(me))
            {
                call get_width(1, width)
                set(nwid, pop(RVAL))
                call do_ancestors(dad, add(1, depth), nwid, max)
                call connect_boxes( me, depth, width, nwid, max)
            }
            if (mom, mother(me))
            {
                call get_width(neg(1), width)
                set(nwid, pop(RVAL))
                call do_ancestors(mom, add(1, depth), nwid, max)
                call connect_boxes( me, depth, width, nwid, max)
            }
        }
        else
        {
            call box_org(depth, width, max)
            call draw_ext(me, pop(RVAL), pop(RVAL), my_tag, eq(depth, max))
        }
    }
}

proc plot_me (me, depth, width, max)
{
if (TRACE) { print("plot_me ") }
    set(last, eq(max, depth))
    set(first, eq(1, depth))
    set(style, ge(add(1, depth), max))
    call box_org(depth, width, max)
    set(my_x, pop(RVAL))
    set(my_y, pop(RVAL))
    /*
     * This if controls whether or not siblings are plotted
     */
    if (first) { call do_sibs(me, my_x, my_y, last) }
        else { call box_me(me, my_x, my_y, last) }

    if (not(lookup(WHICH_CHART, key(me))))
    {
        set(ntag, CURRENT_CHART_NO)
        if (and( last, parents(me)))
        {
            set(CHART_NO, add(1, CHART_NO))
            set(ntag, CHART_NO)
            call draw_ext(me, my_x, my_y, ntag, last)
            enqueue(ILIST, me)
            enqueue(NLIST, ntag)
            insert(FROM_CHART, save(d(CHART_NO)), CURRENT_CHART_NO)
        }
        insert(WHICH_CHART, save(key(me)), ntag)
        addtoset(INDXSET, me, ntag)
    }
}

proc box_me (me, x, y, last)
{
if (TRACE) { print("box_me ") }
    call get_dates(me)
    call print_name(me, 0)
    if (PLOT_INUMS) { set(num, save(concat("-", key(me)))) }
        else { set(num, "") }
    call draw_box_text(x, y, pop(RVAL), pop(RVAL), num, last)
}

proc do_sibs (me, x, y, last)
{
if (TRACE) { print("do_sibs ") }
    set(nkids, nchildren(parents(me)))
    set(bdh, mul(2, BOX_DH))
    set(sy, div(mul(sub(nkids, 1), bdh), 2))
    children( parents(me), child, nchild)
    {
        set(yy, add(y, sy))
        call box_me(child, x, yy, last)
        set(sy, sub(sy, bdh))
    }
}

proc do_index()
{
if (TRACE) { print("do_index ") }
    print(nl()) print("Collecting Index ...")
    if (INDEX_ALL)
    {
        forindi(me, num)
        {
            if (not(lookup(WHICH_CHART, key(me)))) { addtoset(INDXSET, me, 0) }
        }
    }
    print(nl()) print("Sorting Index ...")
    namesort(INDXSET)
    print(nl()) print("Outputing Index ")
    forindiset(INDXSET, me, chart, num)
        { call index_out(me, chart) print(".") }
}

/*
 *             --------  Postscript output routines ---------
 */

proc plot_init (max, title)
{
if (TRACE) { print("plot_init ") }
    set(PAGE, 0)
    copyfile(PS_HDR_FILE)
    call expt(2, sub(max, 2))
    set(h, mul( add( pop(RVAL), 1), mul(2, BOX_DH)))
    set(w, div( mul( add(max, 1), BOX_W), 2))
    set(w, add(w, add( mul(max, BOX_SP), BOX_WW)))
    if (CHART_PREFIX)
        { set(w, add(w, mul( add(LEN_CHART_PREFIX, 3), TEXT_WDT))) }

    "%%BeginSetup" nl()

    "/pointsize " d(INDEX_SIZE) " def" nl()
    "/headerpointsize "d(HEADER_SIZE) " def" nl()
    "/filename (" title ") def" nl()
    "/noheader false def" nl()
    "/date (" date(gettoday()) ") def" nl()

    "/nc-1 " d(BOX_NC_1) " def" nl()
    "/nc-2 " d(BOX_NC_2) " def" nl()
    "/margin-l " d(M_LEFT) " def" nl()
    "/margin-r " d(M_RIGHT) " def" nl()
    "/margin-t " d(M_TOP) " def" nl()
    "/margin-b " d(M_BOT) " def" nl()
    "/width-needed " d(w) " def" nl()
    "/height-needed " d(h) " def" nl()
    "/text-wdt " d(TEXT_WDT) " def" nl()
    "/text-hgt " d(TEXT_HGT) " def" nl()
    "setup" nl()
    "/newpagesetup save def" nl()
    "mark" nl()

    "%%EndSetup" nl()

    set(LINE_COUNT, 0)
}


proc new_plot_page (page_no)
{
if (TRACE) { print("new_plot_page ") }
    set(PAGE, add(1, PAGE))
    "%%Page: " d(page_no) " " d(PAGE) nl() "mark plotpagesetup" nl()
}

proc plot_fini ()
{
    set(PAGE, add(1, PAGE))
}

proc draw_box_text (x, y, name, date, num, last)
{
if (TRACE) { print("draw_box_text ") }
    if (last)
        { "(" name " " date "  " num ") "
          if(CENTER_LAST) { set(t, " ct1") } else { set(t, " t1")}
        }
    else
        { "(" name "  " num ") (" date ") " set(t, " t2") }
    d(x) " " d(y) t nl()
}

proc draw_ext (me, x, y, chartno, last)
{
if (TRACE) { print("draw_ext ") }
    if (parents(me))
    {
        if (last) { set(bw, div(BOX_WW, 2)) }
            else { set(bw, div(BOX_W, 2)) }
        "np " d(add(x, bw)) " " d(y)
        " mto " d(div(BOX_SP, 3)) " 0 rlto drw" nl()
        if (and( chartno, CHART_PREFIX))
        {
            d( add(x, add(bw, add(TEXT_WDT, div(BOX_SP, 3))))) " "
            d( sub(y, div(TEXT_HGT, 2))) " mto ("
            CHART_PREFIX d(chartno) ") show" nl()
        }
    }
}

proc connect_boxes (me, depth, width1, width2, max)
{
if (TRACE) { print("connect_boxes ") }
    call box_org(depth, width1, max)
        set(x1, pop(RVAL))
        set(y1, pop(RVAL))
    call box_org(add(1, depth), width2, max)
        set(x2, pop(RVAL))
        set(y2, pop(RVAL))
    set(dx, div( add(x1, x2), 2))
    set(w2, div(BOX_W, 2))
    set(w3, div(BOX_WW, 2))
    set(dh, 0)
    set(dw, w2)
    set(rad, BOX_H)
    set(style, 0)
    if (eq(depth, 1))
    {
        set(nkids, nchildren(parents(me)))
        set(sy, div( mul( sub(nkids, 1), mul(2, BOX_DH)), 2))
        if (gt(width2, 0))
            { set(y1, add(y1, sy)) } else { set(y1, sub(y1, sy)) }
    }
    if (lt(y1, y2))
        { set(dh, BOX_H) } else { set(dh, neg(BOX_H)) }
    if (eq( sub(max, depth), 1))
    {
        set(dw, w3)
        set(style, 1)
        set(rad, div(rad, 2))
        set(dx, div( sub( add(x1, add(w2, x2)), w3), 2))
    }
    elsif( eq( sub(max, depth), 2))
    {
        set(dw, w2)
        set(style, 1)
    }
    if (style)
    {
        d(div(rad, 2)) " gr np " d(add(x1, w2)) " " d(y1) " mto "
        d(dx) " " d(y1) " " d(dx) " " d(y2) " pto "
        d(sub(x2, dw)) " " d(y2) " pto lto drw" nl()
    }
    else
    {
        d(rad) " gr np " d(x1) " " d(add(y1, dh)) " mto "
        d(x1) " " d(y2) " " d(sub(x2, w2)) " " d(y2) " pto lto drw" nl()
    }
}

proc title_chart (chart_no, me, max)
{
if (TRACE) { print("title_chart ") }
    if (gt( sub(max, 2), 0))
    {
        set(x, 0)
        call expt(2, sub(max, 2))
        set(y, mul( add( pop(RVAL), 1), mul(2, BOX_DH)))
        set(w, div( mul( add(max, 1), BOX_W), 2))
        set(w, add(w, add( mul(max, BOX_SP), BOX_WW)))
        if (CHART_PREFIX)
            { set(w, add(w, mul( add(4, LEN_CHART_PREFIX), TEXT_WDT))) }
        d(y) " " d(w) " " d(x) " 0 mbox 18 1 rbox" nl()
        if (PLOT_DATE)
        {
            d(add(x, LF_WDT)) " 1.2 mul " d(div(SF_HGT,2)) " mto ("
            date(gettoday()) ") show" nl()
        }
        d(LF_WDT) " " d(LF_HGT) " mfont" nl()
        call get_dates(me)
        call print_name(me, 1)
        d(add(x, mul(2, LF_WDT))) " "
            d(sub(y, add(LF_HGT, div(LF_HGT, 2)))) " mto ("
            pop(RVAL) ") show" nl()
        d(add(x, mul(2, LF_WDT))) " "
            d(sub(y, add( mul(LF_HGT, 2), div(LF_HGT,2)))) " mto ("
            pop(RVAL) ") show" nl()
        if (CHART_PREFIX)
        {
            d(add(x, LF_WDT)) " " d(div(LF_HGT,2)) " mto (Chart: "
            CHART_PREFIX d(chart_no)
            if (e, lookup(FROM_CHART, d(chart_no)))
                { "  From: " d(e) }
            ") show" nl()
        }
        "cleartomark showpage" nl()
        "%%EndPage: " d(PAGE) " " d(PAGE) nl()
    }
}

/*
 *             --------  Postscript output routines for index ---------
 */

proc index_fini()
{
if (TRACE) { print("index_fini ") }
    "cleartomark showpage" nl()
    "%%EndPage: " d(PAGE) " " d(PAGE) nl()
    "%%Trailer" nl()
    "%%Pages: " d(PAGE) nl()
}

proc index_out (me, chart)
{
if (TRACE) { print("index_out ") }
    set(blanks, "                                                  ")
    if (not(mod(LINE_COUNT, INDEX_LPP)))
    {
        "%%Page: " d(PAGE) " " d(PAGE) nl()
        "mark indexpagesetup " d(PAGE_INDX) " pagesetup"  nl()
    }

    "("
    if (chart) { call rjt(chart, 5) pop(RVAL) } else { "    " }
    "  " trim( save( concat( key(me),"      ")), 6)
    call get_dates(me)
    call print_name(me, 1)
    "  " trim( save( concat(pop(RVAL),blanks)), 50) " " sex(me)
    " " pop(RVAL) ")l" nl()

    set(LINE_COUNT, add(LINE_COUNT,1))
    if (not(mod(LINE_COUNT, INDEX_LPP)))
    {
        "cleartomark showpage" nl()
        "%%EndPage: " d(PAGE) " " d(PAGE) nl()
        set(PAGE, add(PAGE, 1))
        set(PAGE_INDX, add(PAGE_INDX, 1))
        set(LINE_COUNT, 0)
    }
}

/*
 *             --------  Utility routines ---------
 */

proc print_name (me, last)
{
if (TRACE) { print("print_name ") }
    call get_title(me)
    push(RVAL, save(concat(fullname(me, 1, not(last), 45), pop(RVAL))))
}

proc get_title (me)
{
if (TRACE) { print("get_title ") }
    fornodes(inode(me), node)
    {
        if (not(strcmp("TITL", tag(node)))) { set(n, node) }
    }
    if (n) { push(RVAL, save(concat(" ", value(n)))) }
        else { push(RVAL, "") }
}

proc get_dates (me)
{
if (TRACE) { print("get_dates ") }
    if (e, birth(me)) { set(b, save(concat("( ", date(e)))) }
        else { set(b, "( ") }
    if (e, death(me)) { set(d, save(concat(" - " , date(e)))) }
        else { set(d, " - ") }
    push(RVAL, save(concat(b, concat(d, " )"))))
}

proc box_org (depth, width, max)
{
if (TRACE) { print("box_org ") }
    set(xx, div( mul(BOX_W, 9), 16))
    call expt(2, sub(max, 2))
    set(yy, mul( add( pop(RVAL), 1), BOX_DH))
    if ( eq(depth, 1))
        { push(RVAL, yy) push(RVAL, xx) }
    else
    {
        call expt(2, sub(max, depth))
        set(dy, mul( pop(RVAL), BOX_DH))
        call abs(width)
        set(y, sub( mul(pop(RVAL), dy), div(dy, 2)))
        set(dx, add(BOX_SP, div(BOX_W, 2)))
        set(dd, sub( sub(max, 2), depth))
        set(x, 0)
        if ( eq(dd, neg(1)))
            { set(dxx, div(BOX_W, 2)) }
        elsif (eq(dd, neg(2)))
            { set(dxx, add( div(BOX_W, 2), div(BOX_WW, 2))) }
        else
            { set(dxx, 0) }
        set(x, add(dxx, add(xx, mul(dx, sub(depth, 1)))))
        if ( lt(width, 0)) { set(y, neg(y)) }
        push(RVAL, add(yy, y))
        push(RVAL, x)
    }
}

proc get_width (sign, width)
{
if (TRACE) { print("get_width ") }
    if (eq(width, 0))
        { push(RVAL, sign) }
    else
    {
        call abs(width)
        set(awidth, pop(RVAL))
        set(s2, div(width, awidth))
        if (eq(s2, sign))
            { push(RVAL, mul(width, 2)) }
        else
            { push(RVAL, mul( sub( mul(awidth, 2), 1), s2)) }
    }
}

proc abs (int)
{
if (TRACE) { print("abs ") }
    if (lt(int, 0))
        { push(RVAL, neg(int)) }
    else
        { push(RVAL, int) }
}

proc rjt(n, w)
{
    if (lt(n, 10)) { set(d, 1) }
    elsif (lt(n, 100)) { set(d, 2) }
    elsif (lt(n, 1000)) { set(d, 3) }
    elsif (lt(n, 10000)) { set(d, 4) }
    else  { set(d, 5) }
    if (lt(d, w))
        { set(pad, save( trim("      ", sub(w, d)))) }
    else
        { set(pad, "") }
    push(RVAL, save( concat(pad, save(d(n)))))
}

proc expt(x, y)
{
if (TRACE) { print("expt ") }
    if (le(y, 0)) { set(result, 1) }
    else
    {
        set(result, x)
        while (y, sub(y,1))
            { set(result, mul(result, x)) }
    }
    push(RVAL, result)
}

