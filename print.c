/*
** $Id: print.c,v 1.2 1997/12/02 23:18:50 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#include "print.h"

static void PrintConstants(TFunc* tf)
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
#ifdef DEBUG
   default:		printf("? %d",ttype(o)); break;
#else
   default: break;
#endif
  }
 }
 printf("\n");
}

static void PrintConstant(TFunc* tf, int n)
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
#ifdef DEBUG
   default:		printf("cannot handle ttype(o)=%d",ttype(o)); break;
#else
   default: break;
#endif
  }
 }
}

#define VarStr(i)       svalue(tf->consts+i)

static void PrintCode(TFunc* tf)
{
 Byte* code=tf->code+1;
 Byte* p=code;
 int line=0;
 while (1)
 {
	int op=*p;
	int n,i;
#ifdef DEBUG
	if (op>=NOPCODES)		/* cannot happen */
	 luaL_verror("internal error in PrintCode: "
		"bad opcode %d at %d (NOPCODES=%d)\n",op,(int)(p-code),NOPCODES);
#endif
	n=OpcodeSize[op];
	printf("%6d  ",(int)(p-code));
	{
	 Byte* q=p;
	 i=n;
	 while (i--) printf("%02X",*q++);
	}
	printf("\t%-13s",OpcodeName[op]);
	if (n==1) i=-1; else if (n==2) i=p[1]; else i=p[1]+(p[2]<<8);
	if (n!=1 && op!=SETLIST && op!=CALLFUNC) printf("\t%d",i);

	switch (op)
	{

	case ENDCODE:
#if 0
		printf("\t(%d bytes of code)",(int)(p-code)+1);
#endif
		printf("\n");
		return;

	case PUSHCONSTANT0:
	case PUSHCONSTANT1:
	case PUSHCONSTANT2:
	case PUSHCONSTANT3:
	case PUSHCONSTANT4:
	case PUSHCONSTANT5:
	case PUSHCONSTANT6:
	case PUSHCONSTANT7:
		i=op-PUSHCONSTANT0;
		printf("\t");
	case PUSHCONSTANT:
	case PUSHCONSTANTW:
		goto PRINTCONSTANT;

	case GETDOTTED0:
	case GETDOTTED1:
	case GETDOTTED2:
	case GETDOTTED3:
	case GETDOTTED4:
	case GETDOTTED5:
	case GETDOTTED6:
	case GETDOTTED7:
		i=op-GETDOTTED0;
		printf("\t");
	case GETDOTTED:
	case GETDOTTEDW:
		goto PRINTCONSTANT;

	case PUSHSELF0:
	case PUSHSELF1:
	case PUSHSELF2:
	case PUSHSELF3:
	case PUSHSELF4:
	case PUSHSELF5:
	case PUSHSELF6:
	case PUSHSELF7:
		i=op-PUSHSELF0;
		printf("\t");
	case PUSHSELF:
	case PUSHSELFW:
		goto PRINTCONSTANT;

	case CLOSURE:
PRINTCONSTANT:
		printf("\t; ");
		PrintConstant(tf,i);
		break;

	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
		i=op-PUSHLOCAL0;
		printf("\t");
	case PUSHLOCAL:
		goto PRINTLOCAL;

	case SETLOCAL0:
	case SETLOCAL1:
	case SETLOCAL2:
	case SETLOCAL3:
	case SETLOCAL4:
	case SETLOCAL5:
	case SETLOCAL6:
	case SETLOCAL7:
		i=op-SETLOCAL0;
		printf("\t");
	case SETLOCAL:
PRINTLOCAL:
	{
		char* s=luaF_getlocalname(tf,i+1,line);
		if (s) printf("\t; %s",s);
		break;
	}

	case GETGLOBAL0:
	case GETGLOBAL1:
	case GETGLOBAL2:
	case GETGLOBAL3:
	case GETGLOBAL4:
	case GETGLOBAL5:
	case GETGLOBAL6:
	case GETGLOBAL7:
		i=op-GETGLOBAL0;
		printf("\t");
	case GETGLOBAL:
	case GETGLOBALW:
		goto PRINTGLOBAL;

	case SETGLOBAL0:
	case SETGLOBAL1:
	case SETGLOBAL2:
	case SETGLOBAL3:
	case SETGLOBAL4:
	case SETGLOBAL5:
	case SETGLOBAL6:
	case SETGLOBAL7:
		i=op-SETGLOBAL0;
		printf("\t");
	case SETGLOBAL:
	case SETGLOBALW:

PRINTGLOBAL:
		printf("\t; %s",VarStr(i));
		break;

/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
	case ONTJMP:
	case ONFJMP:
	case JMPW:
	case JMP:
	case IFFJMPW:
	case IFFJMP:
		printf("\t; to %d",(int)(p-code)+i+n);
		break;

	case IFTUPJMPW:
	case IFTUPJMP:
	case IFFUPJMPW:
	case IFFUPJMP:
		printf("\t; to %d",(int)(p-code)-i+n);
		break;

	case SETLIST:
	case CALLFUNC:
		printf("\t%d %d",p[1],p[2]);
		break;

	case SETLINE:
	case SETLINEW:
		printf("\t; \"%s\":%d",tf->fileName->str,line=i);
		break;

#if 0
	default:			/* cannot happen */
		printf("\n");
	 	luaL_verror("internal error in PrintCode: "
			"bad opcode %d at %d\n",*p,(int)(p-code));
		break;
#endif

	}
	printf("\n");
	p+=n;
 }
}

