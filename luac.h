/*
** luac.h
** definitions for luac compiler
** $Id: luac.h,v 1.3 1996/03/01 03:40:47 lhf Exp lhf $
*/

#include "inout.h"
#include "mem.h"
#include "opcode.h"
#include "table.h"

#define VarStr(i)	(lua_table[i].varname->str)
#define VarLoc(i)	(lua_table[i].varname->varindex)
#define StrStr(i)	(lua_constant[i]->str)
#define StrLoc(i)	(lua_constant[i]->constindex)
#define IsMain(f)	(f->lineDefined==0)

extern Word lua_ntable;
extern Word lua_nconstant;
extern int lua_debug;

void DumpHeader(FILE* D);
void DumpFunction(TFunc* tf, FILE* D);
void PrintFunction(TFunc* tf);

/* definitions for chunk headers */

#define ID_CHUNK	27		/* ESC */
#define ID_FUN		'F'
#define ID_VAR		'V'
#define ID_STR		'S'
#define ID_LOC		'L'
#define	SIGNATURE	"Lua"
#define	VERSION		0x23		/* 2.3 */
#define	TEST_WORD	0x1234		/* a word for testing byte ordering */
#define	TEST_FLOAT	0.123456789e-23	/* a float for testing representation */
