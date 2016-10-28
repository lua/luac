# $Id: Makefile,v 1.25 2005/05/12 00:29:32 lhf Exp lhf $
# makefile for Lua compiler

LUA= distr
WARN= -Wall -Wextra $(XWARN)
XWARN= -Wc++-compat -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes
XWARN=

CC= gcc -std=gnu99
CFLAGS= -O2 $(WARN) $(INCS) $G
INCS= -I$(LUA)

LIBS= $(LUA)/liblua.a -lm
OBJS= luac.o
SRCS= luac.c

# targets --------------------------------------------------------------------

all:	luac

luac:	$(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(LIBS):
	make -C $(LUA) a "CFLAGS=$(CFLAGS)"

print.c:	$(LUA)/lopcodes.h
	@diff lopcodes.h $(LUA)

clean:
	-rm -f luac *.o luac.out a.out core core.* mon.out gmon.out tags luac.lst lua
	@#cd test; $(MAKE) $@

co:
	co -l -M $(SRCS)

conl:
	co -M $(SRCS)

ci:
	ci -u $(SRCS)

diff:
	@#-rcsdiff $(SRCS) Makefile 2>&1 | awk -f rcsdiff.awk
	@-rcsdiff $(SRCS) 2>&1 | awk -f rcsdiff.awk

wl:
	@rlog -L -R RCS/* | sed 's/RCS.//;s/,v//'

opp:
	grep Kst lopcodes.h | grep ^OP_; echo ''
	grep RK  lopcodes.h | grep ^OP_; echo ''
	grep pc  lopcodes.h | grep ^OP_; echo ''

diffd:
	-diff ldump.c $(LUA)
	-diff lundump.c $(LUA)
	-diff lundump.h $(LUA)
	-cat luac.c print.c | diff - $(LUA)/luac.c

