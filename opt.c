/*
** $Id: opt.c,v 1.22 2000/10/31 16:57:23 lhf Exp lhf $
** optimize bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luac.h"

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

static int MapConstants(Proto* tf, Hash* map)
{
 int i,j,k,n,m=0;
 TObject o;
 j=0; n=tf->sizeknum;
 for (i=0; i<n; i++)
 {
  setnvalue(&o,tf->knum[i]);
  k=MapConstant(map,j,&o);
  if (k==j) j++;
 }
 m=j;
 j=0; n=tf->sizekstr;
 for (i=0; i<n; i++)
 {
  setsvalue(&o,tf->kstr[i]);
  k=MapConstant(map,j,&o);
  if (k==j) j++;
 }
 return m+j;
}

static void PackConstants(Proto* tf, Hash* map)
{
 int i,j,k,n;
 TObject o;
#ifdef LUA_DEBUG
 printf("%p before pack sizeknum=%d sizekstr=%d\n",tf,tf->sizeknum,tf->sizekstr);
#endif
 j=0; n=tf->sizeknum;
 for (i=0; i<n; i++)
 {
  setnvalue(&o,tf->knum[i]);
  k=MapConstant(map,-1,&o);
  if (k==j) tf->knum[j++]=tf->knum[i];
 }
 tf->sizeknum=j;
 j=0; n=tf->sizekstr;
 for (i=0; i<n; i++)
 {
  setsvalue(&o,tf->kstr[i]);
  k=MapConstant(map,-1,&o);
  if (k==j) tf->kstr[j++]=tf->kstr[i];
 }
 tf->sizekstr=j;
#ifdef LUA_DEBUG
 printf("%p after  pack sizeknum=%d sizekstr=%d\n",tf,tf->sizeknum,tf->sizekstr);
#endif
}

static void OptConstants(Proto* tf)
{
 Instruction* code=tf->code;
 int pc,ni=tf->sizecode;
 int n=tf->sizeknum+tf->sizekstr;
 Hash* map=luaH_new(L,n);
 int m=MapConstants(tf,map);
#ifdef LUA_DEBUG
 printf("%p n=%d m=%d %s\n",tf,n,m,(m==n)?"nothing to optimize":"yes!");
#endif
 if (m==n) return;
 for (pc=0; pc<ni; pc++)
 {
  Instruction i=code[pc];
  int op=GET_OPCODE(i);
  switch (op)
  {
   TObject o;
   int j,k;
   case OP_PUSHNUM: case OP_PUSHNEGNUM:
    j=GETARG_U(i);
    setnvalue(&o,tf->knum[j]);
    k=MapConstant(map,-1,&o);
    if (k!=j) code[pc]=CREATE_U(op,k);
    break;
   case OP_PUSHSTRING: case OP_GETGLOBAL: case OP_GETDOTTED:
   case OP_PUSHSELF:   case OP_SETGLOBAL:
    j=GETARG_U(i);
    setsvalue(&o,tf->kstr[j]);
    k=MapConstant(map,-1,&o);
    if (k!=j) code[pc]=CREATE_U(op,k);
    break;
   default:
    break;
  }
 }
 PackConstants(tf,map);
 luaH_free(L,map);
}

#define OptFunction luaU_optchunk

void OptFunction(Proto* tf)
{
 int i,n=tf->sizekproto;
 OptConstants(tf);
 for (i=0; i<n; i++) OptFunction(tf->kproto[i]);
}
