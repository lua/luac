/*
** $Id: ldumplib.c,v 1.6 2002/02/28 20:09:28 lhf Exp lhf $
** library access to precompiler
** See Copyright Notice in lua.h
*/

#include <stddef.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lobject.h"
#include "lundump.h"

LUALIB_API int lua_dumplibopen (lua_State *L);

static size_t writer(const void* b, size_t size, size_t n, void* B) {
 luaL_addlstring((luaL_Buffer*)B,b,size*n);
 return n;
}

static int dump(lua_State *L) {
 luaL_Buffer b;
 const Closure* c=lua_topointer(L,1);
 if (c->l.nupvalues>0) luaL_argerror(L,1,"cannot dump closures");
 luaL_buffinit(L,&b);
 luaU_dump(c->l.p,writer,&b);
 luaL_pushresult(&b);
 return 1;
}

/*
** Open dump library
*/
LUALIB_API int lua_dumplibopen (lua_State *L) {
 lua_register(L,"dump",dump);
 return 0;
}
