/*
** $Id: opt.c,v 1.24 2001/06/28 13:55:17 lhf Exp lhf $
** optimize bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luac.h"
#include "lobject.h"
#include "lopcodes.h"
#include "ltable.h"

#define LUA_DEBUG
#undef LUA_DEBUG

static lua_State* L;			/* lazy! */

static int MapConstant(Hash* t, int j, const TObject* key)
{
 const TObject* o=luaH_get(t,key);
 if (ttype(o)==LUA_TNUMBER)
  return (int) nvalue(o);
 else
 {
  TObject val;
  setnvalue(&val,j);
  *luaH_set(L,t,key)=val;
  lua_assert(j>=0);
  return j;
 }
}

static int MapConstants(Proto* f, Hash* map)
{
 int i,j=0,n=f->sizek;
 for (i=0; i<n; i++)
 {
  int k=MapConstant(map,j,&f->k[i]);
  if (k==j) j++;
 }
 return j;
}

static void PackConstants(Proto* f, Hash* map)
{
 int i,j=0,n=f->sizek;
#ifdef LUA_DEBUG
 printf("%p before pack sizek=%d\n",f,f->sizek);
#endif
 for (i=0; i<n; i++)
 {
  int k=MapConstant(map,-1,&f->k[i]);
  if (k==j) f->k[j++]=f->k[i];
 }
 f->sizek=j;				/* TODO: messes up free? */
for (; j<n; j++) ttype(&f->k[j])=LUA_TNIL;
#ifdef LUA_DEBUG
 printf("%p after pack sizek=%d\n",f,f->sizek);
#endif
}

static void OptConstants(Proto* f)
{
 Instruction* code=f->code;
 int pc,ni=f->sizecode;
 int n=f->sizek;
 Hash* map=luaH_new(L,n);
 int m=MapConstants(f,map);
#ifdef LUA_DEBUG
 printf("%p n=%d m=%d %s\n",f,n,m,(m==n)?"nothing to optimize":"yes!");
#endif
 if (m==n) return;
 for (pc=0; pc<ni; pc++)
 {
  Instruction i=code[pc];
  int op=GET_OPCODE(i);
  switch (op)
  {
   int j,k;
   case OP_LOADK:
   case OP_GETGLOBAL:
   case OP_SETGLOBAL:
    j=GETARG_Bc(i);
    k=MapConstant(map,-1,&f->k[j]);
    if (k!=j) code[pc]=CREATE_ABc(op,GETARG_A(i),k);
    break;
   case OP_GETTABLE:
   case OP_SETTABLE:
   case OP_SELF:
   case OP_ADD:
   case OP_SUB:
   case OP_MUL:
   case OP_DIV:
   case OP_POW:
   case OP_TESTEQ:
   case OP_TESTNE:
   case OP_TESTLT:
   case OP_TESTLE:
   case OP_TESTGT:
   case OP_TESTGE:
    j=GETARG_C(i);
    if (j<MAXSTACK) break; else j-=MAXSTACK;
#ifdef LUA_DEBUG
printf("a=%d b=%d c=%d\n",GETARG_A(i),GETARG_B(i),GETARG_C(i));
#endif
    k=MapConstant(map,-1,&f->k[j]);
    if (k!=j) code[pc]=CREATE_ABC(op,GETARG_A(i),GETARG_B(i),k+MAXSTACK);
#ifdef LUA_DEBUG
i=code[pc];
printf("a=%d b=%d c=%d\n",GETARG_A(i),GETARG_B(i),GETARG_C(i));
#endif
    break;
   default:
    break;
  }
 }
 PackConstants(f,map);
 luaH_free(L,map);
}

#define OptFunction luaU_optchunk

void OptFunction(lua_State* l, Proto* f)
{
 int i,n=f->sizep;
 L=l;
 OptConstants(f);
 for (i=0; i<n; i++) OptFunction(l,f->p[i]);
}
