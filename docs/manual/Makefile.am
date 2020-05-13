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
		ll-devguide.pdf \
		ll-devguide.txt

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

MANUALS_static = index.html \
		 index.sv.html \
		 $(IMAGES)

###################
# Rules
###################

# Install static files and generated manuals, but only when docs are built
if BUILD_DOCS
doc_DATA = $(MANUALS_static) $(MANUALS)
else
doc_DATA =
endif

# We distribute the above, plus sources.
EXTRA_DIST = $(doc_DATA) $(MANUALSSRC)

# By default, documentation cannot be cleaned.
CLEANFILES =

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
