/*
** $Id: print.c,v 1.4 1998/01/12 13:04:24 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#include "print.h"

#ifdef DEBUG
#define CHECKOP(op,p,code,tf,s) \
if (op>=NOPCODES)			/* cannot happen */\
	luaL_verror("internal error in %s: bad opcode %d at %d in tf=%p\n",\
		s,op,(int)(p-code),tf)
#else
#define CHECKOP(op,p,code,tf,s)
#endif

int CodeSize(TProtoFunc* tf)		/* also used in dump.c */
{
 Byte* code=tf->code+2;
 Byte* p=code;
 while (1)
 {
  int op=*p;
  CHECKOP(op,p,code,tf,"CodeSize");
  p+=Opcode[op].size;
  if (op==ENDCODE) break;
 }
 return p-code;
}

static void PrintConstants(TProtoFunc* tf)
{
 int i;
 int n=tf->nconsts;
 printf("constants (%d):",n);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  printf("\n\t%d ",i);
  switch (ttype(o))
  {
   case LUA_T_NUMBER:	printf("N %g",nvalue(o)); break;
   case LUA_T_STRING:	printf("S \"%s\"",svalue(o)); break;
   case LUA_T_PROTO:	printf("F %p",tfvalue(o)); break;
   default:				/* cannot happen */
#ifdef DEBUG
			printf("? %d",ttype(o)); 
#endif
   break;
  }
 }
 printf("\n");
}

static void PrintConstant(TProtoFunc* tf, int n)
{
 if (n<0 || n>=tf->nconsts)
 {
  printf("(bad constant)");
 }
 else
 {
  TObject* o=tf->consts+n;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:	printf("%g",nvalue(o)); break;
   case LUA_T_STRING:	printf("\"%s\"",svalue(o)); break;
   case LUA_T_PROTO:	printf("function at %p",tfvalue(o)); break;
   default:				/* cannot happen */
#ifdef DEBUG
	luaL_verror("internal error in PrintConstant: "
		"bad constant #%d type=%d\n",n,ttype(o));
#endif
   break;
  }
 }
}

#define VarStr(i)       svalue(tf->consts+i)

static void PrintCode(TProtoFunc* tf)
{
 Byte* code=tf->code+2;
 Byte* p=code;
 int line=0;
 while (1)
 {
	int op=*p;
	int n,i;
	CHECKOP(op,p,code,tf,"PrintCode");
	n=Opcode[op].size;
	printf("%6d  ",(int)(p-code));
	{
	 Byte* q=p;
	 int i=n;
	 while (i--) printf("%02X",*q++);
	}
	printf("%*s%-13s",2*(5-n),"",Opcode[op].name);

	if (op==CALLFUNC || op==SETLIST) i=p[1];
	else if (n==1) i=Opcode[op].arg;
	else if (n==2) i=p[1];
	else i=p[1]+(p[2]<<8);
	op=Opcode[op].class;
	if (n!=1) printf("\t%d",i); else if (i>=0) printf("\t");

	switch (op)
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
		printf("\t; %s",VarStr(i));
		break;

/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case IFFJMP:
		printf("\t; to %d",(int)(p-code)+i+n);
		break;

	case IFTUPJMP:
	case IFFUPJMP:
		printf("\t; to %d",(int)(p-code)-i+n);
		break;

	case SETLIST:
		if (n>=3) printf(" %d",p[n-1]);
		break;

	case CALLFUNC:
		if (n==3) printf(" %d",p[2]);
		break;

	case SETLINE:
		printf("\t; \"%s\":%d",tf->fileName->str,line=i);
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

static Byte* FindFunction(TProtoFunc* tf, TProtoFunc* Main)
{
 Byte* code=Main->code+2;
 Byte* p=code;
 while (1)
 {
  int op=*p;
  int n,i;
  CHECKOP(op,p,code,tf,"FindFunction");
  if (op==ENDCODE) break;
  n=Opcode[op].size;
  if (n==1) i=Opcode[op].arg; else if (n==2) i=p[1]; else i=p[1]+(p[2]<<8);
  op=Opcode[op].class;
  if (op==PUSHCONSTANT)
  {
   TObject* o=Main->consts+i;
   if (ttype(o)==LUA_T_PROTO && tfvalue(o)==tf) return p;
  }
  p+=n;
 }
#ifdef DEBUG
 luaL_verror("internal error in FindFunction: function %p not found\n",tf);
#endif
 return NULL;				/* avoid warnings */
}

static void PrintHeader(TProtoFunc* tf, TProtoFunc* Main)
{
 int size=CodeSize(tf);
 if (IsMain(tf))
  printf("\nmain of \"%s\" (%d bytes at %p)\n",tf->fileName->str,size,tf);
 else if (Main)
 {
  Byte* p=FindFunction(tf,Main);
  printf("\nfunction defined at \"%s\":%d (%d bytes at %p); used at ",
	tf->fileName->str,tf->lineDefined,size,tf);
  if (IsMain(Main))
   printf("main");
  else
   printf("%p",Main);
  printf("+%d\n",(int)(p-Main->code-2));
 }
#if 0
 printf("needs %d stack positions\n",*tf->code);
#endif
}

static void PrintFunction(TProtoFunc* tf, TProtoFunc* Main);

static void PrintFunctions(TProtoFunc* tf)
{
 int i,n=tf->nconsts;
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  if (ttype(o)==LUA_T_PROTO) PrintFunction(tfvalue(o),tf);
 }
}

static void PrintFunction(TProtoFunc* tf, TProtoFunc* Main)
{
 PrintHeader(tf,Main);
 PrintLocals(tf);
 PrintCode(tf);
#if 0
 PrintConstants(tf);
#endif
 PrintFunctions(tf);
}

void PrintChunk(TProtoFunc* Main)
{
 PrintFunction(Main,0);
}
