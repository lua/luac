/*
** luac.h
** definitions for luac compiler
** $Id$
*/

#include "inout.h"
#include "mem.h"
#include "opcode.h"
#include "table.h"

#define VarStr(i)	(lua_table[i].varname->str)
#define VarLoc(i)	(lua_table[i].varname->varindex)
#define StrStr(i)	(lua_constant[i]->str)
#define StrLoc(i)	(lua_constant[i]->constindex)

extern Word lua_ntable;
extern Word lua_nconstant;

void DumpHeader(FILE *D);
void DumpFunction(TFunc *tf, FILE *D);
void PrintFunction(TFunc *tf);

#define	TEST_WORD	0x1234
#define	TEST_FLOAT	1.234567890e-23
#define	SIGNATURE	"Lua"
#define	VERSION		0x23
#define ESC		0x1b
