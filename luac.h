/*
** $Id: luac.h,v 1.19 2001/03/15 17:29:16 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#define LUA_PRIVATE
#include "lua.h"

#include "lobject.h"

extern lua_State *luac_state;		/* lazy! */

/* from dump.c */
void luaU_dumpchunk(const Proto* Main, FILE* D);

/* from opt.c */
void luaU_optchunk(Proto* Main);

/* from print.c */
void luaU_printchunk(const Proto* Main);

#define Sizeof(x)	((int)sizeof(x))
