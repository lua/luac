/*
** luac.h
** definitions for luac compiler
** $Id: luac.h,v 1.4 1996/03/06 15:59:30 lhf Exp lhf $
*/

#include "inout.h"
#include "mem.h"
#include "opcode.h"
#include "table.h"
#include "undump.h"

#define VarStr(i)	(lua_table[i].varname->str)
#define VarLoc(i)	(lua_table[i].varname->varindex)
#define StrStr(i)	(lua_constant[i]->str)
#define StrLoc(i)	(lua_constant[i]->constindex)

extern Word lua_ntable;
extern Word lua_nconstant;
extern int lua_debug;

void DumpHeader(FILE* D);
void DumpFunction(TFunc* tf, FILE* D);
void PrintFunction(TFunc* tf);
