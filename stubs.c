/*
** $Id: stubs.c,v 1.18 2000/09/18 20:03:46 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "luac.h"
#undef L

#ifndef NOSTUBS

const char luac_ident[] = "$luac: " LUA_VERSION " " LUA_COPYRIGHT " $\n"
                          "$Authors: " LUA_AUTHORS " $";

/*
* avoid lapi ldebug ldo lgc ltm lvm
* use only lcode lfunc llex lmem lobject lparser lstate lstring ltable lzio
*/

/* simplified from ldo.c */
void lua_error(lua_State* L, const char* s)
{
 UNUSED(L);
 if (s) fprintf(stderr,"luac: %s\n",s);
 exit(1);
}

/* avoid runtime modules in lstate.c */

#include "ldo.h"
#include "lgc.h"
#include "ltm.h"

void luaC_collect(lua_State *L, int all){ UNUSED(L); UNUSED(all); }
void luaD_breakrun (lua_State *L, int errcode)  { UNUSED(L); UNUSED(errcode); }
void luaD_init(lua_State *L, int stacksize) { UNUSED(L); UNUSED(stacksize); }
void luaT_init(lua_State *L){ UNUSED(L);}

const char *lua_tostring(lua_State *L, int index)
{ UNUSED(L); UNUSED(index); return NULL; }
void lua_pushcclosure(lua_State *L, lua_CFunction fn, int n)
{ UNUSED(L); UNUSED(fn); UNUSED(n); }
void lua_setglobal(lua_State *L, const char *name)
{ UNUSED(L); UNUSED(name); }

/*
* the code below avoids the lexer and the parser (llex lparser lcode).
* it is useful if you only want to load binary files.
* this works for interpreters like lua.c too.
*/

#ifdef NOPARSER

#include "llex.h"
#include "lparser.h"

void luaX_init(lua_State *L){ UNUSED(L); }

Proto *luaY_parser(lua_State *L, ZIO *z) {
 UNUSED(z);
 lua_error(L,"parser not loaded");
 return NULL;
}

#endif
#endif
