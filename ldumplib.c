/*
** $Id: ldumplib.c,v 1.2 2001/06/28 13:55:17 lhf Exp lhf $
** library access to precompiler
** See Copyright Notice in lua.h
*/

#include <stdio.h>

#define LUA_PRIVATE
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

LUALIB_API int lua_dumplibopen (lua_State *L);

static void myfputc(int c, luaL_Buffer* B) {
 char b[1];
 b[0]=c;
 luaL_addlstring(B,b,1);
}

static void myfwrite(const void* b, size_t size, size_t n, luaL_Buffer* B) {
 luaL_addlstring(B,b,size*n);
}

#define WRITETO	luaL_Buffer*
#define WRITE1	myfputc
#define WRITE	myfwrite
#include "dump.c"

static int dump(lua_State *L) {
 if (!lua_isfunction(L,1) || lua_iscfunction(L,1)) {
  luaL_argerror(L,1,"Lua function expected");
  return 0;
 }
 else {
  luaL_Buffer b;
  const Closure* c=lua_topointer(L,1);
  if (c->nupvalues>0) luaL_argerror(L,1,"cannot dump closures");
  luaL_buffinit(L,&b);
  luaU_dumpchunk(c->f.l,&b);
  luaL_pushresult(&b);
  return 1;
 }
}

static const struct luaL_reg dumplib[] = {
{"dump", dump},
};

/*
** Open dump library
*/
LUALIB_API int lua_dumplibopen (lua_State *L) {
 luaL_openl(L,dumplib);
 return 0;
}
