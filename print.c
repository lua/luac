/*
** $Id: print.c,v 1.27 2000/04/24 17:32:29 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#define DEBUG
#undef DEBUG

#ifdef DEBUG
static void PrintLocvars(const Proto* tf)
{
 LocVar* v=tf->locvars;
 int i=0;
 int n=-1;
 if (v==NULL) return;
 printf("locvars for %p at %p:\n",tf,v);
 printf("%6s\t#\tL\tN\tP\n","I");
 do
 {
  if (v->varname==NULL) --n; else ++n;
  printf("%6d\t%d\t%d\t%s\t%p\n",
	i++,n,v->line,v->varname?v->varname->str:"-",v->varname);
 } while (v++->line>=0);
}
#endif

#define P_OP(x)	printf("%-11s  ", x)
#define P_NONE
#define P_LINE	printf("%u", line=GETARG_U(i))
#define P_U	printf("%u", GETARG_U(i))
#define P_S	printf("%d", GETARG_S(i))
#define P_AB	printf("%d %d", GETARG_A(i), GETARG_B(i))
#define P_K	printf("%d\t; %s", GETARG_U(i),tf->kstr[GETARG_U(i)]->str)
#define P_J	printf("%d\t; to %d", GETARG_S(i),GETARG_S(i)+at+1)
#define P_N	printf("%d\t; %g", GETARG_U(i),tf->knum[GETARG_U(i)])
#define P_F	printf("%d %d\t; %p", GETARG_A(i), GETARG_B(i),tf->kproto[GETARG_A(i)])
#define	P_L	P_U
#define	P_LB	P_AB
#define P_sAL	printf("%d %d", GETARG_sA(i), GETARG_B(i))

static void PrintCode(const Proto* tf)
{
 const Instruction* code=tf->code;
 const Instruction* p=code;
 int line=0;
 for (;;)
 {
  int at=p-code;
  Instruction i=*p;
#if 1
  printf("%6d  %08lX  ",at,i);
#else
  printf("%6d  ",at);
#endif
  switch (GET_OPCODE(i)) {
#include "print.h"
  }
  printf("\n");
  if (i==OP_END) break;
  p++;
 }
}

static void PrintLocals(const Proto* tf)
{
 const LocVar* v=tf->locvars;
 int n,i;
 if (v==NULL || v->line<0) return;
 n=tf->numparams;
 printf("locals:");
 for (i=0; i<n; v++,i++)		/* arguments */
  printf(" %s",v->varname->str);
 if (tf->is_vararg)
 {
  printf(" [%s]",v->varname->str);
  v++;
 }
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

#define SS(x)	(x==1)?"":"s"
#define S(x)	x,SS(x)

static void PrintHeader(const Proto* tf, const Proto* Main, int at)
{
 int size=luaU_codesize(tf);
 if (IsMain(tf))
  printf("\nmain " SOURCE " (%d instructions/%d bytes at %p)\n",
	tf->source->str,tf->lineDefined,size,size*sizeof(Instruction),tf);
 else
 {
  printf("\nfunction " SOURCE " (%d instructions/%d bytes at %p); used at ",
	tf->source->str,tf->lineDefined,size,size*sizeof(Instruction),tf);
  if (Main && IsMain(Main))
   printf("main");
  else
   printf("%p",Main);
  printf("+%d\n",at);
 }
 printf("has %d%s param%s, uses %d stack position%s, ",
	tf->numparams,tf->is_vararg?"+":"",SS(tf->numparams),S(tf->maxstacksize));
 printf("%d string%s, %d number%s, %d function%s\n",
	S(tf->nkstr),S(tf->nknum),S(tf->nkproto));
}

static void PrintFunction(const Proto* tf, const Proto* Main, int at);

static void PrintFunctions(const Proto* Main)
{
 int i,n=Main->nkproto;
 for (i=0; i<n; i++) PrintFunction(Main->kproto[i],Main,i);
}

static void PrintFunction(const Proto* tf, const Proto* Main, int at)
{
 PrintHeader(tf,Main,at);
#ifdef DEBUG
 PrintLocvars(tf);
#endif
 PrintLocals(tf);
 PrintCode(tf);
 PrintFunctions(tf);
}

#define INDENT	printf("%3d  %*s",level,level,"");

static void InfoFunction(const Proto* tf, int level)
{
 INDENT printf("%p\n",tf);
 INDENT printf("  %d lineDefined\n",tf->lineDefined);
 INDENT printf("  [%s] source\n",tf->source->str);
 INDENT printf("  %d numparams\n",tf->numparams);
 INDENT printf("  %d is_vararg\n",tf->is_vararg);
 INDENT printf("  %d maxstacksize\n",tf->maxstacksize);
 INDENT printf("  %d codesize\n",luaU_codesize(tf));
 INDENT printf("  %d strings\n",tf->nkstr);
 INDENT printf("  %d numbers\n",tf->nknum);
 INDENT printf("  %d protos\n",tf->nkproto);
 {
  int i,n=tf->nkproto;
  ++level;
  for (i=0; i<n; i++) InfoFunction(tf->kproto[i],level);
 }
}

void luaU_printchunk(const Proto* Main)
{
#if 0
 InfoFunction(Main,0);
#endif
 PrintFunction(Main,0,0);
}

int luaU_codesize(const Proto* tf)
{
 const Instruction* code=tf->code;
 const Instruction* p=code;
 while (*p++!=OP_END);
 return p-code;
}
