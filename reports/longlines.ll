/*
 * @progname       longlines.ll
 * @version        2.0
 * @author         Chandler
 * @category       
 * @output         Text
 * @description    

Find the maximal-length male and female lineages in the database.
Optionally, find the maximal-length lineage through a specified ancestor.
 
longlines

Version 1 - 1994 May 19 - John F. Chandler
Version 2 - 2000 May 4 - John F. Chandler

This program works only with LifeLines.

*/
global(len)     /* current lineage length */
global(lenmax)  /* longest lineage found */
global(ends)    /* keys of last persons */
global(linsex)  /* sex of lineage desired */

proc main(){
getindi(indi,"Enter specific ancestor, if any, whose longest line you want:")
if(indi) {
        "Longest descent from " name(indi) " (" key(indi) ")\n\n"
        set(linsex,sex(indi))
        set(len,1)
        call getline(indi)
        call dumplines()
} else {
        "Longest lineages in database\n\n   Male"
        call getall("M")
        call dumplines()
        "\n   Female"
        call getall("F")
        call dumplines()
}
}

/* scan all offspring matching the sex of the input person, and
   return the longest lineage(s) from those -- if no matching
   offspring, just return the input person as a lineage */
proc getline(indi)
{
incr(len)
families(indi,fam,spou,num) {
        children(fam,child,numc) {
                if(eq(0,strcmp(linsex,sex(child)))) {
                        set(found,1)
                        call getline(child)
                }
        }
}
decr(len)
if(and(not(found),ge(len,lenmax))) {
        if(gt(len,lenmax)) {list(ends)}
        enqueue(ends,save(key(indi)))
        set(lenmax,len)
}}

proc getall(this_sex)
{
set(linsex,this_sex)
set(lenmax,0)
print("Starting ", linsex, " ...\n")

/* find all eligible starting points */
/* assume that a nameless person doesn't count */
forindi (indi, num) {
        set(skip,"")
        if(eq(0,strcmp(linsex,"M"))) {set(par,father(indi))}
        else {set(par,mother(indi))}
        if(par) {set(skip,name(par))}
        if(and(eq(0,strcmp(linsex,sex(indi))),eq(0,strcmp("",skip)))) {
                set(len,1)
                call getline(indi)
        }
}}

proc dumplines()
{
/* report results */
"\n   Maximal length " d(lenmax) "\n"
/* dump each lineage, starting with most recent person */
while(end, dequeue(ends)) {
        "\n"
        set(count, lenmax)
        set(line,indi(end))
        while(line) {
                if(eq(count,0)) {"   (extension of the requested line...)\n"}
                decr(count)
                if(eq(0,strcmp(name(line),""))) {"_____"}
                name(line) " (" key(line) ")"
                if(x, birth(line)) {" b. " year(x)}
                if(y, death(line)) {
                        if(x) {","}
                        " d. " year(y)
                }
                "\n"
                if(eq(0,strcmp(linsex,"M"))) {set(line,father(line))}
                else {set(line,mother(line))}
        }
        if(lt(count,0)) {"   (length " d(sub(lenmax,count)) " with extension)\n"}
}}
