/*
** $Id: opt.c,v 1.16 2000/01/28 17:51:09 lhf Exp lhf $
** optimize bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

#define OP_NOP	-1UL

#if 0
static void FixConstants(Proto* tf, int* C)
{
 Instruction* code=tf->code;
 Instruction* p=code;
 for (;;)
 {
  Opcode OP;
  int n=INFO(tf,p,&OP);
  int op=OP.class;
  int i=OP.arg+longarg;
  if (op==OP_PUSHCONSTANT || op==OP_GETGLOBAL || op==OP_GETDOTTED ||
      op==OP_PUSHSELF     || op==OP_SETGLOBAL || op==OP_CLOSURE)
   FixArg(p,i,C[i],1);
  else if (op==OP_END) break;
  p+=n;
 }
}

static void* V;				/* for sort */

static int compare_numbers(const void* a, const void* b)
{
 const int ia=*(const int*)a;
 const int ib=*(const int*)b;
 Number* v=(Number*) V;
 Number va=v[ia];
 Number vb=v[ib];
 return (va==vb) ? 0 : ia-ib;
}

static void FixNumbers(Instruction* code, int* map)
{
 Instruction* p;
 for (p=code;; p++)
 {
  Instruction i=*p;
  int op=GET_OPCODE(i);
  switch (op)
  {
   case OP_PUSHNUM: case OP_PUSHNEGNUM:
    int j=GETARG_U(i);
    *p=CREATE_U(o,map[j]);
    break;
   case OP_END: return;
  }
 }
}

static void FindNumbers(Instruction* code, int* seen)
{
 Instruction* p;
 for (p=code;; p++)
 {
  Instruction i=*p;
  int op=GET_OPCODE(i);
  switch (op)
  {
   case OP_PUSHNUM: case OP_PUSHNEGNUM:
    seen[GETARG_U(i)]=1;
   case OP_END: return;
  }
 }
}

static int OptNumbers(Proto* tf, int* C, int* D)
{
 int i,k;
 int n=tf->nknum;
 Number* v=tf->knum;
 for (i=0; i<n; i++) C[i]=D[i]=i;	/* group duplicates */
 V=v; qsort(C,n,sizeof(*C),compare_numbers);
 k=C[0];				/* build duplicate table */
 for (i=1; i<n; i++)
 {
  int j=C[i];
  if (v[k]==v[j]) D[j]=k; else k=j;
 }
 k=0;					/* build rename map & pack constants */
 for (i=0; i<n; i++)
 {
  if (D[i]==i)				/* new value */
  {
   v[k]=v[i];
   C[i]=k++;
  }
  else C[i]=C[D[i]];
 }
 return k;
}

static void FindStrings(Instruction* code, int* seen)
{
 Instruction* p;
 for (p=code;; p++)
 {
  Instruction i=*p;
  int op=GET_OPCODE(i);
  switch (op)
  {
   case OP_PUSHSTRING: case OP_GETGLOBAL: case OP_GETDOTTED:
   case OP_PUSHSELF:   case OP_SETGLOBAL:
    seen[GETARG_U(i)]=1;
   case OP_END: return;
  }
 }
}

static int compare_strings(const void* a, const void* b)
{
 const int ia=*(const int*)a;
 const int ib=*(const int*)b;
 TString** v=(TString**) V;
 TString* va=v[ia];
 TString* vb=v[ib];
 return (strcmp(va->str,vb->str)==0) ? 0 : ia-ib;
}

static void OptConstants(Proto* tf)
{
 static int* N=NULL;
 static int* S=NULL;
 static int* D=NULL;
 static int* U=NULL;
 int n,nnum,nstr;
 n=tf->nknum;
 if (n>0) 
 {
  luaM_reallocvector(L,N,n,int);
  luaM_reallocvector(L,D,n,int);
  nnum=OptNumbers(tf,N,D);
 }
 n=tf->nkstr;
 if (n>0) 
 {
  luaM_reallocvector(L,S,n,int);
  luaM_reallocvector(L,D,n,int);
  luaM_reallocvector(L,U,n,int);
  nstr=OptStrings(tf,S,D,U);
 }
 if (nnum!=tf->nknum || nstr!=tf->nkstr) FixConstants(tf,C);
}

#endif

static int FixJump(Instruction* a, Instruction* b)
{
 Instruction* p;
 int nop=0;
 for (p=a; p<b; p++)
 {
  Instruction op=*p;
  if (op==OP_NOP) ++nop;
  else if (op==OP_END) break;
 }
 return nop;
}

static void FixJumps(Instruction* code)
{
 Instruction* p=code;
 for (;;)
 {
  Instruction i=*p;
  int op=GET_OPCODE(i);
  int j=GETARG_S(i);
  if (ISJUMP(op))
  {
   int n;
   if (j>0) n=FixJump(p,p+j+1); else n=FixJump(p+j+1,p);
   if (n>0) 
   {
    if (j>0) j-=n; else j+=n;
    *p=CREATE_S(op,j);
   }
  }
  else if (op==OP_END) break;
  ++p;
 }
}

static int FixDebug(Instruction* code)
{
 Instruction* p;
 int nop=0;
 for (p=code;; p++)
 {
  Instruction op=*p;
  if (GET_OPCODE(op)==OP_SETLINE) { *p=OP_NOP; ++nop; }
  else if (op==OP_NOP) ++nop;
  else if (op==OP_END) break;
 }
 return nop;
}

static void PackCode(Instruction* code)
{
 Instruction* p=code;
 Instruction* q=code;
 for (;;)
 {
  Instruction op=*p++;
  if (op!=OP_NOP) *q++=op;
  if (op==OP_END) break;
 }
}

static void OptCode(Proto* tf)
{
 Instruction* code=tf->code;
 if (FixDebug(code)>0)
 {
  FixJumps(code);
  PackCode(code);
 }
}

static void OptFunction(Proto* tf);

static void OptFunctions(Proto* tf)
{
 int i,n=tf->nkproto;
 for (i=0; i<n; i++) OptFunction(tf->kproto[i]);
}

static void OptFunction(Proto* tf)
{
 OptCode(tf);
#if 0
 OptConstants(tf);
#endif
 OptFunctions(tf);
 tf->source=luaS_new(L,"");
 tf->locvars=NULL;			/* lazy! */
}

void luaU_optchunk(Proto* Main)
{
 OptFunction(Main);
}
