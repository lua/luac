/*
** $Id: luac.h,v 1.7 1999/03/08 11:08:43 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#include "lauxlib.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

typedef struct
{
 char* name;
 int op;
 int class;
 int args;
 int arg;
 int arg2;
} Opcode;

void luaU_dumpchunk(TProtoFunc* Main, FILE* D);
void luaU_printchunk(TProtoFunc* Main);
void luaU_optchunk(TProtoFunc* Main);
void luaU_testchunk(TProtoFunc* Main);
int luaU_opcodeinfo(TProtoFunc* tf, Byte* p, Opcode* I, char* xFILE, int xLINE);
int luaU_codesize(TProtoFunc* tf);

#define INFO(tf,p,I)	luaU_opcodeinfo(tf,p,I,__FILE__,__LINE__)

/* fake (but convenient) opcodes */
#define NOP	255
#define STACK	(-1)
#define ARGS	(-2)
#define VARARGS	(-3)
