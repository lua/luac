/*
** $Id$
** library access to precompiler
** See Copyright Notice in lua.h
*/

#include <stdio.h>

#include "lauxlib.h"
#include "lparser.h"
#include "lundump.h"

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

static int prestring(lua_State *L) {
 size_t l;
 const char* s=luaL_check_lstr(L,1,&l); 
 ZIO z;
 luaL_Buffer b;
 luaL_buffinit(L,&b);
 luaZ_mopen(&z,s,l,s);
 luaU_dumpchunk(luaY_parser(L,&z),&b);
 luaL_pushresult(&b);
 return 1;
}

static int prefile(lua_State *L) {
 const char* filename=luaL_check_string(L,1); 
 ZIO z;
 char source[512];
 FILE* f=fopen(filename,"r");
 luaL_Buffer b;
 if (f==NULL) {
  lua_pushnil(L);
  return 1;
 }
 sprintf(source,"@%.*s",(int)sizeof(source)-2,filename);
 luaL_buffinit(L,&b);
 luaZ_Fopen(&z,f,source);
 luaU_dumpchunk(luaY_parser(L,&z),&b);
 if (f!=stdin) fclose(f);
 luaL_pushresult(&b);
 return 1;
}

static const struct luaL_reg dumplib[] = {
{"prestring", prestring},
{"prefile", prefile},
};

/*
** Open dump library
*/
int lua_dumplibopen (lua_State *L) {
 luaL_openl(L,dumplib);
 return 0;
}
