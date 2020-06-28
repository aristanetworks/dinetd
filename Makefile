# Copyright © 2020 Arista Networks, Inc. All rights reserved.
#
# Use of this source code is governed by the MIT license that can be found
# in the LICENSE file.

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
MANDIR ?= $(DATADIR)/man

CFLAGS ?= -O2
CFLAGS += -std=c99 -Wall -Wextra -Wno-unused-parameter -fno-strict-aliasing
CPPFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

SRCS := main.c usage.c
OBJS := $(subst .c,.o,$(SRCS))
BINS := dinetd
MANS := dinetd.1.gz

all: $(BINS) man

generate: usage.txt
	(echo "/* Copyright © 2020 Arista Networks, Inc. All rights reserved."; \
	 echo " */"; \
	 echo ""; \
	 echo "/* This file is generated from usage.txt. Do not edit. */"; \
	 xxd -i usage.txt) > usage.c

dinetd: $(OBJS)
	$(LINK.o) -o $@ $^

%.gz: %.scd
	scdoc <$< | gzip -c >$@

install: $(BINS) $(MANS)
	install -m 755 -D dinetd $(DESTDIR)$(BINDIR)/dinetd
	install -m 644 -D dinetd.1.gz $(DESTDIR)$(MANDIR)/man1/dinetd.1.gz

check: export PATH := $(DESTDIR)$(BINDIR):${PATH}
check: $(BINS)
	./test/cram.sh test

clean:
	$(RM) $(BINS) $(OBJS) dinetd.1.gz

.PHONY: all clean install generate check man
