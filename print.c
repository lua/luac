/*
** $Id: print.c,v 1.13 1998/07/12 00:17:37 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#ifdef DEBUG
void PrintConstant1(TProtoFunc* tf, int i)
{
 TObject* o=tf->consts+i;
 printf("%6d ",i);
 if (i<0 || i>=tf->nconsts)
  printf("(bad constant #%d: max=%d)",i,tf->nconsts);
 else
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	printf("N " NUMBER_FMT "\n",nvalue(o));
	break;
   case LUA_T_STRING:
	printf("S %p\t\"%s\"\n",(void*)tsvalue(o),svalue(o));
	break;
   case LUA_T_PROTO:
	printf("F %p\n",(void*)tfvalue(o));
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
 if (i<0 || i>=tf->nconsts)
  printf("(bad constant #%d: max=%d)",i,tf->nconsts);
 else
 {
  TObject* o=tf->consts+i;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	printf(NUMBER_FMT,nvalue(o));
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
	printf("(bad constant #%d: type=%d [%s])\n",i,ttype(o),luaO_typename(o));
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
	case SETLOCALDUP:
	{
		char* s=luaF_getlocalname(tf,i+1,line);
		if (s) printf("\t; %s",s);
		break;
	}

	case GETGLOBAL:
	case SETGLOBAL:
	case SETGLOBALDUP:
		printf("\t; %s",VarStr(i));
		break;

	case SETLINE:
		printf("\t; \"%s\":%d",tf->source->str,line=i);
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

static void PrintLocals(TProtoFunc* tf)
{
 LocVar* v=tf->locvars;
 int n,i=0;
 if (v==NULL || v->varname==NULL) return;
 n=tf->code[1]; if (n>=ZEROVARARG) n-=ZEROVARARG;

 printf("locals:");
 if (n>0)
 {
  for (i=0; i<n; v++,i++) printf(" %s",v->varname->str);
 }
 if (v->varname!=NULL)
 {
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
 }
 printf("\n");
}

#define IsMain(f)	(f->lineDefined==0)

static void PrintHeader(TProtoFunc* tf, TProtoFunc* Main, int at)
{
 int size=luaU_codesize(tf);
 if (IsMain(tf))
  printf("\nmain of \"%s\" (%d bytes at %p)\n",tf->source->str,size,(void*)tf);
 else if (Main)
 {
  printf("\nfunction defined at \"%s\":%d (%d bytes at %p); used at ",
	tf->source->str,tf->lineDefined,size,(void*)tf);
  if (IsMain(Main))
   printf("main");
  else
   printf("%p",(void*)Main);
  printf("+%d\n",at);
 }
}

static void PrintFunction(TProtoFunc* tf, TProtoFunc* Main, int at);

static void PrintFunctions(TProtoFunc* Main)
{
 Byte* code=Main->code;
 Byte* p=code;
 int longarg;
 while (1)
 {
  Opcode OP;
  int n=INFO(Main,p,&OP);
  if (OP.class==PUSHCONSTANT || OP.class==CLOSURE)
  {
   int i=OP.arg+longarg;
   TObject* o=Main->consts+i;
   if (ttype(o)==LUA_T_PROTO) PrintFunction(tfvalue(o),Main,(int)(p-code));
  }
  else if (OP.op==ENDCODE) break;
  else if (OP.op==LONGARG) longarg=OP.arg<<16;
  else longarg=0;
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
