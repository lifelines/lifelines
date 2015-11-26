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
		ll-devguide.html

MANUALS_sv =	ll-reportmanual.sv.html \
		ll-userguide.sv.html

MANUALS =	$(MANUALS_en) $(MANUALS_sv)

MANUALSSRC_en = ll-reportmanual.xml \
		ll-userguide.xml \
		ll-devguide.xml

MANUALSSRC_sv = ll-reportmanual.sv.xml \
		ll-userguide.sv.xml

MANUALSSRC =	$(MANUALSSRC_en) $(MANUALSSRC_sv)

IMAGES =	listops.jpg

MANUALS_static = ll-index.html \
		 ll-index.sv.html

###################
# Rules
###################

CONFIG_CLEAN_FILES = $(MANUALS)

dist_doc_DATA = $(MANUALS) $(MANUALS_static)

.xml.pdf:
	-jw -f docbook -b pdf $^

.xml.html:
	-jw -f docbook -b html -V nochunks -d $(srcdir)/ll_html.dsl $^

.html.txt:
	if [ -f /usr/bin/lynx ] ; then \
	/usr/bin/lynx -force_html -dump -nolist -width=76 $^ > $@ ;\
	fi
