/*
** $Id: luac.h,v 1.20 2001/06/28 13:55:17 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#define LUA_PRIVATE
#include "lua.h"

#include "lobject.h"

/* from dump.c */
void luaU_dumpchunk(const Proto* Main, FILE* D);

/* from opt.c */
void luaU_optchunk(lua_State* L, Proto* f);

/* from print.c */
void luaU_printchunk(const Proto* Main);
