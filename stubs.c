/*
** $Id: stubs.c,v 1.13 1999/10/07 12:13:13 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#ifdef NOSTUBS

/* according to gcc, ANSI C forbids an empty source file */
void luaU_dummy(void);
void luaU_dummy(void){}

#else

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#undef L
#define	UNUSED(x)	(void)x

/*
* avoid lapi lauxlib lbuiltin ldo lgc ltable ltm lvm
* use only lbuffer lfunc llex lmem lobject lparser lstate lstring lzio
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

/* avoid runtime modules in lstate.c */

#include "lbuiltin.h"
#include "ldo.h"
#include "lgc.h"
#include "ltable.h"
#include "ltm.h"

void luaB_predefine(lua_State *L){ UNUSED(L); }
void luaC_collect(lua_State *L, int all){ UNUSED(L); UNUSED(all); }
void luaD_gcIM(lua_State *L, const TObject *o){ UNUSED(L); UNUSED(o); }
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
void luaD_init(lua_State *L){ UNUSED(L); }

TProtoFunc* luaY_parser(lua_State *L, ZIO *z) {
 UNUSED(z);
 lua_error(L,"parser not loaded");
 return NULL;
}

#else

/* copied from lauxlib.c */
int luaL_findstring (const char *name, const char *const list[]) {
  int i;
  for (i=0; list[i]; i++)
    if (strcmp(list[i], name) == 0)
      return i;
  return -1;  /* name not found */
}

/* copied from lauxlib.c */
void luaL_chunkid (char *out, const char *source, int len) {
  len -= 13;  /* 13 = strlen("string ''...\0") */
  if (*source == '@')
    sprintf(out, "file `%.*s'", len, source+1);
  else if (*source == '(')
    strcpy(out, "(C code)");
  else {
    const char *b = strchr(source , '\n');  /* stop string at first new line */
    int lim = (b && (b-source)<len) ? b-source : len;
    sprintf(out, "string `%.*s'", lim, source);
    strcpy(out+lim+(13-5), "...'");  /* 5 = strlen("...'\0") */
  }
}

void luaD_checkstack(lua_State *L, int n){ UNUSED(L); UNUSED(n); }

#define DEFAULT_STACK_SIZE      1024
#define EXTRA_STACK     32

/* copied from ldo.c */
void luaD_init (lua_State *L) {
  L->stack = luaM_newvector(L, DEFAULT_STACK_SIZE+EXTRA_STACK, TObject);
  L->stack_last = L->stack+(DEFAULT_STACK_SIZE-1);
  L->Cstack.base = L->Cstack.lua2C = L->top = L->stack;
  L->Cstack.num = 0;
}

#endif
#endif
