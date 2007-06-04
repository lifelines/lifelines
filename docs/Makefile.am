# This makefile creates the documentation directory

# AUTOMAKE_OPTIONS = no-dependencies

###################
# Manuals
###################

MANUALS_en =	ll-reportmanual.html ll-reportmanual.pdf \
		ll-reportmanual.txt \
		ll-userguide.html ll-userguide.pdf \
		ll-userguide.txt ll-devguide.html \
		ll-index.html ll-index.sv.html

MANUALS_sv =	ll-reportmanual.sv.html \
		ll-userguide.sv.html

MANUALS =	$(MANUALS_en) $(MANUALS_sv)

IMAGES =	listops.jpg

MANUALSSRC =	ll-reportmanual.xml \
		ll-userguide.xml \
		ll-devguide.xml

###################
# Obsolete Manuals
###################

OLDGENFILES =	lifelines.html lifelines.txt \
		lifelines.pdf lifelines.ps
OLDSRCFILES =	lifelines.sgml
OLDFILES =	quickref.pdf quickref.ps

###################
# Man Pages Manuals
###################

MANPAGES =   btedit.1 dbverify.1 llines.1 llexec.1

###################
# Rules
###################

EXTRA_DIST = $(MANUALSSRC) $(MANUALS) $(MANPAGES) $(IMAGES) lifelines.vim
man1_MANS = $(MANPAGES)

CONFIG_CLEAN_FILES = $(OLDGENFILES) $(OLDSRCFILES) $(OLDFILES) $(MANUALS)

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
