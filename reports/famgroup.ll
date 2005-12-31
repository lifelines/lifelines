/*
 * @progname       famgroup.ll
 * @version        1.1
 * @author         Kris Stanton  <kriss@medianet.com>
 * @category       
 * @output         TeX
 * @description
 *
 *   Family Group Sheet for LifeLines
 *
 *   Minor fixes by Patrick Texier 12/28/2005
 *
 *   The output is in LaTeX format.  Therefore, the name of the output file
 *   should end in ".tex".  To print (assuming the name of the output file is
 *   "out.tex"):
 *       latex out
 *       dvips out -o out.ps
 *       lpr out.ps
 *  or if you have it, you can generate a pdf with
 *       pdflatex out
 */

proc main ()
{
        getfam(fam)
        dayformat(0)
        monthformat(6)
        dateformat(0)
        set(tday, gettoday())
        set (nl,nl())
        set(h,husband(fam))
        set(w,wife(fam))
        col(0) "\\documentclass[landscape]\{article\}"
        col(0) "\\setlength\{\\topmargin\}\{-1.3in\}"
        col(0) "\\setlength\{\\oddsidemargin\}\{-.8in\}"
        col(0) "\\setlength\{\\evensidemargin\}\{-.8in\}"
        col(0) "\\setlength\{\\textwidth\}\{11in\}"
        col(0) "\\setlength\{\\textheight\}\{9in\}"
        col(0) "\\pagestyle\{empty\}"
        col(0) "\\begin\{document\}"
        col(0) "\\begin{center}"
        col(0) "\\bfseries \\Large Family Group Sheet"
        col(0) "\\end{center}" nl nl
        col(0) "\\begin\{tabular\}\{lp\{7.85in\}\}"
        col(0) "\\bfseries \\Large Husband's Name & \\Large "
        col(0) fullname(h,1,1,50) " (\\#"key(h)") \\\\ \\cline{2-2}"
        col(0) "\\end{tabular}" nl nl

        set(evt, birth(h))
        col(0) "\\begin{tabular}{p{.25in}p{.87in}p{3.75in}p{.4in}p{3.75in}}"
        col(0) "& When Born & " stddate(evt) " & Where & "
        place(evt) " \\\\ \\cline{3-3} \\cline{5-5}"

        set(evt, death(h))
        col(0) "& When Died & " stddate(evt) " & Where & "
        place(evt)" \\\\ \\cline{3-3} \\cline{5-5}"

        set(evt, burial(h))
        col(0) "& When Buried & " stddate(evt) " & Where & "
        place(evt) " \\\\ \\cline{3-3} \\cline{5-5}"

        set(evt, marriage(fam))
        col(0) "& When Married & " stddate(evt) " & Where & "
        place(evt)" \\\\ \\cline{3-3} \\cline{5-5}"
        col(0) "\\end{tabular}" nl nl

        col(0) "\\begin{tabular}{p{.25in}lp{7.84in}}"
        col(0) "& Other Wives (if any) & "
        spouses (h, sname, famname, number) {
                if (ne(w,sname)) {
                        "$\\triangleright$ " name(sname)
                        " \\hspace{.1in} "
                        }
                }
        " \\\\ \\cline{3-3}"
        col(0) "\\end{tabular}" nl nl

        col(0) "\\begin{tabular}{p{.25in}p{.87in}p{3.75in}p{.68in}p{3.48in}}"
        col(0) "& His Father & " name(father(h))
        if (father(h)) { " (\\#" key(father(h))")" }
        " & His Mother & " name(mother(h))
        if (father(h)) { " (\\#" key(mother(h))")" }
        " \\\\ \\cline{3-3} \\cline{5-5}"
        col(0) "\\end{tabular}" nl nl

        col(0) "\\vspace{.1in}"
        col(0) "\\begin{tabular}{lp{7.44in}}"
        col(0) "\\bfseries \\Large Wife's Maiden Name & \\Large "
        fullname(w,1,1,50) " (\\#"key(w)")" " \\\\ \\cline{2-2}"
        col(0) "\\end{tabular}" nl nl

        set(evt, birth(w))
        col(0) "\\begin{tabular}{p{.25in}p{.87in}p{3.75in}p{.4in}p{3.75in}}"
        col(0) "& When Born & " stddate(evt) " & Where & "
        place(evt)" \\\\ \\cline{3-3} \\cline{5-5}"

        set(evt, death(w))
        col(0) "& When Died & " stddate(evt) " & Where & "
        place(evt)" \\\\ \\cline{3-3} \\cline{5-5}"

        set(evt, burial(w))
        col(0) "& When Buried & " stddate(evt) " & Where & "
        place(evt) " \\\\ \\cline{3-3} \\cline{5-5}"
        col(0) "\\end{tabular}" nl nl

        col(0) "\\begin{tabular}{p{.25in}lp{7.6in}}"
        col(0) "& Other Husbands (if any) & "
        spouses (w, sname, famname, number) {
                if (ne(h,sname)) {
                        "$\\triangleright$ " name(sname)
                        "\\hspace{.1in} "
                        }
                }
        " \\\\ \\cline{3-3}"
        col(0) "\\end{tabular}" nl nl

        col(0) "\\begin{tabular}{p{.25in}p{.87in}p{3.75in}p{.7in}p{3.44in}}"
        col(0) "& Her Father & " name(father(w))
        if (father(w)) { " (\\#" key(father(w))")" }
        " & Her Mother &" name(mother(w))
        if (mother(w)) { " (\\#" key(mother(w))")" }
        " \\\\ \\cline{3-3} \\cline{5-5}"
        col(0) "\\end{tabular}" nl nl

        col(0) "\\vspace{.1in}"
        col(0) "\\scriptsize"
        col(0) "\\begin{tabular}{c|p{2.15in}|cp{.6in}p{.25in}|p{2in}|cp{.6in}p{.25in}|p{2.1in}} \\hline \\hline"
        col(0) "M/F & Children & "
        col(0) "\\multicolumn{3}{c|}{When Born} & Where Born & "
        col(0) "\\multicolumn{3}{c|}{When Died} & Married \\\\ "
        col(0) "& (in order of birth) & \\centering Day & \\centering Month & \\centering Year &"
        col(0) "City/Town, County, State/Country & "
        col(0) "\\centering Day & \\centering Month & \\centering Year & \\\\ \\hline \\hline"

        children(fam, child, num) {
                set(ns, nspouses(child))
                families(child, fvar, svar, no) {
                        if(eq(1,ns)) {
                                col(0) "& (\\#"key(child)") & & & & & & & &\\small Date: "
                                stddate(marriage(fvar)) " \\\\"
                                }
                        if(and(gt(ns,1),eq(no,1))) {
                                col(0) "& (\\#"key(child)") & & & & & & & &\\small Date: "
                                stddate(marriage(fvar)) " $\\dagger$ \\\\"
                                }
                        }
                if(eq(0,ns)) {
                        col(0) "& (\\#"key(child)") & & & & & & & & \\small Date: \\\\ "
                        }

                extractdate(birth(child), ddy, mmo, yyr)
                col(0) "\\small " sex(child) "& \\small " d(num)" \\hspace{.1in}"
                givens(child) nl
                " & \\centering \\small " if(ne(ddy,0)) {d(ddy)} " & \\centering \\small "
                if(eq(mmo,1)){ "January" }
                if(eq(mmo,2)){ "February" }
                if(eq(mmo,3)){ "March" }
                if(eq(mmo,4)){ "April" }
                if(eq(mmo,5)){ "May" }
                if(eq(mmo,6)){ "June" }
                if(eq(mmo,7)){ "July" }
                if(eq(mmo,8)){ "August" }
                if(eq(mmo,9)){ "September" }
                if(eq(mmo,10)){ "October" }
                if(eq(mmo,11)){ "November" }
                if(eq(mmo,12)){ "December" }
                " & \\centering \\small " if(ne(yyr,0)) {d(yyr)}
                " & \\small " place(birth(child))
		if(death(child)) {
                	extractdate(death(child), ddy, mmo, yyr)
		}
		else {
			set(ddy, 0)
			set(mmo, 0)
			set(yyr, 0)
		}
                col(0) " & \\centering \\small " if(ne(ddy,0)){d(ddy)}
                " & \\small \\centering "
                if(eq(mmo,1)){ "January" }
                if(eq(mmo,2)){ "February" }
                if(eq(mmo,3)){ "March" }
                if(eq(mmo,4)){ "April" }
                if(eq(mmo,5)){ "May" }
                if(eq(mmo,6)){ "June" }
                if(eq(mmo,7)){ "July" }
                if(eq(mmo,8)){ "August" }
                if(eq(mmo,9)){ "September" }
                if(eq(mmo,10)){ "October" }
                if(eq(mmo,11)){ "November" }
                if(eq(mmo,12)){ "December" }
                " & \\centering \\small " if(ne(yyr,0)){d(yyr)}
                " & \\small To: "
                families(child, fvar, svar, no) {
                        if(and(gt(ns,0),eq(no,1))) {
                                name(svar) " \\\\ \\hline "}
                        }
                if(eq(0,ns)) {
                        " \\\\ \\hline "}
                }
        set(left, sub(14, nchildren(fam)))
        while(gt(left, 0)) {
                col(0) "& & & & & & & & &\\small Date: \\\\"
                col(0) "&\\small " d(sub(15,left)) " & & & & & & & &\\small To: \\\\ \\hline"
                set(left, sub(left,1))
                }
        col(0) "\\end{tabular}" nl nl
        col(0) "\\hspace{8in} \\scriptsize $\\dagger =$ more than one marriage"
        col(0) "\\end{document}" nl
}

/* End of Report */
