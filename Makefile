# $Id: Makefile,v 1.22 2003/08/29 10:44:45 lhf Exp lhf $
# makefile for Lua compiler

LUA=..
WARN= -ansi -pedantic -Wall -W
CFLAGS= -O2 $(WARN) $(INCS) $G
INCS= -I$(LUA)

LIBS= $(LUA)/liblua.a $(LUA)/lauxlib.o
OBJS= ldump.o luac.o lundump.o print.o lopcodes.o
SRCS= ldump.c luac.c lundump.c print.c lundump.h

# targets --------------------------------------------------------------------

all:	luac

luac:	$(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(LIBS):
	make -C $(LUA) o $@

lopcodes.o:	$(LUA)/lopcodes.c $(LUA)/lopcodes.h
	$(CC) -o $@ -c $(CFLAGS) -DLUA_OPNAMES $(LUA)/lopcodes.c

print.c:	$(LUA)/lopcodes.h
	@diff lopcodes.h $(LUA)

clean:
	-rm -f luac *.o luac.out a.out core core.* mon.out gmon.out tags luac.lst
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

lo:
	cp -fp $(LUA)/lopcodes.h .

tags:	$(SRCS)
	ctags $(SRCS)

depend:
	@$(CC) -MM $(CFLAGS) $(SRCS)

opp:
	grep Kst lopcodes.h | grep ^OP_; echo ''
	grep RK  lopcodes.h | grep ^OP_; echo ''
	grep pc  lopcodes.h | grep ^OP_; echo ''

u:	$(OBJS)
	nm -o $(OBJS) | grep 'U lua'

api:	$(OBJS)
	nm -o *.o | grep 'T lua._' | sed 's/:.*T /	/'

libc:	$(OBJS)
	rm -f lua.o
	nm -o *.o | grep -v 'lib.o' | grep ' U ' | grep -v ' U lua'

