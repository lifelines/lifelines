#
# lifelines.spec - RPM configuration file for Lifelines
#
# To generate RPMs, place the tar.gz in /usr/src/packages/SOURCES/ and
# (or /usr/src/redhat/SOURCES as appropriate)
# then "rpm -ba lifelines.spec"
#

%define lifelines_version       3.0.13

Name: lifelines
Summary: lifelines genealogy program
Version: %{lifelines_version}
Release: 1
Copyright: MIT
Group: Utilities/System
Source:         http://download.sourceforge.net/lifelines/lifelines-%{lifelines_version}.tar.gz
URL:            http://lifelines.sourceforge.net/
Packager:       Marc Nozell <marc@nozell.com>
Provides:       lifelines
%description
This program allows the tracking of genealogical information.  The lifelines
reports are the power of the system.

%prep 
%setup

%build
#make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"
sh -c ./configure 
make 

%install
mkdir -p /usr/local/share/lifelines/reports
install -s -m 755 -o 0 -g 0 liflines/llines /usr/local/bin/llines
install  -m 755 -o 0 -g 0 docs/llines.1 /usr/local/man/man1/llines.1
install  -m 755 -o 0 -g 0 reports/CREDIT /usr/local/share/lifelines/reports/CREDIT
install  -m 755 -o 0 -g 0 reports/2ppage.ll /usr/local/share/lifelines/reports/2ppage.ll
install  -m 755 -o 0 -g 0 reports/4gen1.ll /usr/local/share/lifelines/reports/4gen1.ll
install  -m 755 -o 0 -g 0 reports/6gen1.ll /usr/local/share/lifelines/reports/6gen1.ll
install  -m 755 -o 0 -g 0 reports/8gen1.ll /usr/local/share/lifelines/reports/8gen1.ll
install  -m 755 -o 0 -g 0 reports/ahnentafel.ll /usr/local/share/lifelines/reports/ahnentafel.ll
install  -m 755 -o 0 -g 0 reports/alive.ll /usr/local/share/lifelines/reports/alive.ll
install  -m 755 -o 0 -g 0 reports/alllines.ll /usr/local/share/lifelines/reports/alllines.ll
install  -m 755 -o 0 -g 0 reports/alllines.sgml.ll /usr/local/share/lifelines/reports/alllines.sgml.ll
install  -m 755 -o 0 -g 0 reports/ancestors2.ll /usr/local/share/lifelines/reports/ancestors2.ll
install  -m 755 -o 0 -g 0 reports/bias.ll /usr/local/share/lifelines/reports/bias.ll
install  -m 755 -o 0 -g 0 reports/bkdes16-1.ll /usr/local/share/lifelines/reports/bkdes16-1.ll
install  -m 755 -o 0 -g 0 reports/bkdes16-1.ll /usr/local/share/lifelines/reports/book-latex.ll
install  -m 755 -o 0 -g 0 reports/cid.ll /usr/local/share/lifelines/reports/cid.ll
install  -m 755 -o 0 -g 0 reports/connect2.ll /usr/local/share/lifelines/reports/connect2.ll
install  -m 755 -o 0 -g 0 reports/count_anc.ll /usr/local/share/lifelines/reports/count_anc.ll
install  -m 755 -o 0 -g 0 reports/count_desc.ll /usr/local/share/lifelines/reports/count_desc.ll
install  -m 755 -o 0 -g 0 reports/count_paternal_desc.ll /usr/local/share/lifelines/reports/count_paternal_desc.ll
install  -m 755 -o 0 -g 0 reports/cousins.ll /usr/local/share/lifelines/reports/cousins.ll
install  -m 755 -o 0 -g 0 reports/coverage.ll /usr/local/share/lifelines/reports/coverage.ll
install  -m 755 -o 0 -g 0 reports/dates.ll /usr/local/share/lifelines/reports/dates.ll
install  -m 755 -o 0 -g 0 reports/desc-henry.ll /usr/local/share/lifelines/reports/desc-henry.ll
install  -m 755 -o 0 -g 0 reports/exercise.ll /usr/local/share/lifelines/reports/exercise.ll
install  -m 755 -o 0 -g 0 reports/fam10c.ll /usr/local/share/lifelines/reports/fam10c.ll
install  -m 755 -o 0 -g 0 reports/fam16rn1.ll /usr/local/share/lifelines/reports/fam16rn1.ll
install  -m 755 -o 0 -g 0 reports/familyisfm1.ll /usr/local/share/lifelines/reports/familyisfm1.ll
install  -m 755 -o 0 -g 0 reports/famrep1.ll /usr/local/share/lifelines/reports/famrep1.ll
install  -m 755 -o 0 -g 0 reports/famrep3.ll /usr/local/share/lifelines/reports/famrep3.ll
install  -m 755 -o 0 -g 0 reports/famrep6.ll /usr/local/share/lifelines/reports/famrep6.ll
install  -m 755 -o 0 -g 0 reports/famtree1.ll /usr/local/share/lifelines/reports/famtree1.ll
install  -m 755 -o 0 -g 0 reports/fdesc.ll /usr/local/share/lifelines/reports/fdesc.ll
install  -m 755 -o 0 -g 0 reports/find.ll /usr/local/share/lifelines/reports/find.ll
install  -m 755 -o 0 -g 0 reports/findmissing.ll /usr/local/share/lifelines/reports/findmissing.ll
install  -m 755 -o 0 -g 0 reports/fix_nameplac.ll /usr/local/share/lifelines/reports/fix_nameplac.ll
install  -m 755 -o 0 -g 0 reports/formatted_gedcom.ll /usr/local/share/lifelines/reports/formatted_gedcom.ll
install  -m 755 -o 0 -g 0 reports/genancc1.ll /usr/local/share/lifelines/reports/genancc1.ll
install  -m 755 -o 0 -g 0 reports/gender_order.ll /usr/local/share/lifelines/reports/gender_order.ll
install  -m 755 -o 0 -g 0 reports/genetics.ll /usr/local/share/lifelines/reports/genetics.ll
install  -m 755 -o 0 -g 0 reports/givens_gender_finder.ll /usr/local/share/lifelines/reports/givens_gender_finder.ll
install  -m 755 -o 0 -g 0 reports/hasnotes1.ll /usr/local/share/lifelines/reports/hasnotes1.ll
install  -m 755 -o 0 -g 0 reports/igi-merge.ll /usr/local/share/lifelines/reports/igi-merge.ll
install  -m 755 -o 0 -g 0 reports/index1.ll /usr/local/share/lifelines/reports/index1.ll
install  -m 755 -o 0 -g 0 reports/index_mm.ll /usr/local/share/lifelines/reports/index_mm.ll
install  -m 755 -o 0 -g 0 reports/indiv2.r.ll /usr/local/share/lifelines/reports/indiv2.r.ll
install  -m 755 -o 0 -g 0 reports/indiv3.ll /usr/local/share/lifelines/reports/indiv3.ll
install  -m 755 -o 0 -g 0 reports/infant_mortality.ll /usr/local/share/lifelines/reports/infant_mortality.ll
install  -m 755 -o 0 -g 0 reports/least_related.ll /usr/local/share/lifelines/reports/least_related.ll
install  -m 755 -o 0 -g 0 reports/longlines.ll /usr/local/share/lifelines/reports/longlines.ll
install  -m 755 -o 0 -g 0 reports/marriages1.ll /usr/local/share/lifelines/reports/marriages1.ll
install  -m 755 -o 0 -g 0 reports/menu.ll /usr/local/share/lifelines/reports/menu.ll
install  -m 755 -o 0 -g 0 reports/namefreq.ll /usr/local/share/lifelines/reports/namefreq.ll
install  -m 755 -o 0 -g 0 reports/namefreq2.ll /usr/local/share/lifelines/reports/namefreq2.ll
install  -m 755 -o 0 -g 0 reports/names_freq.ll /usr/local/share/lifelines/reports/names_freq.ll
install  -m 755 -o 0 -g 0 reports/namesformat1.ll /usr/local/share/lifelines/reports/namesformat1.ll
install  -m 755 -o 0 -g 0 reports/nonpatronymics.ll /usr/local/share/lifelines/reports/nonpatronymics.ll
install  -m 755 -o 0 -g 0 reports/pafcompat.ll /usr/local/share/lifelines/reports/pafcompat.ll
install  -m 755 -o 0 -g 0 reports/partition.ll /usr/local/share/lifelines/reports/partition.ll
install  -m 755 -o 0 -g 0 reports/pdesc1.ll /usr/local/share/lifelines/reports/pdesc1.ll
install  -m 755 -o 0 -g 0 reports/pdesc2.r.ll /usr/local/share/lifelines/reports/pdesc2.r.ll
install  -m 755 -o 0 -g 0 reports/pdesc3.ll /usr/local/share/lifelines/reports/pdesc3.ll
install  -m 755 -o 0 -g 0 reports/pdesc4.ll /usr/local/share/lifelines/reports/pdesc4.ll
install  -m 755 -o 0 -g 0 reports/pedigreel.ll /usr/local/share/lifelines/reports/pedigreel.ll
install  -m 755 -o 0 -g 0 reports/places.ll /usr/local/share/lifelines/reports/places.ll
install  -m 755 -o 0 -g 0 reports/ps-anc2.ll /usr/local/share/lifelines/reports/ps-anc2.ll
install  -m 755 -o 0 -g 0 reports/ps-anc5.ll /usr/local/share/lifelines/reports/ps-anc5.ll
install  -m 755 -o 0 -g 0 reports/ps-anc6.ll /usr/local/share/lifelines/reports/ps-anc6.ll
install  -m 755 -o 0 -g 0 reports/ps-anc7.ll /usr/local/share/lifelines/reports/ps-anc7.ll
install  -m 755 -o 0 -g 0 reports/ps-anc8.ll /usr/local/share/lifelines/reports/ps-anc8.ll
install  -m 755 -o 0 -g 0 reports/register-tex.ll /usr/local/share/lifelines/reports/register-tex.ll
install  -m 755 -o 0 -g 0 reports/register1.ll /usr/local/share/lifelines/reports/register1.ll
install  -m 755 -o 0 -g 0 reports/regvital.ll /usr/local/share/lifelines/reports/regvital.ll
install  -m 755 -o 0 -g 0 reports/regvital1.ll /usr/local/share/lifelines/reports/regvital1.ll
install  -m 755 -o 0 -g 0 reports/relate.ll /usr/local/share/lifelines/reports/relate.ll
install  -m 755 -o 0 -g 0 reports/related_spouses.ll /usr/local/share/lifelines/reports/related_spouses.ll
install  -m 755 -o 0 -g 0 reports/relation.ll /usr/local/share/lifelines/reports/relation.ll
install  -m 755 -o 0 -g 0 reports/rllgen.ll /usr/local/share/lifelines/reports/rllgen.ll
install  -m 755 -o 0 -g 0 reports/rslgen.ll /usr/local/share/lifelines/reports/rslgen.ll
install  -m 755 -o 0 -g 0 reports/select.li /usr/local/share/lifelines/reports/select.li
install  -m 755 -o 0 -g 0 reports/showlines1.ll /usr/local/share/lifelines/reports/showlines1.ll
install  -m 755 -o 0 -g 0 reports/simpleged.ll /usr/local/share/lifelines/reports/simpleged.ll
install  -m 755 -o 0 -g 0 reports/soundex-isfm.ll /usr/local/share/lifelines/reports/soundex-isfm.ll
install  -m 755 -o 0 -g 0 reports/soundex1.ll /usr/local/share/lifelines/reports/soundex1.ll
install  -m 755 -o 0 -g 0 reports/sources.ll /usr/local/share/lifelines/reports/sources.ll
install  -m 755 -o 0 -g 0 reports/stats.ll /usr/local/share/lifelines/reports/stats.ll
install  -m 755 -o 0 -g 0 reports/surname1.ll /usr/local/share/lifelines/reports/surname1.ll
install  -m 755 -o 0 -g 0 reports/timeline1.ll /usr/local/share/lifelines/reports/timeline1.ll
install  -m 755 -o 0 -g 0 reports/timeline2.ll /usr/local/share/lifelines/reports/timeline2.ll
install  -m 755 -o 0 -g 0 reports/tinytafel1.ll /usr/local/share/lifelines/reports/tinytafel1.ll
install  -m 755 -o 0 -g 0 reports/tinytafel2.ll /usr/local/share/lifelines/reports/tinytafel2.ll
install  -m 755 -o 0 -g 0 reports/tree.tex /usr/local/share/lifelines/reports/tree.tex
install  -m 755 -o 0 -g 0 reports/verify.ll /usr/local/share/lifelines/reports/verify.ll

