# $Id: Makefile,v 1.4 1999/03/08 11:23:16 lhf Exp lhf $
# $(MAKE)file for lua compiler

# begin of configuration -----------------------------------------------------

# location of lua headers and library
LUA=lua

# compiler -------------------------------------------------------------------

# Sun acc
CC= acc
WARN= #-fast

# gcc
CC= gcc
INCS= -I$(LUA) -I/usr/5include
WARN= -ansi -pedantic -Wall \
 -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-align -Waggregate-return
WARN= -ansi -pedantic -Wall

# SGI cc
CC= cc
WARN= -ansi -fullwarn

# end of configuration -------------------------------------------------------

CFLAGS= -g $(WARN) $(INCS) $(DEFS)
INCS= -I$(LUA)

OBJS= dump.o luac.o lundump.o opcode.o opt.o print.o stubs.o test.o
SRCS= dump.c luac.c lundump.c opcode.c opt.c print.c stubs.c test.c \
      luac.h lundump.h opcode.h luac.man

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

man:	man/cat1/luac.1	luac.html

luac.html:	luac.man
	man2html luac.man >$@

man/cat1/luac.1:	luac.man
	nroff -man luac.man >$@

luac.man:

xman:	man
	env MANPATH=`pwd`/man:$(MANPATH) xman &

debug:	clean
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
	rcsdiff $(SRCS) Makefile

what:
	@grep '^[^	].*:' Makefile | cut -f1 -d: | sort
