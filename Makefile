# $Id: Makefile,v 1.2 1998/06/19 21:23:35 lhf Exp lhf $
# makefile for lua compiler

# begin of configuration -----------------------------------------------------

# location of lua headers and library
LUA=lua

# compiler -------------------------------------------------------------------

# gcc
CC= gcc
INCS= -I$(LUA) -I/usr/5include
WARN= -ansi -Wall\
 -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-align -Waggregate-return
WARN= -ansi -pedantic -Wall

# Sun acc
CC= acc
WARN= #-fast

# SGI cc
CC= cc
WARN= -ansi -fullwarn

# end of configuration -------------------------------------------------------

CFLAGS= -g $(WARN) $(INCS) $(DEFS)
INCS= -I$(LUA)

OBJS= dump.o luac.o lundump.o print.o stubs.o opcode.o opt.o test.o
SRCS= dump.c luac.c lundump.c print.c stubs.c opcode.c opt.c test.c \
      luac.h lundump.h opcode.h luac.man

# targets --------------------------------------------------------------------

all:	opcode.h luac man lua

luac:	$(OBJS) $(LUA)/liblua.a
	$(CC) -o $@ $(OBJS) $(LUA)/liblua.a

lua:	../tmp/lua

../tmp/lua:	lundump.c lundump.h
	cd ../tmp; make update

opcode.h: lua/lopcodes.h mkopcodeh
	-mv -f $@ $@,old
	mkopcodeh lua/lopcodes.h >$@
	-diff $@,old $@
	rm -f opcode.o $@,old

man:	man/cat1/luac.1	luac.html

luac.html:	luac.man
	man2html luac.man >$@

man/cat1/luac.1:	luac.man
	nroff -man luac.man >$@

luac.man:

xman:	man
	env MANPATH=`pwd`/man:$MANPATH xman &

debug:	clean
	make DEFS="-DDEBUG"

noparser:
	rm -f stubs.o
	make DEFS="-DNOPARSER"

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
	cd test; make $@

co:
	co -l -M $(SRCS)

conl:
	co -M $(SRCS)

ci:
	ci $(SRCS)

diff:
	rcsdiff $(SRCS) Makefile

what:
	@grep '^[^	].*:	' Makefile | cut -f1 -d:
