# This makefile is for the lifelines reports

AUTOMAKE_OPTIONS = no-dependencies
### pkgdir := $(pkgdatadir)/reports
LL_REPORTS = \
	2ppage.ll 4gen1.ll 6gen1.ll 8gen1.ll \
	ahnentafel.ll alive.ll alllines.ll alllines.sgml.ll	\
	ancestors2.ll bias.ll bkdes16-1.ll book-latex.ll \
	cid.ll connect2.ll	\
	count_anc.ll count_desc.ll count_paternal_desc.ll cousins.ll	\
	coverage.ll dates.ll desc-henry.ll \
	fam10c.ll fam16rn1.ll	\
	familyisfm1.ll famrep1.ll famrep3.ll famrep6.ll famtree1.ll	\
	fdesc.ll find.ll findmissing.ll fix_nameplac.ll	\
	formatted_gedcom.ll genancc1.ll gender_order.ll genetics.ll	\
	givens_gender_finder.ll hasnotes1.ll igi-merge.ll index1.ll	\
	index_mm.ll indiv2.r.ll indiv3.ll infant_mortality.ll	\
	least_related.ll longlines.ll marriages1.ll menu.ll	\
	namefreq.ll namefreq2.ll names_freq.ll namesformat1.ll	\
	nonpatronymics.ll pafcompat.ll partition.ll pdesc1.ll	\
	pdesc2.r.ll pdesc3.ll pdesc4.ll pedigreel.ll places.ll	\
	ps-anc2.ll ps-anc5.ll ps-anc6.ll ps-anc7.ll ps-anc8.ll	\
	register-tex.ll register1.ll regvital.ll regvital1.ll	\
	relate.ll related_spouses.ll relation.ll rllgen.ll rslgen.ll	\
	showlines1.ll simpleged.ll soundex-isfm.ll	\
	soundex1.ll sources.ll stats.ll surname1.ll timeline1.ll	\
	timeline2.ll tinytafel1.ll tinytafel2.ll verify.ll \
	st/st_all.ll st/st_aux.ll st/st_collate.ll \
	st/st_collate_8859-1.ll st/st_collate_UTF-8.ll \
	st/st_convert.ll st/st_date.ll st/st_db.ll \
	st/st_number.ll st/st_string.ll st/st_string_UTF-8.ll
OTHER_REPORTS = select.li boc.gif ll.png gen_index
dist_pkgdata_DATA = index.html
pkg_REPORTS = $(LL_REPORTS) $(OTHER_REPORTS) $(dist_pkgdata_DATA)

EXTRA_DIST = \
	$(pkg_REPORTS) \
	tree.tex \
	CREDIT README
PERL = perl
index.html: $(LL_REPORTS)
	-$(PERL) $(top_srcdir)/reports/gen_index $^
