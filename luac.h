/*
** $Id: luac.h,v 1.22 2001/11/01 08:50:39 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#include "lua.h"

#include "lobject.h"

/* from dump.c */
void luaU_dumpchunk(const Proto* Main, FILE* D);

/* from print.c */
void luaU_printchunk(const Proto* Main);
