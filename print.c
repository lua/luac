/*
** $Id: print.c,v 1.33 2001/03/15 17:29:16 lhf Exp lhf $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>

#include "luac.h"
#include "ldebug.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"

/* macros used in print.h, included in PrintCode */
#define P_OP(x)	printf("%-11s\t",x)
#define P_NONE
#define P_AB	printf("%d %d",GETARG_A(i),GETARG_B(i))
#define P_F	printf("%d %d\t; %p",GETARG_A(i),GETARG_B(i),tf->kproto[GETARG_A(i)])
#define P_J	printf("%d\t; to %d",GETARG_S(i),GETARG_S(i)+pc+2)
#define P_Q	PrintString(tf,GETARG_U(i))
#define P_K	printf("%d\t; %s",GETARG_U(i),getstr(tf->kstr[GETARG_U(i)]))
#define P_L	PrintLocal(tf,GETARG_U(i),pc)
#define P_N	printf("%d\t; " LUA_NUMBER_FMT,GETARG_U(i),tf->knum[GETARG_U(i)])
#define P_S	printf("%d",GETARG_S(i))
#define P_U	printf("%u",GETARG_U(i))

static void PrintString(const Proto* tf, int n)
{
 const char* s=getstr(tf->kstr[n]);
 printf("%d\t; ",n);
 putchar('"');
 for (; *s; s++)
 {
  switch (*s)
  {
   case '"': printf("\\\""); break;
   case '\a': printf("\\a"); break;
   case '\b': printf("\\b"); break;
   case '\f': printf("\\f"); break;
   case '\n': printf("\\n"); break;
   case '\r': printf("\\r"); break;
   case '\t': printf("\\t"); break;
   case '\v': printf("\\v"); break;
   default: putchar(*s); break;
  }
 }
 putchar('"');
}

static void PrintLocal(const Proto* tf, int n, int pc)
{
 const char* s=luaF_getlocalname(tf,n+1,pc); 
 printf("%u",n);
 if (s!=NULL) printf("\t; %s",s);
}

static void PrintCode(const Proto* tf)
{
 const Instruction* code=tf->code;
 int pc,n=tf->sizecode;
 for (pc=0; pc<n; pc++) 
 {
  Instruction i=code[pc];
  int line=luaG_getline(tf->lineinfo,pc,1,NULL);
#if 0
  printf("%0*lX",Sizeof(i)*2,i);
#endif
  printf("\t%d\t",pc+1);
  if (line>=0) printf("[%d]\t",line); else printf("[-]\t");
  switch (GET_OPCODE(i))
  {
#include "print.h"
  }
  printf("\n");
 }
}

#define IsMain(tf)	(tf->lineDefined==0)

#define SS(x)	(x==1)?"":"s"
#define S(x)	x,SS(x)

static void PrintHeader(const Proto* tf)
{
 printf("\n%s <%d:%s> (%d instruction%s, %d bytes at %p)\n",
 	IsMain(tf)?"main":"function",tf->lineDefined,getstr(tf->source),
	S(tf->sizecode),tf->sizecode*Sizeof(Instruction),tf);
 printf("%d%s param%s, %d stack%s, ",
	tf->numparams,tf->is_vararg?"+":"",SS(tf->numparams),S(tf->maxstacksize));
 printf("%d local%s, %d string%s, %d number%s, %d function%s, %d line%s\n",
	S(tf->sizelocvars),S(tf->sizekstr),S(tf->sizeknum),S(tf->sizekproto),S(tf->sizelineinfo));
}

#define PrintFunction luaU_printchunk

void PrintFunction(const Proto* tf)
{
 int i,n=tf->sizekproto;
 PrintHeader(tf);
 PrintCode(tf);
 for (i=0; i<n; i++) PrintFunction(tf->kproto[i]);
}
