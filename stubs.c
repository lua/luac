/*
** $Id: stubs.c,v 1.21 2001/03/15 17:29:16 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>

#include "luac.h"
#include "ldebug.h"
#include "ldo.h"
#include "llex.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"

#ifndef NOSTUBS

const char luac_ident[] = "$luac: " LUA_VERSION " " LUA_COPYRIGHT " $\n"
                          "$Authors: " LUA_AUTHORS " $";

/*
* avoid lapi ldebug ldo lgc lstate ltm lvm
* use only lcode lfunc llex lmem lobject lparser lstring ltable lzio
*/

/* simplified from lapi.c */
void lua_error (lua_State* L, const char* s) {
 luaD_error(L,s);
}

/* simplified from ldo.c */
void luaD_error (lua_State* L, const char* s) {
  UNUSED(L);
  if (s) fprintf(stderr,"luac: %s\n",s);
  exit(1);
}

/* simplified from ldo.c */
void luaD_breakrun (lua_State *L, int errcode) {
  UNUSED(errcode);
  lua_error(L,"memory allocation error");
}

/* simplified from lstate.c */
lua_State *lua_newthread (lua_State *OL, int stacksize) {
  lua_State *L = luaM_new(OL, lua_State);
  if (L == NULL) lua_error(L,"not enough memory");
  G(L) = luaM_new(L, global_State);
  G(L)->strt.size = G(L)->udt.size = 0;
  G(L)->strt.nuse = G(L)->udt.nuse = 0;
  G(L)->strt.hash = G(L)->udt.hash = NULL;
  G(L)->Mbuffer = NULL;
  G(L)->Mbuffsize = 0;
  G(L)->rootproto = NULL;
  G(L)->rootcl = NULL;
  G(L)->roottable = NULL;
  G(L)->TMtable = NULL;
  G(L)->sizeTM = 0;
  G(L)->ntag = 0;
  G(L)->nblocks = sizeof(lua_State) + sizeof(global_State);
  luaS_init(L);
  luaX_init(L);
  G(L)->GCthreshold = 4*G(L)->nblocks;
  UNUSED(stacksize);
  return L;
}

/* copied from ldebug.c */
int luaG_getline (int *lineinfo, int pc, int refline, int *prefi) {
  int refi;
  if (lineinfo == NULL || pc == -1)
    return -1;  /* no line info or function is not active */
  refi = prefi ? *prefi : 0;
  if (lineinfo[refi] < 0)
    refline += -lineinfo[refi++];
  lua_assert(lineinfo[refi] >= 0);
  while (lineinfo[refi] > pc) {
    refline--;
    refi--;
    if (lineinfo[refi] < 0)
      refline -= -lineinfo[refi--];
    lua_assert(lineinfo[refi] >= 0);
  }
  for (;;) {
    int nextline = refline + 1;
    int nextref = refi + 1;
    if (lineinfo[nextref] < 0)
      nextline += -lineinfo[nextref++];
    lua_assert(lineinfo[nextref] >= 0);
    if (lineinfo[nextref] > pc)
      break;
    refline = nextline;
    refi = nextref;
  }
  if (prefi) *prefi = refi;
  return refline;
}

/*
* the code below avoids the lexer and the parser (llex lparser).
* it is useful if you only want to load binary files.
* this works for interpreters like lua.c too.
*/

#ifdef NOPARSER

#include "llex.h"
#include "lparser.h"

void luaX_init(lua_State *L) {
  UNUSED(L);
}

Proto *luaY_parser(lua_State *L, ZIO *z) {
  UNUSED(z);
  lua_error(L,"parser not loaded");
  return NULL;
}

#endif
#endif
