# This makefile creates the documentation directory

# AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = quickref.pdf quickref.ps lifelines.sgml
CONFIG_CLEAN_FILES = lifelines.html lifelines.pdf lifelines.ps lifelines.txt

docdir = $(datadir)/doc/lifelines
doc_DATA = $(EXTRA_DIST)


all: simple fancy

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

