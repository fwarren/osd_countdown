#
# Makefile for X-on-screen display
#

PREFIX=/usr/local
EXEC_PREFIX=$(PREFIX)
BINDIR=$(EXEC_PREFIX)/bin
LIBDIR=$(EXEC_PREFIX)/lib
MANDIR=$(PREFIX)/man
INCLUDEDIR=$(PREFIX)/include

CC=gcc
INSTALL=/usr/bin/install -c
INSTALL_DATA=$(INSTALL) -m 644

CFLAGS=-O2 -Wall -pipe -I. -DXOSD_VERSION=\"$(VERSION)\" -I/usr/X11R6/include
LDFLAGS=-L. -L/usr/X11R6/lib -lX11 -lXext -lpthread -lXt

XOSDLIBS=-lxosd

SOURCES=LICENSE Makefile README.md \
	osd_countdown.c osd_countdown.1

ARFLAGS=cru

all: osd_countdown

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o.pic: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ -fPIC

osd_countdown: osd_countdown.o
	$(CC) -o $@ osd_countdown.o $(LDFLAGS) $(XOSDLIBS)

tar: xosd-$(VERSION).tar.gz

install: all
	$(INSTALL) osd_countdown $(BINDIR)
	mkdir -p $(MANDIR)/man1 $(MANDIR)/man3
	$(INSTALL_DATA) osd_countdown.1 $(MANDIR)/man1/

clean:
	rm -f *~ *.o *.o.pic osd_countdown

.PHONY: all tar clean install
# vim: noexpandtab
