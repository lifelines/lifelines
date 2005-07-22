# This makefile creates the documentation directory

# AUTOMAKE_OPTIONS = no-dependencies

# obsolete manuals: quickref.pdf quickref.ps lifelines.sgml

MANUALSSRC = ll-reportmanual.xml ll-userguide.xml

MANUALS_en =     ll-reportmanual.html ll-reportmanual.pdf \
	     ll-reportmanual.txt listops.jpg \
	     ll-userguide.html ll-userguide.pdf \
	     ll-userguide.txt 

MANUALS_sv = ll-reportmanual.sv.html ll-userguide.sv.html

MANUALS = $(MANUALS_en) $(MANUALS_sv)

MANPAGES =   btedit.1 dbverify.1 llines.1

OLDGENFILES = lifelines.html lifelines.pdf lifelines.ps lifelines.txt

EXTRA_DIST = $(MANUALSSRC) $(MANUALS) $(MANPAGES)
man1_MANS = $(MANPAGES)

CONFIG_CLEAN_FILES = $(OLDGENFILES) $(MANUALS)

docdir = $(datadir)/doc/lifelines
doc_DATA = $(MANUALSSRC) $(MANUALS) 
dist_pkgdata_DATA = $(MANUALS)


# sensible values: empty, simple, fancy
all: @DOCS_TARGET@

.xml.pdf:
	-jw -f docbook -b pdf $^

.xml.html:
	-jw -f docbook -b html -V nochunks $^

.html.txt:
	if [ -f /usr/bin/lynx ] ; then \
	/usr/bin/lynx -force_html -dump -nolist -width=76 $^ > $@ ;\
	fi

