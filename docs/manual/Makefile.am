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

# By default, end-users just install documentation and never clean it.
doc_DATA = $(MANUALS_static) $(MANUALS)
CLEANFILES =

# When creating a distribution, include the sources
dist_doc_DATA = $(doc_DATA) $(MANUALSSRC)

# When using --with-docs, provide the power to destroy and the rules to recreate.
if BUILD_DOCS
CLEANFILES += $(MANUALS)

.xml.pdf:
	@TOOL_DBLATEX@ $^

.xml.html:
	@TOOL_XMLTO@ -x @srcdir@/ll-html.xsl xhtml-nochunks $^

.xml.txt:
	@TOOL_XMLTO@ txt $^
endif
