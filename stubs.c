/*
** $Id: stubs.c,v 1.1 1997/12/02 23:18:50 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

/* avoid lapi.o lauxlib.o lbuiltin.o ldo.o lgc.o ltable.o ltm.o lvm.o */
/* use lbuffer.o lfunc.o llex.o lmem.o lobject.o lstring.o lstx.o lzio.o */

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

#if 0
void luaB_predefine(void){}
void luaC_hashcallIM(Hash *l) {}
void luaC_strcallIM(TaggedString *l) {}
void luaD_gcIM(TObject *o) {}
void luaD_init(void) {}
void luaH_free(Hash *frees) {}
void luaT_init(void) {}
#else
#include "lstate.h"
#include "lmem.h"
#include "llex.h"

LState *lua_state = NULL;

void lua_open (void)
{
  if (lua_state) return;
  lua_state = luaM_new(LState);
  L->numCblocks = 0;
  L->Cstack.base = 0;
  L->Cstack.lua2C = 0;
  L->Cstack.num = 0;
  L->errorJmp = NULL;
  L->rootproto.next = NULL;
  L->rootproto.marked = 0;
  L->rootcl.next = NULL;
  L->rootcl.marked = 0;
  L->rootglobal.next = NULL;
  L->rootglobal.marked = 0;
  L->roottable.next = NULL;
  L->roottable.marked = 0;
  L->refArray = NULL;
  L->refSize = 0;
  L->Mbuffsize = 0;
  L->Mbuffer = NULL;
  L->GCthreshold = GARBAGE_BLOCK;
  L->nblocks = 0;
  luaS_init();
  luaX_init();
}
#endif

#ifdef NOPARSER
/* avoid llex.o lstx.o */

int lua_debug=0;

void luaX_init(void){}
void luaY_init(void){}

TProtoFunc* luaY_parser(ZIO* z, char* chunkname)
{
 lua_error("no parser loaded");
}
#endif
