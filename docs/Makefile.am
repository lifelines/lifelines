# This makefile creates the documentation directory

# AUTOMAKE_OPTIONS = no-dependencies

MANUALS =     ll-reportmanual.html ll-reportmanual.pdf \
	     ll-reportmanual.txt \
	     ll-userguide.html ll-userguide.pdf \
	     ll-userguide.txt 
# EXTRA_DIST = quickref.pdf quickref.ps lifelines.sgml llines.1 
EXTRA_DIST = llines.1 \
	      $(MANUALS) ll-reportmanual.xml  ll-userguide.xml 

CONFIG_CLEAN_FILES = lifelines.html lifelines.pdf lifelines.ps lifelines.txt \
	     ll-reportmanual.html ll-reportmanual.pdf ll-reportmanual.txt \
	     ll-userguide.html ll-userguide.pdf ll-userguide.txt 

docdir = $(datadir)/doc/lifelines
doc_DATA = $(EXTRA_DIST)
dist_pkgdata_DATA = $(MANUALS)


# sensible values: empty, simple, fancy
all: @DOCS_TARGET@

text: lifelines.txt 

pdb: lifelines.pdb

html: lifelines.html

simple: text html

ps: lifelines.ps

pdf: lifelines.pdf

fancy: ps pdf # pdb

# Simple Formats

lifelines.txt: lifelines.sgml
	sgmltools --backend=txt $^

lifelines.html: lifelines.sgml
	sgmltools --backend=onehtml $^

lifelines.pdb: lifelines.html
	rm -f $@
	isilo386 -Is156 -Ic -Id -d0 $^

# Fancy Formats

lifelines.ps: lifelines.sgml
	sgmltools --backend=ps $^

lifelines.pdf: lifelines.ps
	ps2pdf $^ $@

%.pdf: %.xml
	-jw -f docbook -b pdf $^

%.html: %.xml
	-jw -f docbook -b html -V nochunks $^

%.txt: %.html
	if [ -f /usr/bin/lynx ] ; then \
	/usr/bin/lynx -force_html -dump -nolist -width=76 $^ > $@ ;\
	fi

