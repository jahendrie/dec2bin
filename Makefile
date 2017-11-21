#===============================================================================
#		Makefile for dec2bin.c
#		Convert decimal numbers to binary
#===============================================================================

CC=gcc
PREFIX=/usr
FILES=dec2bin.c
#OPTFLAGS=-g -Wall
OPTFLAGS=-O3
OUTPUT=dec2bin
SRC=src
DOC=doc
MANPAGE=dec2bin.1.gz
OUTPUTDIR=$(PREFIX)/bin
MANPATH=$(PREFIX)/share/man/man1
DOCPATH=$(PREFIX)/share/doc/dec2bin
LICENSEPATH=$(PREFIX)/share/licenses/dec2bin

all: $(SRC)/dec2bin.c
	$(CC) $(OPTFLAGS) -o $(OUTPUT) $(SRC)/$(FILES)

install:
	install $(OUTPUT) -D $(OUTPUTDIR)/$(OUTPUT)
	install $(DOC)/$(MANPAGE) -D $(MANPATH)/$(MANPAGE)
	install README -D $(DOCPATH)/README
	install $(DOC)/CHANGES -D $(DOCPATH)/CHANGES
	install $(DOC)/LICENSE -D $(LICENSEPATH)/LICENSE

uninstall:
	rm -f $(OUTPUTDIR)/$(OUTPUT)
	rm -f $(MANPATH)/$(MANPAGE)
	rm -r $(DOCPATH)
	rm -r $(LICENSEPATH)

clean:
	rm -f $(OUTPUT)
