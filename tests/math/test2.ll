/*
@progname test1.ll
@author Stephen Dum
@description verify simple math operations
*/
proc main ()
{
    /* check d() */
    /* 32 bit signed 2's complement numbers -2147483648 - 2147483647 */
    /* 64 bit 9223372036854775807 - 9223372036854775808 */

    "Lets try to discern if INT is 32 or 64 bits\n"
    "... and testing input of big integer literals, and printing of them\n"
    set(w,2147483642)  /* maxint - 5 */
    set(x,2147483644)  /* maxint - 3 */
    set(y,2147483646)  /* maxint - 1 */
    set(z,2147483647)  /* maxint - 0 */
    set(a,2147483648)  /* maxint + 1 aka -0 */ 
    set(b,2147483650)  /* maxint + 3 */
    set(c,2147483652)  /* maxint + 5 */
    "2147483642 - 5 " d(w) nl()
    "2147483644 - 3 " d(x) nl()
    "2147483646 - 1 " d(y) nl()
    "2147483647 + 0 " d(z) " 32 bit max signed int" nl()
    "2147483648 + 1 " d(a) nl()
    "2147483650 + 3 " d(b) nl()
    "2147483652 + 5 " d(c) nl()
    
    set(maxint,0)
    set(cnt, 0)
    set(i,w)
    set(last,i)
    while (lt(cnt,10)) {
        if (lt(i,0)) { break() }
        set(last,i)
        incr(i)
        incr(cnt)
    }
    if (lt(cnt,10)) {
        set(maxint,last)
        "flipped at " d(last) " to " d(i) " transition" nl()
        "maximum positive INT is " d(last) " INT must be 32 bit signed int" nl()
        set(bits,32)
    } else {
        "didn't flip in 10 iterations, last value was " d(i) nl()
        "INT must not be 32 bits\n"
    }

    if (eq(maxint,0)) {
    set(i,9223372036854775807)
        /* see if INT is 32 bits */
        set(w,9223372036854775802)  /* maxint - 5 */
        set(x,9223372036854775804)  /* maxint - 3 */
        set(y,9223372036854775806)  /* maxint - 1 */
        set(z,9223372036854775807)  /* maxint - 0 */
        set(a,9223372036854775808)  /* maxint + 1 aka -0 */ 
        set(b,9223372036854775810)  /* maxint + 3 */
        set(c,9223372036854775812)  /* maxint + 5 */
        "9223372036854775802 - 5 " d(w) nl()
        "9223372036854775804 - 3 " d(x) nl()
        "9223372036854775806 - 1 " d(y) nl()
        "9223372036854775807 + 0 " d(z) " 64 bit max signed int" nl()
        "9223372036854775808 + 1 " d(a) nl()
        "9223372036854775810 + 3 " d(b) nl()
        "9223372036854775812 + 5 " d(c) nl()
        
        set(bits,0)
        set(cnt, 0)
        set(i,w)
        set(last,i)
        while (lt(cnt,10)) {
            if (lt(i,0)) { break() }
            set(last,i)
            incr(i)
            incr(cnt)
        }
        if (lt(cnt,10)) {
            set(bits,last)
            "flipped at " d(last) " to " d(i) " transition" nl()
            "maximum positive INT is " d(bits) " INT must be 64 bit signed int" nl()
        } else {
            "didn't flip in 10 iterations, last value was " d(i) nl()
            "Don't know what the size of INT is!\n"
        }

    }

    "\nNext verify values around 0 are input and output ok\n"
    set(c,5)  /* maxint + 5 */
    set(b,3)  /* maxint + 3 */
    set(a,1)  /* maxint + 1 aka -0 */ 
    set(z,0)  /* maxint   0 */
    set(y,-1)  /* maxint - 1 */
    set(x,-3)  /* maxint - 3 */
    set(w,-5)  /* maxint - 5 */
    "+ 5 " d(c) nl()
    "+ 3 " d(b) nl()
    "+ 1 " d(a) nl()
    "  0 " d(z) nl()
    "- 1 " d(y) nl()
    "- 3 " d(x) nl()
    "- 5 " d(w) nl()
    
    set(cnt, 0)
    set(i,c)
    set(last,i)
    while (lt(cnt,10)) {
        if (lt(i,0)) { break() }
        set(last,i)
        decr(i)
        decr(cnt)
    }
    if (lt(cnt,10)) {
        "flipped at " d(last) " to " d(i) " transition" nl()
    } else {
        "didn't flip in 10 iterations, last value was " d(i) nl()
    }
    "\n"

    "Check results of simple 2 arg integer add, sub, mult, div\n"
    rjustify("i",10)  rjustify("j",10)  rjustify("add",10)
          rjustify("sub",10) rjustify("mul",10) rjustify("div",10) nl()
    set(i,-5)
    while(lt(i,5)) {
        set(j,-5 )
        while(lt(j,5)) {
            rjustify(d(i),10) rjustify(d(j),10)
              rjustify(d(add(i,j)),10)
              rjustify(d(sub(i,j)),10)
              rjustify(d(mul(i,j)),10)
              if (ne(j,0)) {
                  rjustify(d(div(i,j)),10) 
              } else {
                  rjustify("==",10) 
              }
              nl()
            incr(j)
        }
        incr(i)
    }
    rjustify("i",12)  rjustify("j",12)  rjustify("add",12)
          rjustify("sub",12) rjustify("mul",20) rjustify("div",8) nl()
    set(i,1)
    while(lt(i,10)) {
        set(i1,add(i1,40019387))
        set(j,1)
        while(lt(j,10)) {
            set(j1,add(j1,4019469))
            rjustify(d(i1),12) rjustify(d(j1),12)
              rjustify(d(add(i1,j1)),12)
              rjustify(d(sub(i1,j1)),12)
              rjustify(d(mul(i1,j1)),20)
              if (ne(j1, 0)) {
                  rjustify(f(div(float(i1),float(j1)),5),8) 
              } else {
                  rjustify("--",8) 
              }
              nl()
            incr(j)
        }
        incr(i)
   }

/* check out: mod exp and or */
    "\nCheck results of simple 2 arg integer mod, exp, boolean and and or\n"
     rjustify("i",10) rjustify("j",10)  rjustify("mod",10) rjustify("exp",10)
                rjustify("and",10) rjustify("or",10) nl()
    set(i,-5)
    while(lt(i,5)) {
        set(j,-5 )
        while(lt(j,5)) {
            rjustify(d(i),10) rjustify(d(j),10)
              if (eq(j,0)) {
                  rjustify("--",10)
              } else {
                  rjustify(d(mod(i,j)),10)
              }
              rjustify(d(exp(i,j)),10)
              rjustify(d(and(i,j)),10)
              rjustify(d( or(i,j)),10)
              nl()
            incr(j)
        }
        incr(i)
    }
/* eq ne lt */
    "\nCheck results of simple 2 arg integer eq ne lt\n"
     rjustify("i",10) rjustify("j",10)  rjustify("eq",10) rjustify("ne",10)
                      rjustify("lt",10)  nl()
    set(i,-5)
    while(lt(i,5)) {
        set(j,-5 )
        while(lt(j,5)) {
            rjustify(d(i),10) rjustify(d(j),10)
              rjustify(d(eq(i,j)),10)
              rjustify(d(ne(i,j)),10)
              rjustify(d(lt(i,j)),10)
              nl()
            incr(j)
        }
        incr(i)
    }
/* gt le ge */
     rjustify("i",10) rjustify("j",10)  rjustify("gt",10) rjustify("le",10) 
                    rjustify("ge",10) nl()
    set(i,-5)
    while(lt(i,5)) {
        set(j,-5 )
        while(lt(j,5)) {
            rjustify(d(i),10) rjustify(d(j),10)
              rjustify(d(gt(i,j)),10)
              rjustify(d(le(i,j)),10)
              rjustify(d(ge(i,j)),10)
              nl()
            incr(j)
        }
        incr(i)
    }
/*  one arg:  not neg  */
    "\nCheck results of one arg not, neg (incr,decr don't return value\n"
    "    and they are tested here as they are used for looping\n"
      rjustify("i",10) rjustify("not",10) rjustify("neg",10) nl()
    set(i,-5)
    while(lt(i,5)) {
        rjustify(d(i),10)
        rjustify(d(not(i)),10)
        rjustify(d(neg(i)),10)
        nl()
        incr(i)
    }

    "\nCheck results of float and int functions\n"
    /* float int  */
    rjustify("i",10) rjustify("j",10) rjustify("float",10) rjustify("float(,5)",10) rjustify("int",10) nl()
    set(i,-5)
    set(delta,1.750)
    set(ifl, -5.328)
    while(lt(i,5)) {
        set(j,-5 )
        while(lt(j,5)) {
            rjustify(f(ifl),10) rjustify(d(j),10)
              rjustify(f(float(j)),10)
              rjustify(f(float(j),5),10)
              rjustify(d(int(ifl)),10)
              nl()
            incr(j)
            set(ifl, add(ifl,delta))
        }
        incr(i)
    }
    /* sin cos tan arcsin arccos arctan */
    nl()
    "\nCheck results of trig functions\n"
    rjustify("i",12) rjustify("sin",12) rjustify("arcsin",12) rjustify("cos",12)
    rjustify("cos",12) nl()
    set(i,-315)
    while(lt(i,360)) {
        set(tmp,sin(i))
        rjustify(d(i),12)
        rjustify(f(tmp,7),12)
        rjustify(f(arcsin(tmp),7),12)
        set(tmp,cos(i))
        rjustify(f(tmp,7),12)
        rjustify(f(arccos(tmp),7),12)
        nl()
        incr(i, 60)
    }
    nl()
    rjustify("i",12) rjustify("tan",12) rjustify("arctan",12) nl()
    set(i,-315)
    while(lt(i,360)) {
        rjustify(d(i),12)
        set(tmp,tan(i))
        rjustify(f(tmp,7),12)
        rjustify(f(tmp,7),12)
        rjustify(f(arctan(tmp),7),12)
        nl()
        incr(i, 60)
    }

    /* dms2deg deg2dms spdist */
     " quick check of  dms2deg deg2dms spdist\n"
    /* Las Vegas, Clark County, Nevada, USA
     * N36.1699412 W115.1398296
     * Degrees Lat Long 	36.1699412°, -115.1398296°
     * Degrees Minutes	36°10.19647', -115°08.38978'
     * Degrees Minutes Seconds 	36°10'11.7883", -115°08'23.3866"
     *
     * Chicago, Cook County, Illinois, USA
     * N41.8781136 W87.6297982
     * Degrees Lat Long 	41.8781136°, -087.6297982°
     * Degrees Minutes	41°52.68682', -087°37.78789'
     * Degrees Minutes Seconds 	41°52'41.2090", -087°37'47.2735"
     */
    dms2deg(36,10,11.7883,flt)
    "dms2deg 35.10.11.7883 -> " f(flt,7) nl()
    deg2dms(flt,deg,min,sec)
    "deg2dms " f(flt,7) " -> " d(deg) "." d(min) "." d(sec) "\n"
    dms2deg(deg,min,sec,flt)
    "dms2deg " d(deg) "." d(min) "." d(sec) " -> " f(flt,7) nl()
    deg2dms(flt,deg,min,sec)
    "deg2dms " f(flt,7) " -> " d(deg) "." d(min) "." d(sec) "\n"
    dms2deg(deg,min,sec,flt)
    "dms2deg " d(deg) "." d(min) "." d(sec) " -> " f(flt,7) nl()
    deg2dms(flt,deg,min,sec)
    "deg2dms " f(flt,7) " -> " d(deg) "." d(min) "." d(sec) "\n"

    /*  FLOAT spdist(FLOAT lat0, FLOAT long0, FLOAT lat1, FLOAT long1) */
    set(dst,spdist(36.1599412, -115.1398296,41.8781136,-87.6297982))
    "Distance from 36.1599412, -115.1398296 to 41.8781136,-87.6297982 is "
    f(dst,2) " km" nl()
    "   or " f(mul(dst,0.62137),2) nl()
    /* google says its 1747 miles if you stay on the roads.*/

}