%files
%doc README ChangeLog NEWS AUTHORS LICENSE docs/lifelines.sgml docs/quickref.pdf docs/quickref.ps

/usr/local/bin/llines
/usr/local/man/man1/llines.1
/usr/local/share/lifelines/reports/CREDIT
/usr/local/share/lifelines/reports/2ppage.ll
/usr/local/share/lifelines/reports/4gen1.ll
/usr/local/share/lifelines/reports/6gen1.ll
/usr/local/share/lifelines/reports/8gen1.ll
/usr/local/share/lifelines/reports/ahnentafel.ll
/usr/local/share/lifelines/reports/alive.ll
/usr/local/share/lifelines/reports/alllines.ll
/usr/local/share/lifelines/reports/alllines.sgml.ll
/usr/local/share/lifelines/reports/ancestors2.ll
/usr/local/share/lifelines/reports/bias.ll
/usr/local/share/lifelines/reports/bkdes16-1.ll
/usr/local/share/lifelines/reports/book-latex.ll
/usr/local/share/lifelines/reports/cid.ll
/usr/local/share/lifelines/reports/connect2.ll
/usr/local/share/lifelines/reports/count_anc.ll
/usr/local/share/lifelines/reports/count_desc.ll
/usr/local/share/lifelines/reports/count_paternal_desc.ll
/usr/local/share/lifelines/reports/cousins.ll
/usr/local/share/lifelines/reports/coverage.ll
/usr/local/share/lifelines/reports/dates.ll
/usr/local/share/lifelines/reports/desc-henry.ll
/usr/local/share/lifelines/reports/exercise.ll
/usr/local/share/lifelines/reports/fam10c.ll
/usr/local/share/lifelines/reports/fam16rn1.ll
/usr/local/share/lifelines/reports/familyisfm1.ll
/usr/local/share/lifelines/reports/famrep1.ll
/usr/local/share/lifelines/reports/famrep3.ll
/usr/local/share/lifelines/reports/famrep6.ll
/usr/local/share/lifelines/reports/famtree1.ll
/usr/local/share/lifelines/reports/fdesc.ll
/usr/local/share/lifelines/reports/find.ll
/usr/local/share/lifelines/reports/findmissing.ll
/usr/local/share/lifelines/reports/fix_nameplac.ll
/usr/local/share/lifelines/reports/formatted_gedcom.ll
/usr/local/share/lifelines/reports/genancc1.ll
/usr/local/share/lifelines/reports/gender_order.ll
/usr/local/share/lifelines/reports/genetics.ll
/usr/local/share/lifelines/reports/givens_gender_finder.ll
/usr/local/share/lifelines/reports/hasnotes1.ll
/usr/local/share/lifelines/reports/igi-merge.ll
/usr/local/share/lifelines/reports/index1.ll
/usr/local/share/lifelines/reports/index_mm.ll
/usr/local/share/lifelines/reports/indiv2.r.ll
/usr/local/share/lifelines/reports/indiv3.ll
/usr/local/share/lifelines/reports/infant_mortality.ll
/usr/local/share/lifelines/reports/least_related.ll
/usr/local/share/lifelines/reports/longlines.ll
/usr/local/share/lifelines/reports/marriages1.ll
/usr/local/share/lifelines/reports/menu.ll
/usr/local/share/lifelines/reports/namefreq.ll
/usr/local/share/lifelines/reports/namefreq2.ll
/usr/local/share/lifelines/reports/names_freq.ll
/usr/local/share/lifelines/reports/namesformat1.ll
/usr/local/share/lifelines/reports/nonpatronymics.ll
/usr/local/share/lifelines/reports/pafcompat.ll
/usr/local/share/lifelines/reports/partition.ll
/usr/local/share/lifelines/reports/pdesc1.ll
/usr/local/share/lifelines/reports/pdesc2.r.ll
/usr/local/share/lifelines/reports/pdesc3.ll
/usr/local/share/lifelines/reports/pdesc4.ll
/usr/local/share/lifelines/reports/pedigreel.ll
/usr/local/share/lifelines/reports/places.ll
/usr/local/share/lifelines/reports/ps-anc2.ll
/usr/local/share/lifelines/reports/ps-anc5.ll
/usr/local/share/lifelines/reports/ps-anc6.ll
/usr/local/share/lifelines/reports/ps-anc7.ll
/usr/local/share/lifelines/reports/ps-anc8.ll
/usr/local/share/lifelines/reports/register-tex.ll
/usr/local/share/lifelines/reports/register1.ll
/usr/local/share/lifelines/reports/regvital.ll
/usr/local/share/lifelines/reports/regvital1.ll
/usr/local/share/lifelines/reports/relate.ll
/usr/local/share/lifelines/reports/related_spouses.ll
/usr/local/share/lifelines/reports/relation.ll
/usr/local/share/lifelines/reports/rllgen.ll
/usr/local/share/lifelines/reports/rslgen.ll
/usr/local/share/lifelines/reports/select.li
/usr/local/share/lifelines/reports/showlines1.ll
/usr/local/share/lifelines/reports/simpleged.ll
/usr/local/share/lifelines/reports/soundex-isfm.ll
/usr/local/share/lifelines/reports/soundex1.ll
/usr/local/share/lifelines/reports/sources.ll
/usr/local/share/lifelines/reports/stats.ll
/usr/local/share/lifelines/reports/surname1.ll
/usr/local/share/lifelines/reports/timeline1.ll
/usr/local/share/lifelines/reports/timeline2.ll
/usr/local/share/lifelines/reports/tinytafel1.ll
/usr/local/share/lifelines/reports/tinytafel2.ll
/usr/local/share/lifelines/reports/verify.ll
