/*
** $Id: luac.h,v 1.13 1999/12/02 18:51:09 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#define LUA_REENTRANT

#include "lauxlib.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

extern lua_State *lua_state;
#define	L	lua_state		/* lazy! */

/* from dump.c */
void luaU_dumpchunk(const Proto* Main, FILE* D, int native);

/* from opcode.c */
int luaU_codesize(const Proto* tf);

/* from opt.c */
void luaU_optchunk(Proto* Main);

/* from print.c */
void luaU_printchunk(const Proto* Main);

/* from test.c */
void luaU_testchunk(const Proto* Main);
