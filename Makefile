# makefile for lua interpreter
# $Id$

CC= gcc
CFLAGS= $(INCS) $(DEFS) $(WARN) -O2 #-g

# in SunOs /usr/5include contains prototypes for standard lib
INCS= -I../lua # -I/usr/5include
WARN= -Wall -Wmissing-prototypes -Wshadow -ansi

OBJS= luac.o dump.o print.o

all: luac undump

luac: $(OBJS)
	$(CC) -o $@ $(OBJS) ../lua/liblua.a

undump: luau.o undump.o print.o
	$(CC) -o $@ luau.o undump.o print.o ../lua/liblua.a

clean:
	rm -f luac undump $(OBJS) undump.o luau.o

co:
	co -l -M dump.c luac.c luac.h luau.c print.c print.h undump.c undump.h

ci:
	ci dump.c luac.c luac.h luau.c print.c print.h undump.c undump.h
