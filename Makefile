# $Id: Makefile,v 1.7 1999/07/02 19:40:36 lhf Exp lhf $
# makefile for lua compiler

# begin of configuration -----------------------------------------------------

# location of lua headers and library
LUA=lua

# compiler -------------------------------------------------------------------

# Sun acc
CC= acc
WARN= -Xc -vc #-fast

# SGI cc
CC= cc
WARN= -ansi -fullwarn

# gcc
CC= gcc
WARN= -ansi -pedantic -Wall -W
WARN= -ansi -pedantic -Wall \
-W -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-align -Waggregate-return -Wwrite-strings -Wcast-qual -Wnested-externs -Werror
# -Wstrict-prototypes -Wmissing-declarations -Wtraditional -Wconversion -Wredundant-decls

# end of configuration -------------------------------------------------------

CFLAGS= -g $(WARN) $(INCS) $(DEFS)
INCS= -I$(LUA)
#INCS= -I$(LUA) -I/usr/5include

OBJS= dump.o luac.o lundump.o opcode.o opt.o print.o stubs.o test.o
SRCS= dump.c luac.c lundump.c opcode.c opt.c print.c stubs.c test.c \
      luac.h lundump.h opcode.h

# targets --------------------------------------------------------------------

all:	opcode.h stubs luac lua

luac:	$(OBJS) $(LUA)/liblua.a
	$(CC) -o $@ $(OBJS) $(LUA)/liblua.a

lua:	lua/lua

lua/lua: lundump.c lundump.h
	cd lua; $(MAKE) update

opcode.h: lua/lopcodes.h mkopcodeh
	-mv -f $@ $@,old
	nawk -f mkopcodeh lua/lopcodes.h >$@
	-diff $@,old $@
	rm -f opcode.o $@,old

debug:
	rm -f print.o luac
	$(MAKE) DEFS="-DDEBUG"

olddebug:	clean
	$(MAKE) DEFS="-DDEBUG"

noparser:
	rm -f stubs.o luac
	$(MAKE) DEFS="-DNOPARSER"

nostubs:
	rm -f stubs.o luac
	$(MAKE) DEFS="-DNOSTUBS"

stubs:	lua/lua
	lua/lua stubs.lua < stubs.c >stubs.new
	diff stubs.c stubs.new
	-rm -f stubs.new

oldnostubs:
	rm -f stubs.o luac
	cat /dev/null >s.c; cc -o stubs.o -c s.c; rm -f s.c; $(MAKE)

map:	$(OBJS)
	@echo -n '* use only '
	@ld -o /dev/null -M $(OBJS) lua/liblua.a -lc | grep '	' | sort | xargs echo | sed 's/\.o//g'
	grep 'use only' stubs.c

MAP:	map
	nm -o -u $(OBJS) | grep lua._

lmap:	$(OBJS)
	@#ld -o /dev/null -e main -M $(OBJS) lua/liblua.a -lc | sed -n '/lua.liblua/p;/Memory/q' | sort
	@echo -n '* use only '
	@ld -o /dev/null -e main -M $(OBJS) lua/liblua.a -lc | sed -n '/lua.liblua.a(/{s///;s/.o).*//;p;};/Memory/q' | sort | xargs echo
	grep 'use only' stubs.c

lint:
	lint -I$(LUA) *.c >lint.out

clean:
	-rm -f luac *.o luac.out a.out core mon.out
	cd test; $(MAKE) $@

co:
	co -l -M $(SRCS)

conl:
	co -M $(SRCS)

ci:
	ci $(SRCS)

diff:
	@-rcsdiff $(SRCS) Makefile 2>&1 | nawk -f rcsdiff.awk

what:
	@grep '^[^	].*:' Makefile | cut -f1 -d: | sort
