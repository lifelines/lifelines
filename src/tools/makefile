BIN = ../bin
HDRS =../hdrs

DFILES = btedit.c wprintf.c makefile

btedit302: btedit.o wprintf.o ../btree/btree.a
	cc -o btedit302 btedit.o wprintf.o ../btree/btree.a
	rm -rf $(BIN)/btedit302
	ln btedit302 $(BIN)

showkeys: showkeys.o
	cc -o showkeys showkeys.o wprintf.o ../btree/btree.a
	rm -rf $(BIN)/showkeys
	ln showkeys $(BIN)

extract: extract.o
	cc -o extract extract.o wprintf.o ../btree/btree.a
	rm -rf $(BIN)/extract
	ln extract $(BIN)

dump: dump.o
	cc -o dump dump.o wprintf.o ../btree/btree.a
	rm -rf $(BIN)/dump
	ln dump $(BIN)

distribute:
	mkdir ../dist/tools
	cp $(DFILES) ../dist/tools

.c.o:
	cc -c -g -I$(HDRS) $<

