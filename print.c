/*
** $Id: print.c,v 1.17 1999/03/16 18:13:56 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#ifdef DEBUG
static void PrintConstants(TProtoFunc* tf)
{
 int i,n=tf->nconsts;
 printf("constants (%d) for %p:\n",n,tf);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  printf("%6d ",i);
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	printf("N " NUMBER_FMT "\n",(double)nvalue(o));
	break;
   case LUA_T_STRING:
	printf("S %p\t\"%s\"\n",tsvalue(o),svalue(o));
	break;
   case LUA_T_PROTO:
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
#endif

static void PrintConstant(TProtoFunc* tf, int i)
{
 if (i>=tf->nconsts)
  printf("(bad constant #%d: max=%d)",i,tf->nconsts-1);
 else
 {
  TObject* o=tf->consts+i;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	printf(NUMBER_FMT,(double)nvalue(o));
	break;
   case LUA_T_STRING:
	printf("\"%s\"",svalue(o));
	break;
   case LUA_T_PROTO:
	printf("function at %p",(void*)tfvalue(o));
	break;
   case LUA_T_NIL:
	printf("(nil)");
	break;
   default:				/* cannot happen */
	luaU_badconstant("print",i,o,tf);
	break;
  }
 }
}

static void PrintCode(TProtoFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 int line=0;
 int longarg=0;
 while (1)
 {
	Opcode OP;
	int n=INFO(tf,p,&OP);
	int i=OP.arg+longarg;
	longarg=0;
	printf("%6d  ",(int)(p-code));
	{
	 Byte* q=p;
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

	case PUSHCONSTANT:
	case GETDOTTED:
	case PUSHSELF:
	case CLOSURE:
		printf("\t; ");
		PrintConstant(tf,i);
		break;

	case PUSHLOCAL:
	case SETLOCAL:
	{
		char* s=luaF_getlocalname(tf,i+1,line);
		if (s) printf("\t; %s",s);
		break;
	}

	case GETGLOBAL:
	case SETGLOBAL:
		printf("\t; %s",svalue(tf->consts+i));
		break;

	case SETLINE:
		printf("\t; " SOURCE,tf->source->str,line=i);
		break;

	case LONGARG:
		longarg=i<<16;
		break;

/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
	case IFTUPJMP:
	case IFFUPJMP:
		i=-i;
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case IFFJMP:
		printf("\t; to %d",(int)(p-code)+i+n);
		break;

	}
	printf("\n");
	p+=n;
 }
}

static void DumpLocals(TProtoFunc* tf)
{
 LocVar* v=tf->locvars;
 int i=0;
 if (v==NULL) return;
 printf("dumplocals:\n");
 do
 {
  printf("%d line=%d varname=%s [%p]\n",
	i++,v->line,v->varname?v->varname->str:"",v->varname);
 } while (v++->line>=0);
}

static void PrintLocals(TProtoFunc* tf)
{
 LocVar* v=tf->locvars;
 int n,i;
 DumpLocals(tf);
 if (v==NULL || v->line<0) return;
 n=tf->code[1]; if (n>=ZEROVARARG) n-=ZEROVARARG;
 printf("locals:");
 for (i=0; i<n; v++,i++)		/* arguments */
  printf(" %s",v->varname->str);
 for (; v->line>=0; v++)
 {
  if (v->varname==NULL)
  {
   printf(")"); --i;
  }
  else
  {
   printf(" (%s",v->varname->str); i++;
  }
 }
 i-=n;
 while (i--) printf(")");
 printf("\n");
}

#define IsMain(tf)	(tf->lineDefined==0)

static void PrintHeader(TProtoFunc* tf, TProtoFunc* Main, int at)
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

static void PrintFunction(TProtoFunc* tf, TProtoFunc* Main, int at);

static void PrintFunctions(TProtoFunc* Main)
{
 Byte* code=Main->code;
 Byte* p=code;
 int longarg=0;
 while (1)
 {
  Opcode OP;
  int n=INFO(Main,p,&OP);
  int op=OP.class;
  int i=OP.arg+longarg;
  longarg=0;
  if (op==PUSHCONSTANT || op==CLOSURE)
  {
   TObject* o=Main->consts+i;
   if (ttype(o)==LUA_T_PROTO) PrintFunction(tfvalue(o),Main,(int)(p-code));
  }
  else if (op==LONGARG) longarg=i<<16;
  else if (op==ENDCODE) break;
  p+=n;
 }
}

static void PrintFunction(TProtoFunc* tf, TProtoFunc* Main, int at)
{
 PrintHeader(tf,Main,at);
 PrintLocals(tf);
 PrintCode(tf);
#ifdef DEBUG
 PrintConstants(tf);
#endif
 PrintFunctions(tf);
}

void luaU_printchunk(TProtoFunc* Main)
{
 PrintFunction(Main,0,0);
}
