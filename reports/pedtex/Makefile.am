# This makefile is for the lifelines reports

AUTOMAKE_OPTIONS = no-dependencies

# LL_REPORTS is to hold the actual report files
# (included files go in a different target below)
LL_REPORTS = pedtex.ll

# OTHER_REPORTS is to hold included files besides actual report files
# (eg, supporting files, graphics, included files)
OTHER_REPORTS = pedtex.readme setup.tex tree.tex

pkg_REPORTS = $(LL_REPORTS) $(OTHER_REPORTS)

EXTRA_DIST = $(pkg_REPORTS)

dist_pkgdata_DATA = $(EXTRA_DIST)

subreportdir = $(pkgdatadir)/pedtex
subreport_DATA = $(EXTRA_DIST)
