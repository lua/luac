/*
** $Id: opt.c,v 1.4 1998/04/02 20:44:08 lhf Exp lhf $
** optimize bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"
#include "lmem.h"

static void FixConstants(TProtoFunc* tf, int* C)
{
 Byte* code=tf->code;
 Byte* p=code;
 int m=0;				/* size of last LONGARG */
 int longarg=0;
 while (1)
 {
  Opcode OP;
  int n=INFO(tf,p,&OP);
  int op=OP.class;
  int i=OP.arg+longarg;
  if (op==ENDCODE) break;
  else if (op==LONGARG) { m=n; longarg=i<<16; }
  else if (op==PUSHCONSTANT || op==GETGLOBAL || op==GETDOTTED    ||
	   op==PUSHSELF     || op==SETGLOBAL || op==SETGLOBALDUP || op==CLOSURE)
  {
   int j=C[i];
   if (j==i)
    ;
   else if (n==2)
   {
    p[1]=j;
   }
   else
   {
    if (j<=MAX_BYTE)
    {
     p[0]=op;
     p[1]=j;
     p[2]=NOP;
    }
    else 
    {
     p[1]= 0x0000FF & (j>>8);
     p[2]= 0x0000FF &  j;
    }
   }
  }
  else { m=0; longarg=0; }
  p+=n;
 }
}

static TProtoFunc* TF;

static int compare(const void* a, const void* b)
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

static void OptConstants(TProtoFunc* tf)
{
 static int* C=NULL;
 static int* D=NULL;
 int i,k;
 int n=tf->nconsts;
 if (n==0) return;
 C=luaM_reallocvector(C,n,int);
 D=luaM_reallocvector(D,n,int);
 for (i=0; i<n; i++) C[i]=D[i]=i;	/* group duplicates */
 TF=tf; qsort(C,n,sizeof(C[0]),compare);
 k=C[0];				/* build duplicate table */
 for (i=1; i<n; i++)
 {
  int j=C[i];
  TObject* oa=tf->consts+k;
  TObject* ob=tf->consts+j;
  if (ttype(oa)==ttype(ob) && oa->value.i==ob->value.i) D[j]=k; else k=j;
 }
 k=0;					/* build rename map & pack constants */
 for (i=0; i<n; i++)
 {
  if (D[i]==i) { tf->consts[k]=tf->consts[i]; C[i]=k++; } else C[i]=C[D[i]];
 }
 if (k>=n) return;
printf("\t\"%s\":%d reduced constants from %d to %d\n",
	tf->source->str,tf->lineDefined,n,k);
 tf->nconsts=k;
 FixConstants(tf,C);
}

static int NoDebug(TProtoFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 int m=0;				/* size of last LONGARG */
 int nop=0;
 while (1)				/* change SETLINE to NOP */
 {
  Opcode OP;
  int n=INFO(tf,p,&OP);
  int op=OP.class;
  if (op==ENDCODE) break;
  else if (op==NOP) ++nop;
  else if (op==LONGARG) m=n;		/* in case next is SETLINE */
  else if (op==SETLINE)
  {
   nop+=n+m; memset(p-m,NOP,n+m);
  }
  else m=0;
  p+=n;
 }
 return nop;
}

static int FixJump(TProtoFunc* tf, Byte* a, Byte* b)
{
 Byte* p;
 int nop=0;
 for (p=a; p<b; )
 {
  Opcode OP;
  int n=INFO(tf,p,&OP);
  int op=OP.class;
  if (op==ENDCODE) break;
  if (op==NOP) ++nop;
  p+=n;
 }
 return nop;
}

static void FixJumps(TProtoFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 while (1)
 {
  Opcode OP;
  int n=INFO(tf,p,&OP);
  int op=OP.class;
  int i=OP.arg;
  int nop;
  if (op==ENDCODE) break;
  nop=0;
  if (op==IFTUPJMP || op==IFFUPJMP) nop=FixJump(tf,p-i+n,p); else
  if (op==ONTJMP || op==ONFJMP || op==JMP || op==IFFJMP) nop=FixJump(tf,p,p+i+n);
  if (nop>0)
  {
   int j=i-nop;
   if (n==2)
    p[1]=j;
   else
#if 0
   {
    if (j<=MAX_BYTE)			/* does NOT work for nested loops */
    {
     if (op==IFTUPJMP || op==IFFUPJMP) --j;
     p[0]=OP.op-1;			/* *JMP and *JMPW are consecutive */
     p[1]=j;
     p[2]=NOP;
    }
    else 
#endif
    {
     p[1]= 0x0000FF & (j>>8);
     p[2]= 0x0000FF &  j;
    }
#if 0
   }
#endif
  }
  p+=n;
 }
}

static void PackCode(TProtoFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 Byte* q=code;
 while (1)
 {
  Opcode OP;
  int n=INFO(tf,p,&OP);
  int op=OP.class;
  if (op!=NOP) { memcpy(q,p,n); q+=n; }
  p+=n;
  if (op==ENDCODE) break;
 }
printf("\t\"%s\":%d reduced code from %d to %d\n",
	tf->source->str,tf->lineDefined,(int)(p-code),(int)(q-code));
}

static void OptCode(TProtoFunc* tf)
{
 int nop=NoDebug(tf);
 if (nop==0) return;			/* cannot improve code */
 FixJumps(tf);
 PackCode(tf);
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
 tf->locvars=NULL;			/* remove local variables */
 OptConstants(tf);
 OptCode(tf);
 OptFunctions(tf);
}

void luaU_optchunk(TProtoFunc* Main)
{
 OptFunction(Main);
}
