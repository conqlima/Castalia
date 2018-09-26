TOPDIR := /home/robson/Castalia-master/Castalia/src/node/application/manager
SRCDIR := $(TOPDIR)/src
BINDIR := $(TOPDIR)/bin

SOURCES := $(shell find $(SRCDIR) -name "*.c")
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))


CC := gcc
CFLAGS = -c -Wall -I$(TOPDIR) -I/home/robson/Castalia-master/Castalia/src/node/application/manager/src -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include


ALLFILES := $(SOURCES)

.PHONY: all

all: teste
	$(CC) $(OBJECTS) -o $(BINDIR)/$@

teste: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$@

