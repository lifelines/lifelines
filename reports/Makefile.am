# This makefile is for the lifelines reports

AUTOMAKE_OPTIONS = no-dependencies

SUBDIRS = desc-tex2 novel pedtex ps-fan st

# LL_REPORTS is to hold the actual report files
# (included files go in a different target below)
LL_REPORTS = \
	2ppage.ll 4gen1.ll 6gen1.ll 8gen1.ll afg.ll \
	addsour.ll af-import.ll afn_match.ll ahnenliste.ll ahnentafel.ll\
	alive.ll all_anc_lines.ll allfam10c.ll alllines.ll              \
	altern.ll anc2_ged.ll ancestors2.ll anniver.ll                  \
	bday_cover.ll bias.ll  bkdes16-1.ll book-latex.ll 		\
	browser.ll burial_index.ll BW_descendants.ll cid.ll connect2.ll	\
	cgi_html.li common.ll cons.ll cont.ll count_dup.ll 		\
	count_anc.ll count_desc.ll count_paternal_desc.ll cousins.ll	\
	coverage.ll cron.ll dates.ll db_summary.ll desc-henry.ll	\
	desc_ged.ll descged.ll desc_html.ll desc-tree.ll divorce.li	\
	drep2.ll d-rtf.ll dump-ances.ll dump_html.ll dump_html_sd.ll	\
	eol.ll est_life_span.ll extract_gedcom.ll extract_html.ll	\
	extract_set.li fam10c.ll fam16rn1.ll fam_ged.ll famgroup.ll	\
	fami-grps.ll                                                    \
	familycheck.ll familyisfm1.ll famrep.ll famtree1.ll             \
	fileindex.ll                                                    \
	fdesc.ll find.ll findmissing.ll fix_nameplac.ll			\
	formatted_gedcom.ll gedall.ll gedlist.ll gedlod.ll gedn.ll	\
	gedtags.ll ged_write.li genancc.ll                              \
	genancc1.ll gender_order.ll gendex.ll                           \
	genetics.ll genetics2.ll getbaptism.li givens_gender.ll         \
	givens_gender_finder.ll grand.ll                                \
	hasnotes1.ll hp-anc.ll htmlahnen.ll                             \
	html.dn.ll htmlfam.ll html.ll igi-filter.ll igi-import.ll       \
	igi-merge.ll igi-search.ll index1.ll index_html.ll index_mm.ll  \
	interestset.li indiv.ll infant_mortality.ll ldsgedcom.li        \
	line.ll listsour.ll ll2html.ll ll2visio.ll lldb.ll              \
	longlines.ll maritalinfo.ll marriages.ll marriages1.ll menu.ll  \
	name2html.ll                                                    \
	namefreq.ll names_freq.ll namesformat1.ll net-ped.ll            \
        nonpatronymics.ll outsources.li paf-export.ll                   \
	pafcompat.ll partition.ll pdesc.ll paf-import.ll ped.ll         \
	pedigreel.ll pedigree_html.ll pedigreelhs.ll pedigree.ll        \
        places.ll pointers.ll prompt.li ps-anc.ll                       \
	ps-circle.ll ps-pedigree.ll refn.ll reg_html.ll                 \
	register-rtf.ll register-tex.ll register1.ll register1-dot.ll	\
	regvital.ll relate.ll related_spouses.ll relation.ll relink.ll  \
	rfc.ll rllgen.ll rootset.ll rslgen.ll rtflib.li sealing_line.ll \
	search_source.ll                                                \
	select.ll sgsgen.ll shorten.li showlines1.ll simpleged.ll 	\
	soundex1.ll soundex-isfm.ll sour.li                             \
	sour2.li sources.ll sources_bib.ll                              \
	span.ll src.ll src-rtf.ll ssdi_aid.ll ssdi-import.ll            \
	ssdi-search-list.ll stats.ll surname1.ll                        \
	tinytafel1.ll tools.li                                          \
	tree_density.ll ttable.ll upl2.li verify.ll wife.ll             \
	xml-dtd.ll xmlize.ll zombies.ll

# OTHER_REPORTS is to hold included files besides actual report files
# (eg, supporting files, graphics, included files)
OTHER_REPORTS = boc.gif ll.png gen_index bib2html.c bury.c              \
        ps-pedigree.ps tree.tex

# LL_SUBREPORTS is to hold reports that live in subdirectories.
# They are built and installed by the makefiles in their respective
# directories, but are included here as a separate target so that they
# can be included in the generated directory.
LL_SUBREPORTS = \
	desc-tex2/desc-tex.ll desc-tex2/desc-tex2.ll                    \
	novel/novel.ll                                                  \
	pedtex/pedtex.ll ps-fan/ps-fan.ll                               \
	st/st_all.ll

pkg_REPORTS = $(LL_REPORTS) $(OTHER_REPORTS)
pkg_REPORTS_MISC = index.html CREDIT README

EXTRA_DIST = $(pkg_REPORTS) \
	     $(pkg_REPORTS_MISC)

dist_pkgdata_DATA = $(EXTRA_DIST)

CLEANFILES = index.html

PERL = perl
index.html: $(LL_REPORTS) $(LL_SUBREPORTS)
	-$(PERL) $(top_srcdir)/reports/gen_index $^
