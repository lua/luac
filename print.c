/*
** $Id: print.c,v 1.25 2000/01/28 17:51:09 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#ifdef DEBUG
static void PrintConstants(const TProtoFunc* tf)
{
 int i,n=tf->nconsts;
 printf("constants (%d) for %p:\n",n,tf);
 for (i=0; i<n; i++)
 {
  const TObject* o=tf->consts+i;
  printf("%6d ",i);
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	printf("N " NUMBER_FMT "\n",(double)nvalue(o));
	break;
   case LUA_T_STRING:
	printf("S %p\t\"%s\"\n",tsvalue(o),svalue(o));
	break;
   case LUA_T_LPROTO:
	printf("F %p\n",tfvalue(o));
	break;
   case LUA_T_NIL:
	printf("nil\n");
	break;
   default:				/* cannot happen */
	printf("? type=%d\n",ttype(o)); 
	break;
  }
 }
}

static void PrintLocvars(const TProtoFunc* tf)
{
 LocVar* v=tf->locvars;
 int i=0;
 int n=-1;
 printf("locvars for %p at %p:\n",tf,v);
 if (v==NULL) return;
 printf("%6s\t#\tL\tN\tP\n","I");
 do
 {
  if (v->varname==NULL) --n; else ++n;
  printf("%6d\t%d\t%d\t%s\t%p\n",
	i++,n,v->line,v->varname?v->varname->str:"-",v->varname);
 } while (v++->line>=0);
}
#endif

static void PrintCode(const TProtoFunc* tf)
{
 const Byte* code=tf->code;
 const Byte* p=code;
 int line=0;
 int longarg=0;
 for (;;)
 {
	Opcode OP;
	int n=INFO(tf,p,&OP);
	int i=OP.arg+longarg;
	int at=p-code;
	longarg=0;
	printf("%6d  ",at);
	{
	 const Byte* q=p;
	 int j=n;
	 while (j--) printf("%02X",*q++);
	}
	printf("%*s%-14s  ",2*(5-n),"",OP.name);
	if (OP.arg >=0) printf("%d",i);
	if (OP.arg2>=0) printf(" %d",OP.arg2);

	switch (OP.class)
	{

	case ENDCODE:
		printf("\n");
		return;

	case PUSHNUMBER:
       		printf("\t; " NUMBER_FMT,tf->knum[i]);
		break;

	case GETGLOBAL:
	case SETGLOBAL:
	case GETDOTTED:
       		printf("\t; \"%s\"",tf->kstr[i]->str);
		break;

	case PUSHSELF:
	case CLOSURE:
       		printf("\t; %p",tf->kproto[i]);
		break;

	case PUSHLOCAL:
	case SETLOCAL:
	{
		const char* s=luaF_getlocalname(tf,i+1,line);
		if (s) printf("\t; %s",s);
		break;
	}

	case SETLINE:
		printf("\t; " SOURCE,tf->source->str,line=i);
		break;

	case SETNAME:
	{
		printf("\t; %s \"%s\"",luaO_typenames[OP.arg2],tf->kstr[i]->str);
		break;
	}

	case LONGARG:
		longarg=i<<16;
		break;

/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case IFFJMP:
		printf("\t; to %d",at+i+n);
		break;
	case IFTUPJMP:
	case IFFUPJMP:
		printf("\t; to %d",at-i+n);
		break;

	}
	printf("\n");
	p+=n;
 }
}

static void PrintLocals(const TProtoFunc* tf)
{
 const LocVar* v=tf->locvars;
 int n,i;
 if (v==NULL || v->line<0) return;
 n=tf->code[1]; if (n>=ZEROVARARG) n-=ZEROVARARG;
 printf("locals:");
 for (i=0; i<n; v++,i++)		/* arguments */
  printf(" %s",v->varname->str);
 for (; v->line>=0; v++)
 {
  if (v->varname==NULL)
  {
   if (--i<0) luaL_verror(L,"bad locvars[%d]",v-tf->locvars); else printf(")");
  }
  else
  {
   ++i; printf(" (%s",v->varname->str);
  }
 }
 i-=n;
 while (i--) printf(")");
 printf("\n");
}

#define IsMain(tf)	(tf->lineDefined==0)

static void PrintHeader(const TProtoFunc* tf, const TProtoFunc* Main, int at)
{
 int size=luaU_codesize(tf);
 if (IsMain(tf))
  printf("\nmain " SOURCE " (%d bytes at %p)\n",
	tf->source->str,tf->lineDefined,size,tf);
 else
 {
  printf("\nfunction " SOURCE " (%d bytes at %p); used at ",
	tf->source->str,tf->lineDefined,size,tf);
  if (Main && IsMain(Main))
   printf("main");
  else
   printf("%p",Main);
  printf("+%d\n",at);
 }
}

static void PrintFunction(const TProtoFunc* tf, const TProtoFunc* Main, int at);

static void PrintFunctions(const TProtoFunc* Main)
{
 const Byte* code=Main->code;
 const Byte* p=code;
 int longarg=0;
 for (;;)
 {
  Opcode OP;
  int n=INFO(Main,p,&OP);
  int op=OP.class;
  int i=OP.arg+longarg;
  longarg=0;
  if (op==CLOSURE)
   PrintFunction(Main->kproto[i],Main,(int)(p-code));
  else if (op==LONGARG) longarg=i<<16;
  else if (op==ENDCODE) break;
  p+=n;
 }
}

static void PrintFunction(const TProtoFunc* tf, const TProtoFunc* Main, int at)
{
 PrintHeader(tf,Main,at);
#ifdef DEBUG
 PrintLocvars(tf);
#endif
 PrintLocals(tf);
 PrintCode(tf);
#ifdef DEBUG
 PrintConstants(tf);
#endif
 PrintFunctions(tf);
}

void luaU_printchunk(const TProtoFunc* Main)
{
 PrintFunction(Main,0,0);
}
