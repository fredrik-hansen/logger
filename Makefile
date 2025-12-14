# Makefile for logger

CC = gcc
LIBS = -ldl
PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
DESTDIR ?=

all: logger.so detect start

logger.so:  logger.c std_libc.h
	$(CC) -shared -O2 -fPIC logger.c -ologger.so $(LIBS)

detect: detect.c std_libc.h
	$(CC) detect.c -o detect $(LIBS)

start: start.c std_libc.h
	$(CC) start.c -o start $(LIBS)

install: all
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 logger.so $(DESTDIR)$(LIBDIR)/logger.so
	@if [ -z "$(DESTDIR)" ]; then \
		LIBDIR=$(LIBDIR) ./install.sh; \
	else \
		echo "DESTDIR set, skipping ld.so.preload update"; \
	fi

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/logger.so
	@if [ -z "$(DESTDIR)" ] && [ -f /etc/ld.so.preload ]; then \
		sed -i.bak '/logger\.so/d' /etc/ld.so.preload; \
		echo "Removed logger.so from /etc/ld.so.preload"; \
	fi

std_libc.h: check_std_libc.sh
	./check_std_libc.sh

test: all
	./detect
	./start ./detect

clean:
	rm -f detect logger.so start std_libc.h 

