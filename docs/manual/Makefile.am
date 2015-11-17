# This makefile creates the docs/manual directory

# AUTOMAKE_OPTIONS = no-dependencies

###################
# Manuals
###################

MANUALS_en =	ll-reportmanual.html \
		ll-reportmanual.pdf \
		ll-reportmanual.txt \
		ll-userguide.html \
		ll-userguide.pdf \
		ll-userguide.txt \
		ll-devguide.html \
		ll-index.html \
		ll-index.sv.html

MANUALS_sv =	ll-reportmanual.sv.html \
		ll-userguide.sv.html

MANUALS =	$(MANUALS_en) $(MANUALS_sv)

IMAGES =	listops.jpg

MANUALSSRC =	ll-reportmanual.xml \
		ll-userguide.xml \
		ll-reportmanual.sv.xml \
		ll-userguide.sv.xml \
		ll-devguide.xml

###################
# Rules
###################

EXTRA_DIST = $(MANUALS) $(IMAGES)

CONFIG_CLEAN_FILES = $(MANUALS)

docdir = $(datarootdir)/doc/lifelines
doc_DATA = $(MANUALSSRC) $(MANUALS) 
dist_pkgdata_DATA = $(MANUALS)

all: @DOCS_TARGET@

.xml.pdf:
	-jw -f docbook -b pdf $^

.xml.html:
	-jw -f docbook -b html -V nochunks -d $(srcdir)/ll_html.dsl $^

.html.txt:
	if [ -f /usr/bin/lynx ] ; then \
	/usr/bin/lynx -force_html -dump -nolist -width=76 $^ > $@ ;\
	fi
