/*
** $Id: luac.h,v 1.21 2001/07/19 14:34:06 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#define LUA_PRIVATE
#include "lua.h"

#include "lobject.h"

/* from dump.c */
void luaU_dumpchunk(const Proto* Main, FILE* D);

/* from print.c */
void luaU_printchunk(const Proto* Main);
