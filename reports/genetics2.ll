/*
 * @progname       genetics2.ll
 * @version        1 of 1995-10-05
 * @author         Alexander Ottl (ottl@informatik.uni-muenchen.de)
 * @category       
 * @output         Text
 * @description

This LifeLines report program computes the degree of blood relatedness
between any two people in a database.

Genetic distance d(A,B) is defined recursively by:
  d(A,A) = 1
  d(A,B) = d(B,A)
  d(A,B) = d(F(A),B) / 2 + d(M(A),B) / 2
with F(A) and M(A) being the father and mother of A.

The recursive procedure computedist() follows that definition.
That's the beauty of recursion.

        by Alexander Ottl (ottl@informatik.uni-muenchen.de)
        Version 1 (5 Oct 1995)

*/

global(R0)
global(R1)

proc main()
{
    getindimsg(A, "First person:")
    getindimsg(B, "Second person:")
    call computedist(A, B)
    print("\nExpected degree of genetic overlap: ",
        d(R0), "/", d(R1), "\n")
}

/* BOOL ancestor( INDI, INDI ) */
func ancestor(A, B)
{
    if(not(strcmp(key(A),key(B)))) {
        return(1)
    }
    families(A, Fam, Spo, Num1) {
        children(Fam, Chl, Num2) {
            if(ancestor(Chl, B)) {
                return(1)
            }
        }
    }
    return(0)
}

/* Actually this should be a function returning a rational number.
   I might use a list, but I chose to use two global variables
   R0 and R1 for the numerator and denominator */
/* VOID computedist( INDI, INDI ) */
proc computedist(A, B)
{
    /* Recursion must terminate some time.
       One's distance to himself is 1/1 */
    if(not(strcmp(key(A),key(B)))) {
        set(R0,1)
        set(R1,1)
    }
    /* If there is a direct line from A down to B, we must work our way
       upwards from B. There must of course then be no line
       from B down to A, but no one is his own ancestor, right? */
    elsif(ancestor(A, B)) {
        /* print("Common ancestor: ", name(A), "\n") */
        call computedist(B, A)
    }
    /* Now we try to work our way upwards through the parents */
    else {
        set(R0,0)
        set(R1,1)
        if(F,father(A)) {
            call computedist(F, B)
            /* Result by half */
            set(R1, mul(2, R1))
        }
        if(M,mother(A)) {
            /* Save previous result */
            set(Res0, R0)
            set(Res1, R1)
            call computedist(M, B)
            /* Result by half */
            set(R1, mul(2, R1))
            /* Adding up with previous result */
            set(common, mul(R1, Res1))
            set(R0, add(mul(R0, Res1), mul(R1, Res0)))
            set(R1, common)
            call normalize()
        }
    }
}

/* This is not an all-purpose normalizing function.
   We expect the denominator R1 to be a power of 2 and
   to be greater than the numerator R0. */
/* VOID normalize(VOID) */
proc normalize()
{
    if(R0) {
        while(not(mod(R0,2))) {
            set(R0, div(R0,2))
            set(R1, div(R1,2))
        }
    }
    else {
        set(R1,1)
    }
}
