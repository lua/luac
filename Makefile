# $Id: Makefile,v 1.15 2001/11/01 08:52:26 lhf Exp lhf $
# makefile for Lua compiler

# begin of configuration -----------------------------------------------------

# location of Lua headers and library
LUA=lua

# compiler -------------------------------------------------------------------

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

OBJS= dump.o luac.o lundump.o print.o lopcodes.o
SRCS= dump.c luac.c lundump.c print.c luac.h ldumplib.c lundump.h

# targets --------------------------------------------------------------------

all:	luac lib

luac:	$(OBJS) $(LUA)/liblua.a
	$(CC) -o $@ $(OBJS) $(LUA)/liblua.a

lib:	ldumplib.o

ldumplib.o:	ldumplib.c dump.c
	$(CC) $(CFLAGS) -c -o $@ ldumplib.c

lopcodes.o:	../lopcodes.c ../lopcodes.h
	$(CC) $(CFLAGS) -DLUA_OPNAMES -c -o $@ ../lopcodes.c

print.c:	../lopcodes.h
	@diff lopcodes.h ..

debug:	clean
	$(MAKE) DEFS="-DLUA_DEBUG"

noparser:
	rm -f luac.o luac
	$(MAKE) DEFS="-DNOPARSER"

lint:
	lint -I$(LUA) *.c >lint.out

clean:
	-rm -f luac *.o luac.out a.out core mon.out gmon.out tags luac.lst
	@#cd test; $(MAKE) $@

co:
	co -l -M $(SRCS)

conl:
	co -M $(SRCS)

ci:
	ci -u $(SRCS)

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

lo:
	cp -fp $(LUA)/lopcodes.h .

tags:	$(SRCS)
	ctags $(SRCS)

depend:
	@$(CC) -MM $(CFLAGS) $(SRCS)
