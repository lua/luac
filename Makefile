# makefile for lua compiler
# $Id: Makefile,v 1.2 1997/04/10 18:04:35 lhf Exp lhf $

CC= gcc
CFLAGS= $(INCS) $(DEFS) $(WARN) -O2 -g

# in SunOs /usr/5include contains prototypes for standard lib
INCS= -I../lua # -I/usr/5include
WARN= -Wall -Wmissing-prototypes -Wshadow -ansi

OBJS= luac.o dump.o print.o
SRCS= dump.c luac.c luac.h print.c print.h # undump.c undump.h

all: luac

luac: $(OBJS)
	$(CC) -o $@ $(OBJS) ../lua/liblua.a

clean:
	rm -f luac $(OBJS) luac.out

co:
	co -l -M $(SRCS)

ci:
	ci $(SRCS)
