/*
** $Id: opt.c,v 1.17 2000/04/24 17:32:29 lhf Exp lhf $
** optimize bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ltable.h"
#include "luac.h"

#define OP_NOP	-1UL

static int MapConstant(Hash* t, int j, const TObject* key)
{
 const TObject* o=luaH_get(L,t,key);
 if (o!=&luaO_nilobject) 
  return nvalue(o);
 else
 {
  TObject val;
  ttype(&val)=TAG_NUMBER;
  nvalue(&val)=j;
  luaH_set(L,t,key,&val);
  return j;
 }
}

static void OptConstants(Proto* tf)
{
 Instruction* p;
 int nknum,nkstr;
 Hash* map=luaH_new(L,tf->nknum+tf->nkstr);
 nknum=nkstr=-1;
 for (p=tf->code;; p++)
 {
  Instruction i=*p;
  int op=GET_OPCODE(i);
  switch (op)
  {
   TObject o;
   int j,k;
   case OP_PUSHNUM: case OP_PUSHNEGNUM:
    j=GETARG_U(i);
    ttype(&o)=TAG_NUMBER; nvalue(&o)=tf->knum[j];
    k=MapConstant(map,nknum+1,&o);
    if (k!=j) *p=CREATE_U(op,k);
    if (k>nknum) nknum=k;
    break;
   case OP_PUSHSTRING: case OP_GETGLOBAL: case OP_GETDOTTED:
   case OP_PUSHSELF:   case OP_SETGLOBAL:
    j=GETARG_U(i);
    ttype(&o)=TAG_STRING; tsvalue(&o)=tf->kstr[j];
    k=MapConstant(map,nkstr+1,&o);
    if (k!=j) *p=CREATE_U(op,k);
    if (k>nkstr) nkstr=k;
    break;
   case OP_END:
    luaH_free(L,map);
    tf->nknum=nknum+1;
    tf->nkstr=nkstr+1;
    return;
   default:
    break;
  }
 }
}

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
 OptConstants(tf);
 OptFunctions(tf);
 tf->source=luaS_new(L,"");
 tf->locvars=NULL;			/* lazy! */
}

void luaU_optchunk(Proto* Main)
{
 OptFunction(Main);
}
