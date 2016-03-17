# This makefile is for the lifelines reports

AUTOMAKE_OPTIONS = no-dependencies

# LL_REPORTS is to hold the actual report files
# (included files go in a different target below)
LL_REPORTS = ps-fan.ll

# OTHER_REPORTS is to hold included files besides actual report files
# (eg, supporting files, graphics, included files)
OTHER_REPORTS = ps-fan.ps

pkg_REPORTS = $(LL_REPORTS) $(OTHER_REPORTS)

EXTRA_DIST = $(pkg_REPORTS)

dist_pkgdata_DATA = $(EXTRA_DIST)

subreportdir = $(pkgdatadir)/ps-fan
subreport_DATA = $(EXTRA_DIST)
