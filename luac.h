/*
** luac.h
** definitions for luac compiler
** $Id: luac.h,v 1.6 1997/04/14 14:34:45 lhf Exp lhf $
*/

#include "inout.h"
#include "luamem.h"
#include "opcode.h"
#include "table.h"
#include "undump.h"

#define VarStr(i)	(lua_table[i].varname->str)
#define VarLoc(i)	(lua_table[i].varname->u.s.varindex)
#define StrStr(i)	(lua_constant[i]->str)
#define StrLoc(i)	(lua_constant[i]->u.s.constindex)

extern Word lua_ntable;
extern Word lua_nconstant;
extern int lua_debug;

void LinkFunctions(TFunc* tf);
void PrintFunction(TFunc* tf);
void DumpHeader(FILE* D);
void DumpFunction(TFunc* tf, FILE* D);
