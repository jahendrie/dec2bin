#===============================================================================
#		Makefile for dec2bin.c
#		Convert decimal numbers to binary
#===============================================================================

CC=gcc
PREFIX=/usr
FILES=dec2bin.c
OPTFLAGS=-g -Wall
OUTPUT=dec2bin
SRC=src
DOC=doc
MANPAGE=dec2bin.1.gz
OUTPUTDIR=$(PREFIX)/bin
MANPATH=$(PREFIX)/share/man/man1

all: $(SRC)/dec2bin.c
	$(CC) $(OPTFLAGS) -o $(OUTPUT) $(SRC)/$(FILES)

install:
	install $(OUTPUT) -D $(OUTPUTDIR)/$(OUTPUT)
	install $(DOC)/$(MANPAGE) -D $(MANPATH)/$(MANPAGE)

uninstall:
	rm -f $(OUTPUTDIR)/$(OUTPUT)
	rm -f $(MANPATH)/$(MANPAGE)

clean:
	rm -f $(OUTPUT)
