/*
** $Id: stubs.c,v 1.15 2000/01/28 17:51:09 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#ifdef NOSTUBS

/* according to gcc, ANSI C forbids an empty source file */
void luaU_dummy(void);

#else

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#undef L

const char luac_ident[] = "$luac: " LUA_VERSION " " LUA_COPYRIGHT " $\n"
                          "$Authors: " LUA_AUTHORS " $";

/*
* avoid lapi lauxlib lbuiltin ldebug ldo lgc lref ltable ltm lvm
* use only lbuffer lcode lfunc llex lmem lobject lparser lstate lstring lzio
*/

/* simplified from ldo.c */
void lua_error(lua_State* L, const char* s)
{
 UNUSED(L);
 if (s) fprintf(stderr,"luac: %s\n",s);
 exit(1);
}

/* copied from lauxlib.c */
void luaL_verror (lua_State *L, const char *fmt, ...) {
  char buff[500];
  va_list argp;
  va_start(argp, fmt);
  vsprintf(buff, fmt, argp);
  va_end(argp);
  lua_error(L, buff);
}

/* copied from lauxlib.c */
void luaL_filesource (char *out, const char *filename, int len) {
  if (filename == NULL) filename = "(stdin)";
  sprintf(out, "@%.*s", len-2, filename);  /* -2 for '@' and '\0' */
}

/* copied from lauxlib.c */
int luaL_findstring (const char *name, const char *const list[]) {
  int i;
  for (i=0; list[i]; i++)
    if (strcmp(list[i], name) == 0)
      return i;
  return -1;  /* name not found */
}

/* avoid runtime modules in lstate.c */

#include "lbuiltin.h"
#include "ldo.h"
#include "lgc.h"
#include "ltable.h"
#include "ltm.h"

void luaB_predefine(lua_State *L){ UNUSED(L); }
void luaC_collect(lua_State *L, int all){ UNUSED(L); UNUSED(all); }
void luaH_free(lua_State *L, Hash *frees){ UNUSED(L); UNUSED(frees); }
void luaT_init(lua_State *L){ UNUSED(L);}

/*
* the code below avoids the lexer and the parser (llex lparser).
* it is useful if you only want to load binary files.
* this works for interpreters like lua.c too.
*/

#ifdef NOPARSER

#include "llex.h"
#include "lparser.h"

void luaX_init(lua_State *L){ UNUSED(L); }
void luaD_init(lua_State *L, int stacksize) { UNUSED(L); UNUSED(stacksize); }

Proto *luaY_parser(lua_State *L, ZIO *z) {
 UNUSED(z);
 lua_error(L,"parser not loaded");
 return NULL;
}

#else

#define EXTRALEN	sizeof("string \"...\"0")

/* copied from lauxlib.c */
void luaL_chunkid (char *out, const char *source, int len) {
  if (*source == '(') {
    strncpy(out, source+1, len-1);  /* remove first char */
    out[len-1] = '\0';  /* make sure `out' has an end */
    out[strlen(out)-1] = '\0';  /* remove last char */
  }
  else {
    len -= EXTRALEN;
    if (*source == '@')
      sprintf(out, "file `%.*s'", len, source+1);
    else {
      const char *b = strchr(source , '\n');  /* stop at first new line */
      int lim = (b && (b-source)<len) ? b-source : len;
      sprintf(out, "string \"%.*s\"", lim, source);
      strcpy(out+lim+(EXTRALEN-sizeof("...\"0")), "...\"");
    }
  }
}

void luaD_checkstack(lua_State *L, int n){ UNUSED(L); UNUSED(n); }

#define EXTRA_STACK     32

/* copied from ldo.c */
void luaD_init (lua_State *L, int stacksize) {
  L->stack = luaM_newvector(L, stacksize+EXTRA_STACK, TObject);
  L->stack_last = L->stack+(stacksize-1);
  L->stacksize = stacksize;
  L->Cstack.base = L->Cstack.lua2C = L->top = L->stack;
  L->Cstack.num = 0;
}

#endif
#endif
