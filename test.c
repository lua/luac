/*
** $Id: test.c,v 1.5 1999/03/16 18:13:56 lhf Exp lhf $
** test integrity
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

#define AT		" at %d" IN
#define ATLOC		at,INLOC)
#define UNSAFE(s)	luaL_verror("unsafe code: " s AT

static int check(int n, TProtoFunc* tf, int at, int sp, int ss)
{
 if (n==0) return sp;
 sp+=n;
 if (sp<00) UNSAFE("stack underflow (sp=%d)"),sp,ATLOC;
 if (sp>ss) UNSAFE("stack overflow (sp=%d ss=%d)"),sp,ss,ATLOC;
 return sp;
}

#define CHECK(before,after)	\
	sp=check(-(before),tf,at,sp,ss), sp=check(after,tf,at,sp,ss)

static void TestStack(TProtoFunc* tf, int size, int* SP, int* JP)
{
 Byte* code=tf->code;
 Byte* p=code;
 int longarg=0;
 int ss=0;
 int sp=0;
 while (1)
 {
	Opcode OP;
	int n=INFO(tf,p,&OP);
	int op=OP.class;
	int i=OP.arg+longarg;
	int at=p-code;
	longarg=0;
#if 0
printf("%6d%8d    %-14s  %d %d\n",at,sp,OP.name,i,OP.arg2);
#endif
	switch (op)			/* test sanity of operands */
	{
	case PUSHCONSTANT:
	case GETGLOBAL:
	case GETDOTTED:
	case PUSHSELF:
	case SETGLOBAL:
	case CLOSURE:
	{
		TObject* o;
		if (i>=tf->nconsts)
			UNSAFE("bad constant #%d (max=%d)"),
			i,tf->nconsts-1,ATLOC;
		o=tf->consts+i;
		if ((op==CLOSURE   && ttype(o)!=LUA_T_PROTO)
		 || (op==GETGLOBAL && ttype(o)!=LUA_T_STRING)
		 || (op==SETGLOBAL && ttype(o)!=LUA_T_STRING))
			UNSAFE("bad operand to %s"),OP.name,ATLOC;
		break;
	}
	case PUSHLOCAL:
	case SETLOCAL:
		if (i>=sp) UNSAFE("bad local #%d (max=%d)"),i,sp-1,ATLOC;
		break;
	case ONTJMP:
	case ONFJMP:
		JP[at]=-(at+i+n);	/* negate to remember ON?JMP */
		break;
	case JMP:			/* remember JMP targets */
	case IFFJMP:
		JP[at]=at+i+n;
		break;
	case IFTUPJMP:
	case IFFUPJMP:
		JP[at]=at-i+n;
		break;
	}

	SP[at]=sp;			/* remember depth before instruction */

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
		UNSAFE("cannot test opcode %d [%s]"),OP.op,OP.name,ATLOC;
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
   if (to<2 || to>=size)
    UNSAFE("invalid jump to %d (range is 2..%d)"),to,size-1,ATLOC;
   a=SP[to];
   if (a<0)
    UNSAFE("invalid jump to %d (not an instruction)"),to,ATLOC;
   while (SP[++j]<0) ;			/* find next instruction */
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
 luaM_reallocvector(SP,size,int); memset(SP,-1,size*sizeof(int));
 luaM_reallocvector(JP,size,int); memset(JP, 0,size*sizeof(int));
 TestStack(tf,size,SP,JP);
 TestJumps(tf,size,SP,JP);
}

static void TestLocals(TProtoFunc* tf)
{
 LocVar* v=tf->locvars;
 int i,l,n;
 if (v==NULL) return;
 for (l=n=i=0; v->line>=0; v++,i++)
 {
  if (l>v->line)
   UNSAFE("bad linenumber %d in locvars[%d]"),v->line,i,ATLOC;
  else
   l=v->line;
  if (v->varname==NULL)
  {
   if (--n<0) UNSAFE("no scope to close in locvars[%d]"),i,ATLOC;
  }
  else
   ++n;
 }
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
	break;
   case LUA_T_STRING:
	break;
   case LUA_T_PROTO:
	TestFunction(tfvalue(o));
	break;
   case LUA_T_NIL:
	break;
   default:				/* cannot happen */
	luaU_badconstant("print",i,o,tf);
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
