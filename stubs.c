/*
** $Id: stubs.c,v 1.5 1998/03/05 15:45:08 lhf Exp lhf $
** avoid runtime modules in luac
** See Copyright Notice in lua.h
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

/*
* avoid lapi.o lauxlib.o lbuiltin.o ldo.o lgc.o ltable.o ltm.o lvm.o
* use lbuffer.o lfunc.o llex.o lmem.o lobject.o lparser.o lstate.o lstring.o lzio.o
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

/* avoid runtime modules in lstate.c */
void luaB_predefine(void){}
void luaC_hashcallIM(Hash *l){}
void luaC_strcallIM(TaggedString *l){}
void luaD_gcIM(TObject *o){}
void luaD_init(void){}
void luaH_free(Hash *frees){}
void luaT_init(void){}

/*
* the code below avoids the lexer and the parser.
* useful if you only want to load binary files. this works for lua.c too.
*/

#ifdef NOPARSER
/* avoid llex.o lstx.o */
#include "lparser.h"

int lua_debug=0;

void luaX_init(void){}
void luaY_init(void){}

TProtoFunc* luaY_parser(ZIO* z)
{
 lua_error("parser not loaded");
}
#endif
