/*
** $Id: print.c,v 1.6 1998/01/19 16:10:52 lhf Exp lhf $
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
 Byte* code=tf->code;
 Byte* p=code+2;			/* skip headers bytes */
 while (1)
 {
  int op=*p;
  CHECKOP(op,p,code,tf,"CodeSize");
  p+=Opcode[op].size;
  if (op==ENDCODE) break;
 }
 return p-code;
}

#ifdef DEBUG
void PrintConstant1(TProtoFunc* tf, int i)
{
 TObject* o=tf->consts+i;
 printf("%6d ",i,o->value.i);
 switch (ttype(o))
 {
  case LUA_T_NUMBER:
	printf("N %g\n",(double)nvalue(o));	/* LUA_NUMBER */
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
 int i;
 int n=tf->nconsts;
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
   case LUA_T_NUMBER:	printf("%g",(double)nvalue(o)); break;	/* LUA_NUMBER */
   case LUA_T_STRING:	printf("\"%s\"",svalue(o)); break;
   case LUA_T_PROTO:	printf("function at %p",tfvalue(o)); break;
   default:				/* cannot happen */
#ifdef DEBUG
	luaL_verror("internal error in PrintConstant: "
		"bad constant #%d type=%d\n",i,ttype(o));
#endif
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
 printf("%6d  %02X%*s%-13s\t%d\n",(int)(p-code),*p,8,"","STACK",*p); p++;
 printf("%6d  %02X%*s",(int)(p-code),*p,8,"");
 if (*p>=ZEROVARARG)
  printf("%-13s\t%d\n","VARARGS",*p-ZEROVARARG);
 else
  printf("%-13s\t%d\n","ARGS",*p);
 p++;
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
	if (n!=1) printf("\t%d",i); else if (i>=0) printf("\t");

	switch (Opcode[op].class)
	{

	case ENDCODE:
		printf("\n");
		return;

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
		if (n>=3) printf(" %d",p[n-1]);
		break;

	case CALLFUNC:
		if (n==3) printf(" %d",p[2]);
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

static Byte* FindFunction(TProtoFunc* tf, TProtoFunc* Main)
{
 Byte* code=Main->code;
 Byte* p=code+2;			/* skip headers bytes */
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
  printf("+%d\n",(int)(p-Main->code));
 }
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
#ifdef DEBUG
 PrintConstants(tf);
#endif
 PrintFunctions(tf);
}

void PrintChunk(TProtoFunc* Main)
{
 PrintFunction(Main,0);
}
