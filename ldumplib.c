/*
** $Id: ldumplib.c,v 1.8 2002/08/07 00:36:03 lhf Exp lhf $
** library access to precompiler
** See Copyright Notice in lua.h
*/

#include "lua.h"
#include "lauxlib.h"

#include "lundump.h"

static int writer (const void* b, size_t size, void* B) {
  luaL_addlstring(cast(luaL_Buffer*, B), b, size);
  return 1;
}

static int luaB_dump (lua_State *L) {
  int status;
  luaL_Buffer b;
  luaL_check_type(L, 1, LUA_TFUNCTION);
  luaL_buffinit(L,&b);
  status = lua_dump(L, writer, &b);
  luaL_pushresult(&b);
  return status;
}

/*
{"dump", luaB_dump},
Add this to base_funcs[] in lbaselib.c
*/

#include "lualib.h"

/*
** Open dump library
*/
LUALIB_API int lua_dumplibopen (lua_State *L);

LUALIB_API int lua_dumplibopen (lua_State *L) {
 lua_register(L,"dump",luaB_dump);
 return 0;
}
