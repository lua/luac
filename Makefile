# $Id: Makefile,v 1.20 2003/03/11 23:52:39 lhf Exp lhf $
# makefile for Lua compiler

# begin of configuration -----------------------------------------------------

# location of Lua headers and library
LUA=..

# compiler -------------------------------------------------------------------

# gcc
CC= gcc
WARN= -ansi -pedantic -Wall \
-W -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-align -Waggregate-return -Wcast-qual -Wnested-externs -Wwrite-strings
WARN= -ansi -pedantic -Wall -W

# end of configuration -------------------------------------------------------

CFLAGS= -O2 $(WARN) $(INCS) $G
INCS= -I$(LUA)
LIBS= $(LUA)/liblua.a $(LUA)/liblualib.a

OBJS= ldump.o luac.o lundump.o print.o lopcodes.o
SRCS= ldump.c luac.c lundump.c print.c lundump.h

# targets --------------------------------------------------------------------

all:	luac

luac:	$(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(LIBS):
	cd $(LUA); make

lopcodes.o:	$(LUA)/lopcodes.c $(LUA)/lopcodes.h
	$(CC) $(CFLAGS) -DLUA_OPNAMES -c -o $@ $(LUA)/lopcodes.c

print.c:	$(LUA)/lopcodes.h
	@diff lopcodes.h $(LUA)

debug:
	$(CC) -c $(CFLAGS) -DLUA_USER_H='"ltests.h"' *.c
	$(MAKE)

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
	@-rcsdiff $(SRCS) Makefile 2>&1 | awk -f rcsdiff.awk

wl:
	@rlog -L -R RCS/* | sed 's/RCS.//;s/,v//' 

what:
	@grep '^[^	].*:' Makefile | cut -f1 -d: | sort | column

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

opp:
	grep Kst lopcodes.h | grep ^OP_; echo ''
	grep RK  lopcodes.h | grep ^OP_; echo ''
	grep PC  lopcodes.h | grep ^OP_; echo ''
