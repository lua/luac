/*
** $Id: test.c,v 1.1 1998/03/30 11:22:25 lhf Exp lhf $
** test integrity
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

static int ss=0;
static int sp=0;

static void grow(TProtoFunc* tf, int n, int at)
{
 sp+=n;
printf("tf=%p at=%d ss=%d sp=%d\n",tf,at,ss,sp);
 if (sp<00) luaL_verror("unsafe code: stack underflow at %d in %p (\"%s\":%d)",
	at,tf,tf->fileName->str,tf->lineDefined);
 if (sp>ss) luaL_verror("unsafe code: stack overflow at %d in %p (\"%s\":%d)",
	at,tf,tf->fileName->str,tf->lineDefined);
}

#define GROW(n)	grow(tf,n,at)

static void TestCode(TProtoFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 ss=*p;
 sp=0;
 while (1)
 {
	Opcode OP;
	int n=INFO(tf,p,&OP);
	int op=OP.op;
	int i=OP.arg;
	int at=p-code;

	switch (OP.class)
	{

	case STACK:		ss=i;		break;

	case ENDCODE:				return;

	case ARGS:
	case SETLINE:
						break;

	case GETDOTTED:
	case MINUSOP:
	case NOTOP:
				GROW(0);	break;

	case PUSHNIL:		GROW(i+1);	break;

	case PUSHNUMBER:
	case PUSHCONSTANT:
	case PUSHUPVALUE:
	case PUSHLOCAL:
	case GETGLOBAL:
	case PUSHSELF:
	case CREATEARRAY:
				GROW(1);	break;

	case GETTABLE:
	case SETLOCAL:
	case SETGLOBAL:
	case EQOP:
	case NEQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
				GROW(-1);	break;

	case SETTABLE:
		if (OP.op==SETTABLE0) GROW(-3); else GROW(-5-2*i);
		break;

	case SETLIST:
		if (OP.op==SETLIST0) GROW(-i-1); else GROW(-OP.arg2-1);
		break;

	case SETMAP:		GROW(2*i+2);	break;

	case CLOSURE:
				GROW(-i);	break;

	case CALLFUNC:
				GROW(OP.arg-OP.arg2-1); break;

	default:
		fprintf(stderr,"TestCode: cannot handle %6d  %s\n",at,OP.name);
		break;
	}

	p+=n;
 }
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
   default:
	luaL_verror("cannot test constant #%d: type=%d [%s]",
		i,ttype(o),luaO_typename(o));
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

void TestChunk(TProtoFunc* Main)
{
 TestFunction(Main);
}
