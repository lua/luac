/*
** $Id: print.c,v 1.11 1998/06/13 16:54:15 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

int CodeSize(TProtoFunc* tf)		/* also used in dump.c */
{
 Byte* code=tf->code;
 Byte* p=code;
 while (1)
 {
  Opcode OP;
  p+=INFO(tf,p,&OP);
  if (OP.op==ENDCODE) break;
 }
 return p-code;
}

#ifdef DEBUG
void PrintConstant1(TProtoFunc* tf, int i)
{
 TObject* o=tf->consts+i;
 printf("%6d ",i);
 switch (ttype(o))
 {
  case LUA_T_NUMBER:
	printf("N " NUMBER_FMT "\n",(double)nvalue(o));	/* LUA_NUMBER */
	break;
  case LUA_T_STRING:
	printf("S %p\t\"%s\"\n",tsvalue(o),svalue(o));
	break;
  case LUA_T_PROTO:
	printf("F %p\n",tfvalue(o));
	break;
  default:				/* cannot happen */
	printf("? %d\n",ttype(o)); 
  break;
 }
}

static void PrintConstants(TProtoFunc* tf)
{
 int i,n=tf->nconsts;
 printf("constants (%d):\n",n);
 for (i=0; i<n; i++) PrintConstant1(tf,i);
}
#endif

static void PrintConstant(TProtoFunc* tf, int i)
{
#ifdef DEBUG
 if (i<0 || i>=tf->nconsts)
 {
  printf("(bad constant #%d. max=%d)",i,tf->nconsts);
 }
 else
#endif
 {
  TObject* o=tf->consts+i;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	printf(NUMBER_FMT,(double)nvalue(o));		/* LUA_NUMBER */
	break;
   case LUA_T_STRING:
	printf("\"%s\"",svalue(o));
	break;
   case LUA_T_PROTO:
	printf("function at %p",tfvalue(o));
	break;
   default:				/* cannot happen */
	LUA_INTERNALERROR("bad constant");
	break;
  }
 }
}

#define VarStr(i)       svalue(tf->consts+i)

static void PrintCode(TProtoFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 int line=0;
 while (1)
 {
	Opcode OP;
	int n=INFO(tf,p,&OP);
	int op=OP.op;
	int i=OP.arg;
	printf("%6d  ",(int)(p-code));
	{
	 Byte* q=p;
	 int j=n;
	 while (j--) printf("%02X",*q++);
	}
	printf("%*s%-13s",2*(5-n),"",OP.name);

	if (n!=1 || op<0) printf("\t%d",i); else if (i>=0) printf("\t");

	switch (OP.class)
	{

	case ENDCODE:
		printf("\n");
		return;

	case CLOSURE:
		printf(" %d",OP.arg2);
	case PUSHCONSTANT:
	case GETDOTTED:
	case PUSHSELF:
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
		printf("\t; %s",VarStr(i));
		break;

	case SETLIST:
	case CALLFUNC:
		if (n>=3) printf(" %d",OP.arg2);
		break;

	case SETLINE:
		printf("\t; \"%s\":%d",tf->fileName->str,line=i);
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

static void PrintLocals(TProtoFunc* tf)
{
 LocVar* v=tf->locvars;
 int n,i=0;
 if (v==NULL || v->varname==NULL) return;
 n=tf->code[1]; if (n>=ZEROVARARG) n-=ZEROVARARG;

 printf("locals:");
 if (n>0)
 {
#ifdef TRACELOCALS
  for (i=0; i<n; v++,i++) printf(" %s[%d@%d]",v->varname->str,i,v->line);
#else
  for (i=0; i<n; v++,i++) printf(" %s",v->varname->str);
#endif
 }
 if (v->varname!=NULL)
 {
  for (; v->line>=0; v++)
  {
   if (v->varname==NULL)
   {
#ifdef TRACELOCALS
    printf(")[%d@%d]",--i,v->line);
#else
    printf(")"); --i;
#endif
   }
   else
#ifdef TRACELOCALS
    printf(" (%s[%d@%d]",v->varname->str,i++,v->line);
#else
    printf(" (%s",v->varname->str); i++;
#endif
  }
  i-=n;
  while (i--) printf(")");
 }
 printf("\n");
}

static void PrintHeader(TProtoFunc* tf, TProtoFunc* Main, int at)
{
 int size=CodeSize(tf);
 if (IsMain(tf))
  printf("\nmain of \"%s\" (%d bytes at %p)\n",tf->fileName->str,size,tf);
 else if (Main)
 {
  printf("\nfunction defined at \"%s\":%d (%d bytes at %p); used at ",
	tf->fileName->str,tf->lineDefined,size,tf);
  if (IsMain(Main))
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
 while (1)
 {
  Opcode OP;
  int n=INFO(Main,p,&OP);
  if (OP.class==ENDCODE) break;
  if (OP.class==PUSHCONSTANT || OP.class==CLOSURE)
  {
   int i=OP.arg;
   TObject* o=Main->consts+i;
   if (ttype(o)==LUA_T_PROTO) PrintFunction(tfvalue(o),Main,(int)(p-code));
  }
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

void PrintChunk(TProtoFunc* Main)
{
 PrintFunction(Main,0,0);
}
