/*
** $Id: luac.h,v 1.15 2000/06/28 14:12:55 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "ltable.h"
#include "lundump.h"

extern lua_State *lua_state;
#define	L	lua_state		/* lazy! */

/* from dump.c */
void luaU_dumpchunk(const Proto* Main, FILE* D);
int luaU_codesize(const Proto* tf);

/* from opt.c */
void luaU_optchunk(Proto* Main);

/* from print.c */
void luaU_printchunk(const Proto* Main);

/* from test.c */
void luaU_testchunk(const Proto* Main);