static void PrintLocals(TFunc* tf)
{
 LocVar* v=tf->locvars;
 int n=0;
 int i=0;
 Byte* p;
 if (v==NULL || v->varname==NULL) return;
 for (p=tf->code+1;;)
  if (*p==SETLINE) p+=2; else if (*p==SETLINEW) p+=3; else break;
 if (*p==ARGS || *p==VARARGS) n=p[1]; else
 if (*p>ARGS && *p<VARARGS) n=*p-ARGS0;
 printf("PrintLocals: %d %s n=%d\n",(int)(p-tf->code)-1,OpcodeName[*p],n);

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

static Byte* FindFunction(TFunc* tf, TFunc* Main)
{
 Byte* code=Main->code+1;
 Byte* p=code;
 while (1)
 {
  int op=*p;
  int i=-1;
#ifdef DEBUG
  if (op>=NOPCODES)			/* cannot happen */
   luaL_verror("internal error in FindFunction: bad opcode %d at %d\n",
	op,(int)(p-code));
#endif
  if (op==ENDCODE) break;
  else if (op==PUSHCONSTANT) i=p[1];
  else if (op>PUSHCONSTANT && op<PUSHCONSTANTW) i=op-PUSHCONSTANT0;
  p+=OpcodeSize[op];
  if (i>=0)
  {
   TObject* o=Main->consts+i;
   if (ttype(o)==LUA_T_PROTO && tfvalue(o)==tf) return p+1;
  }
 }
 return NULL;				/* to avoid warnings */
}

int CodeSize(TFunc* tf)
{
 Byte* code=tf->code+1;
 Byte* p=code;
 while (1)
 {
  int op=*p;
#ifdef DEBUG
  if (op>=NOPCODES)			/* cannot happen */
   luaL_verror("internal error in CodeSize: bad opcode %d at %d\n",
	op,(int)(p-code));
#endif
  p+=OpcodeSize[op];
  if (op==ENDCODE) break;
 }
 return p-code;
}

#undef VarStr
#define VarStr(i)       svalue(Main->consts+i)

static void PrintHeader(TFunc* tf, TFunc* Main)
{
 int size=CodeSize(tf);
 if (IsMain(tf))
  printf("\nmain of \"%s\" (%d bytes at %p)\n",tf->fileName->str,size,tf);
 else if (Main)
 {
  Byte* p=FindFunction(tf,Main);
  Byte* op=p;
  printf("\nfunction ");
  if (p)
  switch (*p)				/* try to get name */
  {
   int i;
   case SETGLOBAL0:
   case SETGLOBAL1:
   case SETGLOBAL2:
   case SETGLOBAL3:
   case SETGLOBAL4:
   case SETGLOBAL5:
   case SETGLOBAL6:
   case SETGLOBAL7:
    i=*p-SETGLOBAL0;
    goto PRINTGLOBAL;
   case SETGLOBAL:
    i=p[1];
    goto PRINTGLOBAL;
   case SETGLOBALW:
    i=p[1]+(p[2]<<8);
PRINTGLOBAL:
    printf("%s ",VarStr(i));
    break;
   case SETTABLE0:			/* try method definition */
   {
    int t=-1,m=-1;
    if (p[-3]>PUSHCONSTANT && p[-3]<PUSHCONSTANTW
     && p[-4]>GETGLOBAL && p[-4]<GETGLOBALW)
    {
     t=p[-4]-GETGLOBAL-1;
     m=p[-3]-PUSHCONSTANT-1;
#if 0
printf("t=%d m=%d\n",t,m);
#endif
    }
    if (t>=0 && m>=0)
    {
     int c=(tf->locvars && tf->locvars->varname &&
     strcmp(tf->locvars->varname->str,"self")==0)
		? ':' : '.';
     printf("%s%c%s ",VarStr(t),c,VarStr(m));
    }
    break;
   }
  }
  printf("defined at \"%s\":%d (%d bytes at %p); used at ",
	tf->fileName->str,tf->lineDefined,size,tf);
  if (IsMain(Main))
   printf("main");
  else
   printf("%p",Main);
  printf("+%d\n",(int)(op-Main->code-3));
 }
#if 0
 printf("needs %d stack positions\n",*tf->code);
#endif
}

static void PrintFunction(TFunc* tf, TFunc* Main);

static void PrintFunctions(TFunc* tf)
{
 int i,n=tf->nconsts;
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  if (ttype(o)==LUA_T_PROTO) PrintFunction(tfvalue(o),tf);
 }
}

static void PrintFunction(TFunc* tf, TFunc* Main)
{
 PrintHeader(tf,Main);
 PrintLocals(tf);
 PrintCode(tf);
#if 0
 PrintConstants(tf);
#endif
 PrintFunctions(tf);
}

void PrintChunk(TFunc* Main)
{
 PrintFunction(Main,0);
}
