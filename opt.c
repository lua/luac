/*
** $Id$
** optimize bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#include "print.h"

#if 0
#define DEBUG
#else
#endif

static void PrintConstant1(TProtoFunc* tf, int i)
{
 TObject* o=tf->consts+i;
#if 0
 printf("%6d %d ",i,o->value.i);
#else
 printf("%6d ",i,o->value.i);
#endif
 switch (ttype(o))
 {
  case LUA_T_NUMBER:	printf("N %g",(double)nvalue(o)); break;/* LUA_NUMBER */
  case LUA_T_STRING:	printf("S %p\t\"%s\"",tsvalue(o),svalue(o)); break;
  case LUA_T_PROTO:	printf("F %p",tfvalue(o)); break;
  default:				/* cannot happen */
		       printf("? %d",ttype(o)); 
  break;
 }
 printf("\n");
}

static TProtoFunc* TF;
#define N 20000

static int compare(const void* a, const void *b)
{
 int ia=*(int*)a;
 int ib=*(int*)b;
 int t;
 TObject* oa=TF->consts+ia;
 TObject* ob=TF->consts+ib;
 t=ttype(oa)-ttype(ob);		if (t) return t;
 t=oa->value.i-ob->value.i;	if (t) return t;
 return ia-ib;
}

static void FixConstants(TProtoFunc* tf, int* C)
{
 Byte* code=tf->code;
 Byte* p=code+2;			/* skip headers bytes */
 while (1)
 {
  int op=*p;
  int n,i;
  if (op==ENDCODE) break;
  n=Opcode[op].size;
  if (n==1) i=Opcode[op].arg; else if (n==2) i=p[1]; else i=p[1]+(p[2]<<8);
  op=Opcode[op].class;
#ifdef PACK
  if (op==SETLINE) memset(p,NOP,n); else
#endif
  if (	op==PUSHCONSTANT || op==GETDOTTED || op==PUSHSELF ||
	op==GETGLOBAL    || op==SETGLOBAL)
  {
   int j=C[i];
   if (j==i)
    ;
   else if (n==1)
   {
    p[0]=op+j+1;
   }
   else if (n==2)
   {
#ifdef PACK
    if (j<8) { p[0]=op+j+1; p[1]=NOP; } else p[1]=j;
#else
    p[1]=j;
#endif
   }
   else
   {
#ifdef PACK
    if (j<255)
    {
     p[0]=op;
     p[1]=j;
     p[2]=NOP;
    }
    else 
#endif
    {
     p[1]= 0x0000FF &  j;
     p[2]= 0x0000FF & (j>>8);
    }
   }
  }
  p+=n;
 }
}

void OptConstants(TProtoFunc* tf)
{
 static int C[N];
 static int D[N];
 int i,k;
 int n=tf->nconsts;
 if (n>=N) luaL_verror("SortConstants: too many constants (%d > %d)",n,N);
 for (i=0; i<n; i++) C[i]=i;
 for (i=0; i<n; i++) D[i]=i;
 TF=tf;
 qsort(C,n,sizeof(C[0]),compare);
#ifdef DEBUG
 printf("after sort:\n");
 for (i=0; i<n; i++) PrintConstant1(tf,C[i]);
#endif
 k=C[0];
 for (i=1; i<n; i++)
 {
  int j=C[i];
  TObject* oa=tf->consts+k;
  TObject* ob=tf->consts+j;
  if (ttype(oa)==ttype(ob) && oa->value.i==ob->value.i) D[j]=k; else k=j;
 }
#ifdef DEBUG
 printf("duplicates:\n");
 for (i=0; i<n; i++) if (D[i]!=i) printf("%6d = %d\n",i,D[i]);
#endif
 k=0;
 for (i=0; i<n; i++)
 {
  if (D[i]==i) { tf->consts[k]=tf->consts[i]; C[i]=k++; } else C[i]=C[D[i]];
 }
 if (k>=n) return;
#ifdef DEBUG
 printf("pack map:\n");
 for (i=0; i<n; i++) printf("%6d = %d\n",i,C[i]);
 printf("after pack (%d/%d):\n",k,n);
 for (i=0; i<k; i++) PrintConstant1(tf,i);
#endif
 tf->nconsts=k;
 FixConstants(tf,C);
}

static void OptFunction(TProtoFunc* tf);

static void OptFunctions(TProtoFunc* tf)
{
 int i,n=tf->nconsts;
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  if (ttype(o)==LUA_T_PROTO) OptFunction(tfvalue(o));
 }
}

static void OptFunction(TProtoFunc* tf)
{
 OptConstants(tf);
 OptFunctions(tf);
}

void OptChunk(TProtoFunc* Main)
{
 OptFunction(Main);
}
