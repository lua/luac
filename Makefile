# $Id: Makefile,v 1.11 2000/10/31 17:09:23 lhf Exp $
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
-W -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-align -Waggregate-return -Wcast-qual -Wnested-externs -Wwrite-strings

# end of configuration -------------------------------------------------------

CFLAGS= -O2 $(WARN) $(INCS) $(DEFS) $G
INCS= -I$(LUA)

OBJS= dump.o luac.o lundump.o opt.o print.o stubs.o
SRCS= dump.c luac.c lundump.c opt.c print.c stubs.c ldumplib.c \
	luac.h lundump.h print.h

# targets --------------------------------------------------------------------

all:	print.h stubs luac lua

luac:	$(OBJS) $(LUA)/liblua.a
	$(CC) -o $@ $(OBJS) $(LUA)/liblua.a

lua:	$(LUA)/lua

$(LUA)/lua: lundump.c lundump.h
	cd $(LUA); $(MAKE) update

print.h: $(LUA)/lopcodes.h mkprint.lua
	-mv -f $@ $@,old
	lua mkprint.lua <$(LUA)/lopcodes.h >$@
	-diff $@,old $@

stubs:	$(LUA)/lua
	diff lstate.c $(LUA)
	$(LUA)/lua stubs.lua <stubs.c >stubs.new
	diff stubs.c stubs.new
	-rm -f stubs.new

lib:	ldumplib.o

ldumplib.o:	ldumplib.c dump.c
	$(CC) $(CFLAGS) -c -o $@ ldumplib.c

debug:	clean
	$(MAKE) DEFS="-DLUA_DEBUG"

noparser:
	rm -f stubs.o luac
	$(MAKE) DEFS="-DNOPARSER"

nostubs:
	rm -f stubs.o luac
	$(MAKE) DEFS="-DNOSTUBS"

map:	$(OBJS)
	@echo -n '* use only '
	@ld -o /dev/null -e main -M $(OBJS) $(LUA)/liblua.a -lc | sed -n '/lua.liblua.a(/{s///;s/.o).*//;p;};/Memory/q' | sort | xargs echo
	grep 'use only' stubs.c

lint:
	lint -I$(LUA) *.c >lint.out

clean:
	-rm -f luac *.o luac.out a.out core mon.out gmon.out tags
	#cd test; $(MAKE) $@

co:
	co -l -M $(SRCS)

conl:
	co -M $(SRCS)

ci:
	ci $(SRCS)

diff:
	@-rcsdiff $(SRCS) Makefile 2>&1 | nawk -f rcsdiff.awk

wl:
	rlog -L -R RCS/*

what:
	@grep '^[^	].*:' Makefile | cut -f1 -d: | sort

ln:
	ln -s L/*.[ch] .

u:	$(OBJS)
	nm -o $(OBJS) | grep 'U lua'

ls:
	cp -fp $(LUA)/lstate.c .

tags:	$(SRCS)
	ctags $(SRCS)
