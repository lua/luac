/*
** $Id: test.c,v 1.3 1999/03/08 11:08:43 lhf Exp lhf $
** test integrity
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

#define AT		" at %d in %p (\"%s\":%d)"
#define UNSAFE(s)	luaL_verror("unsafe code: " s AT
#define ATLOC		at,tf,tf->source->str,tf->lineDefined)

static int check(int n, TProtoFunc* tf, int at, int sp, int ss)
{
#if 0
printf("check in  n=%d tf=%p ss=%d sp=%d at=%d\n",n,tf,ss,sp,at);
#endif
 if (n==0) return sp;
 sp+=n;
 if (sp<00) UNSAFE("stack underflow"),ATLOC;
 if (sp>ss) UNSAFE("stack overflow"),ATLOC;
#if 0
printf("check out n=%d tf=%p ss=%d sp=%d at=%d\n",n,tf,ss,sp,at);
#endif
 return sp;
}

#define CHECK(before,after)	\
	sp=check(-(before),tf,at,sp,ss), sp=check(after,tf,at,sp,ss)

static void TestStack(TProtoFunc* tf, int size, int* SP, int* JP)
{
 Byte* code=tf->code;
 Byte* p=code;
 int longarg=0;
 int ss=*p;
 int sp=0;
 while (1)
 {
	Opcode OP;
	int n=INFO(tf,p,&OP);
	int op=OP.class;
	int i=OP.arg+longarg;
	int at=p-code;

	longarg=0;
	SP[at]=sp;

	switch (op)
	{
	case PUSHCONSTANT:
	case GETGLOBAL:
	case GETDOTTED:
	case PUSHSELF:
	case SETGLOBAL:
	case CLOSURE:
		if (i>=tf->nconsts)
			UNSAFE("bad constant #%d (max=%d)"),i,tf->nconsts,ATLOC;
		break;
	case PUSHLOCAL:
	case SETLOCAL:
		if (i>=sp)
			UNSAFE("bad local #%d (max=%d)"),i,sp-1,ATLOC;
		break;
	case ONTJMP:
	case ONFJMP:
		JP[at]=-(at+i+n);	/* negate to remember ON?JMP */
		break;
	case JMP:
	case IFFJMP:
		JP[at]=at+i+n;
		break;
	case IFTUPJMP:
	case IFFUPJMP:
		JP[at]=at-i+n;
		break;
	}

#if 0
printf("tf=%p ss=%d sp=%d JP=%d at=%d %s %d %d\n",tf,ss,sp,JP[at],at,OP.name,i,OP.arg2);
#endif

	switch (op)
	{
	case STACK:		ss=i;			break;
	case ARGS:		CHECK(0,i);		break;
	case VARARGS:					break;
	case ENDCODE:					return;
	case RETCODE:		CHECK(i,0); sp=i;	break;
	case CALL:		CHECK(OP.arg2+1,i);	break;
	case TAILCALL:		CHECK(OP.arg2,0); sp=i;	break;
	case PUSHNIL:		CHECK(0,i+1);		break;
	case POP:		CHECK(0,-i);		break;
	case PUSHNUMBER:
	case PUSHNUMBERNEG:
	case PUSHCONSTANT:
	case PUSHUPVALUE:
	case PUSHLOCAL:
	case GETGLOBAL:		CHECK(0,1);		break;
	case GETTABLE:		CHECK(2,1);		break;
	case GETDOTTED:		CHECK(1,1);		break;
	case PUSHSELF:		CHECK(1,2);		break;
	case CREATEARRAY:	CHECK(0,1);		break;
	case SETLOCAL:		CHECK(1,0);		break;
	case SETGLOBAL:		CHECK(1,0);		break;
	case SETTABLEPOP:	CHECK(3,0);		break;
	case SETTABLE:		CHECK(i+3,i+2);		break;
	case SETLIST:		CHECK(OP.arg2+1,1);	break;
	case SETMAP:		CHECK(2*(i+1)+1,1);	break;
	case NEQOP:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:		CHECK(2,1);		break;
	case MINUSOP:
	case NOTOP:		CHECK(1,1);		break;
	case ONTJMP:
	case ONFJMP:
	case IFFJMP:
	case IFTUPJMP:
	case IFFUPJMP:		CHECK(1,0);		break;
	case JMP:					break;
	case CLOSURE:		CHECK(OP.arg2,1);	break;
	case SETLINE:					break;
	case LONGARG:
		longarg=i<<16;
		if (longarg<0) UNSAFE("longarg overflow"),ATLOC;
		break;
	case CHECKSTACK:				break;
	default:			/* cannot happen */
		UNSAFE("cannot handle opcode %d [%s]"),OP.op,OP.name,ATLOC;
		break;
	}
	p+=n;
 }
}

static void TestJumps(TProtoFunc* tf, int size, int* SP, int* JP)
{
 int i;
#define at i				/* for ATLOC */
 for (i=0; i<size; i++)
 {
  int to=JP[i];
  int on=0;
  if (to<0) { on=1; to=-to; };		/* handle ON?JMP */
  if (to!=0)
  {
   int j=i;
   int a,b;
   while (SP[++j]<0) ;			/* find next instruction */

#if 0
printf("tf=%p to=%d at=%d next=%d on=%d\n",tf,to,i,j,on);
printf("SP[%d]=%d SP[%d]=%d\n",j,SP[j],to,SP[to]);
#endif

   if (to<2 || to>=size)
    UNSAFE("invalid jump to %d (range is 2..%d)"),to,size-1,ATLOC;
   a=SP[to];
   if (a<0)
    UNSAFE("invalid jump to %d (not an instruction)"),to,ATLOC;
   b=SP[j]+on;
   if (a!=b)
    UNSAFE("stack inconsistency in jump to %d (%d x %d)"),to,b,a,ATLOC;
  }
 }
}

static void TestCode(TProtoFunc* tf)
{
 static int* SP=NULL;
 static int* JP=NULL;
 int size=luaU_codesize(tf);
 SP=luaM_reallocvector(SP,size,int);
 JP=luaM_reallocvector(JP,size,int);
 memset(SP,-1,size*sizeof(*SP));
 memset(JP, 0,size*sizeof(*JP));
 TestStack(tf,size,SP,JP);
 TestJumps(tf,size,SP,JP);
}

static void TestLocals(TProtoFunc* tf)
{
}

static void TestFunction(TProtoFunc* tf);

static void TestConstants(TProtoFunc* tf)
{
 int i,n=tf->nconsts;
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
   case LUA_T_STRING:
	break;
   case LUA_T_PROTO:
	TestFunction(tfvalue(o));
	break;
   case LUA_T_NIL:
	break;
   default:
	luaL_verror("cannot test constant #%d: type=%d [%s]",
		" in %p (\"%s\":%d)",
		i,ttype(o),luaO_typename(o),
		tf,tf->source->str,tf->lineDefined);
	break;
  }
 }
}

static void TestFunction(TProtoFunc* tf)
{
 TestCode(tf);
 TestLocals(tf);
 TestConstants(tf);
}

void luaU_testchunk(TProtoFunc* Main)
{
 TestFunction(Main);
}
