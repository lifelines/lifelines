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
		ll-reportmanual.sv.pdf \
		ll-userguide.sv.html \
		ll-userguide.sv.pdf

MANUALS =	$(MANUALS_en) $(MANUALS_sv)

MANUALSSRC_en = ll-reportmanual.xml \
		ll-userguide.xml \
		ll-devguide.xml

MANUALSSRC_sv = ll-reportmanual.sv.xml \
		ll-userguide.sv.xml

MANUALSSRC =	$(MANUALSSRC_en) $(MANUALSSRC_sv) \
		ll-html.xsl

IMAGES =	listops.jpg

MANUALS_static = ll-index.html \
		 ll-index.sv.html \
		 $(IMAGES)

###################
# Rules
###################

CONFIG_CLEAN_FILES = $(MANUALS)

if BUILD_DOCS
dist_doc_DATA = $(MANUALS) $(MANUALS_static)
endif

.xml.pdf:
	@TOOL_DBLATEX@ $^

.xml.html:
	@TOOL_XMLTO@ -x ll-html.xsl xhtml-nochunks $^

.xml.txt:
	@TOOL_XMLTO@ txt $^
