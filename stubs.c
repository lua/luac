/*
** $Id: stubs.c,v 1.9 1999/02/03 00:38:28 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

/*
* avoid lapi lauxlib lbuiltin ldo lgc ltable ltm lvm
* use only lbuffer lfunc llex lmem lobject lparser lstate lstring lzio
*/

/* simplified from ldo.c */
void lua_error(char* s)
{
 if (s) fprintf(stderr,"luac: %s\n",s);
 exit(1);
}

/* copied from lauxlib.c */
void luaL_verror(char* fmt, ...)
{
 char buff[500];
 va_list argp;
 va_start(argp,fmt);
 vsprintf(buff,fmt,argp);
 va_end(argp);
 lua_error(buff);
}

/* copied from lauxlib.c */
int luaL_findstring (char* name, char* list[])
{
 int i;
 for (i=0; list[i]; i++)
   if (strcmp(list[i], name) == 0)
     return i;
 return -1;
}

/* copied from lauxlib.c */
void luaL_chunkid (char *out, char *source, int len) {
  len -= 13;  /* 13 = strlen("string ''...\0") */
  if (*source == '@')
    sprintf(out, "file `%.*s'", len, source+1);
  else if (*source == '(')
    strcpy(out, "(C code)");
  else {
    char *b = strchr(source , '\n');  /* stop string at first new line */
    int lim = (b && (b-source)<len) ? b-source : len;
    sprintf(out, "string `%.*s'", lim, source);
    strcpy(out+lim+(13-5), "...'");  /* 5 = strlen("...'\0") */
  }
}

/* avoid runtime modules in lstate.c */

#include "lbuiltin.h"
#include "ldo.h"
#include "lgc.h"
#include "ltable.h"
#include "ltm.h"

void luaB_predefine(void){}
void luaC_hashcallIM(Hash *l){}
void luaC_strcallIM(TaggedString *l){}
void luaD_gcIM(TObject *o){}
void luaH_free(Hash *frees){}
void luaT_init(void){}

/* copied from ldo.c */

#include "lmem.h"

#ifndef STACK_LIMIT
#define STACK_LIMIT     6000
#endif

#define STACK_UNIT	128

void luaD_init (void) {
  L->stack.stack = luaM_newvector(STACK_UNIT, TObject);
  L->stack.top = L->stack.stack;
  L->stack.last = L->stack.stack+(STACK_UNIT-1);
}


void luaD_checkstack (int n)
{
  struct Stack *S = &L->stack;
  if (S->last-S->top <= n) {
    StkId top = S->top-S->stack;
    int stacksize = (S->last-S->stack)+1+STACK_UNIT+n;
    S->stack = luaM_reallocvector(S->stack, stacksize, TObject);
    S->last = S->stack+(stacksize-1);
    S->top = S->stack + top;
    if (stacksize >= STACK_LIMIT) {  /* stack overflow? */
        lua_error("stack size overflow");
    }
  }
}

/* copied from lapi.c */
int lua_setdebug (int debug) {
  int old = L->debug;
  L->debug = debug;
  return old;
}

/*
* the code below avoids the lexer and the parser (llex lparser).
* it is useful if you only want to load binary files.
* this works for interpreters like lua.c too.
*/

#ifdef NOPARSER

#include "llex.h"
#include "lparser.h"

void luaX_init(void){}

TProtoFunc* luaY_parser(ZIO *z) {
 lua_error("parser not loaded");
 return NULL;
}

#endif

