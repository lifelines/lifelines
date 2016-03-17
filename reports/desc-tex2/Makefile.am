# This makefile is for the lifelines reports

AUTOMAKE_OPTIONS = no-dependencies

# LL_REPORTS is to hold the actual report files
# (included files go in a different target below)
LL_REPORTS = desc-tex.ll desc-tex2.ll

# OTHER_REPORTS is to hold included files besides actual report files
# (eg, supporting files, graphics, included files)
OTHER_REPORTS = \
        drtree.tex extree2.tex                                \
	poster2.tex                                           \
        pstricks.con README                                   \
        ex1.tex mysetup.tex poster.doc                        \
        pstricks.pro setup.tex                                \
        drsetup.tex extree1.tex                               \
	poster1.tex                                           \
        poster.tex pstricks.tex tree.tex 

pkg_REPORTS = $(LL_REPORTS) $(OTHER_REPORTS)

subreportdir = $(pkgdatadir)/desc-tex2
subreport_DATA = $(pkg_REPORTS)
dist_subreport_DATA = $(pkg_REPORTS)
