# makefile for lua interpreter
# $Id: Makefile,v 1.1 1997/04/10 17:58:10 lhf Exp lhf $

CC= gcc
CFLAGS= $(INCS) $(DEFS) $(WARN) -O2 #-g

# in SunOs /usr/5include contains prototypes for standard lib
INCS= -I../lua # -I/usr/5include
WARN= -Wall -Wmissing-prototypes -Wshadow -ansi

OBJS= luac.o dump.o print.o

all: luac

luac: $(OBJS)
	$(CC) -o $@ $(OBJS) ../lua/liblua.a

clean:
	rm -f luac undump $(OBJS) undump.o

co:
	co -l -M dump.c luac.c luac.h print.c print.h undump.c undump.h

ci:
	ci dump.c luac.c luac.h print.c print.h undump.c undump.h
