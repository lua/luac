# $Id: Makefile,v 1.3 1997/06/27 20:23:34 lhf Exp lhf $
# makefile for lua compiler

LUA=lua
CC= gcc
CFLAGS= $(INCS) $(DEFS) $(WARN) -g

INCS= -I$(LUA) -I/usr/5include
#WARN= -ansi -pedantic -Wall

WARN= -ansi -Wall\
 -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-align -Waggregate-return

# for SGI
CC= cc
WARN= -ansi -fullwarn
INCS= -I$(LUA)

OBJS= luac.o dump.o print.o lundump.o
SRCS= dump.c luac.c luac.h print.c print.h lundump.c lundump.h

all: luac

luac: $(OBJS) $(LUA)/liblua.a
	$(CC) -o $@ $(OBJS) -L$(LUA) -llua

printh:
#	sed -n '/^[A-Z]/{s=,.*==;s=$$=",=;s=^= "=;p;}' < lua/lopcodes.h >op
	sed -n '/^[A-Z]/{s=\([A-Z0-9]*\).*$$= "\1",=;p;}' < lua/lopcodes.h > op

clean:
	rm -f luac $(OBJS) luac.out core

co:
	co -l -M $(SRCS)

ci:
	ci $(SRCS)
