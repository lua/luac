/*
** $Id: print.c,v 1.29 2000/09/18 20:03:46 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#define DEBUG
#undef DEBUG

#define P_OP(x)	printf("%-11s  ", x)
#define P_NONE
#define P_U	printf("%u", GETARG_U(i))
#define P_S	printf("%d", GETARG_S(i))
#define P_AB	printf("%d %d", GETARG_A(i), GETARG_B(i))
#define P_K	printf("%d\t; %s", GETARG_U(i),tf->kstr[GETARG_U(i)]->str)
#define P_J	printf("%d\t; to %d", GETARG_S(i),GETARG_S(i)+at+1)
#define P_N	printf("%d\t; %g", GETARG_U(i),tf->knum[GETARG_U(i)])
#define P_F	printf("%d %d\t; %p", GETARG_A(i), GETARG_B(i),tf->kproto[GETARG_A(i)])
#define	P_L	printf("%u\t; %s", GETARG_U(i), tf->locvars ? tf->locvars[GETARG_U(i)].varname->str : "")
#define	P_LB	P_AB
#define P_sAL	printf("%d %d", GETARG_sA(i), GETARG_B(i))

static void PrintCode(const Proto* tf)
{
 const Instruction* code=tf->code;
 const Instruction* p=code;
 for (;;)
 {
  int at=p-code+1;
  Instruction i=*p;
#ifdef SHOWHEX
  printf("%6d  %08lX  ",at,i);
#else
  printf("%6d    ",at);
#endif
  switch (GET_OPCODE(i)) {
#include "print.h"
  }
  printf("\n");
  if (i==OP_END) break;
  p++;
 }
}

#if 0
static void PrintLocals(const Proto* tf)
{
 const LocVar* v=tf->locvars;
 int n,i;
 if (v==NULL) return;
 n=tf->numparams;
 printf("locals:");
 for (i=0; i<n; v++,i++)		/* arguments */
  printf(" %s",v->varname->str);
 if (tf->is_vararg)
 {
  printf(" [%s]",v->varname->str);
  v++;
 }
 for (; v->pc>=0; v++)
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
#endif

#define IsMain(tf)	(tf->lineDefined==0)

#define SS(x)	(x==1)?"":"s"
#define S(x)	x,SS(x)

static void PrintHeader(const Proto* tf)
{
 int size=luaU_codesize(tf);
 printf("\n%s " SOURCE " (%d instruction%s/%d bytes at %p)\n",
 	IsMain(tf)?"main":"function",
	tf->source->str,tf->lineDefined,S(size),size*Sizeof(Instruction),tf);
 printf("%d%s param%s, %d stack position%s, ",
	tf->numparams,tf->is_vararg?"+":"",SS(tf->numparams),S(tf->maxstacksize));
 printf("%d local%s, %d string%s, %d number%s, %d function%s\n",
	S(tf->nlocvars),S(tf->nkstr),S(tf->nknum),S(tf->nkproto));
}

#define PrintFunction luaU_printchunk

void PrintFunction(const Proto* tf)
{
 int i,n=tf->nkproto;
 PrintHeader(tf);
#ifdef DEBUG
 PrintLocals(tf);
#endif
 PrintCode(tf);
 for (i=0; i<n; i++) PrintFunction(tf->kproto[i]);
}
