/*
** $Id: luac.h,v 1.3 1998/01/13 20:05:24 lhf Exp lhf $
** definitions for luac
** See Copyright Notice in lua.h
*/

#include "lauxlib.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#ifdef DEBUG
#define INTERNAL_ERROR(s)	\
	luaL_verror("internal error: %s [%s:%s]",s?s:"",__FILE__,__LINE)
#else
#define INTERNAL_ERROR(s)
#endif

typedef struct
{
 char* name;
 int size;
 int op;
 int class;
 int arg;
 int arg2;
} Opcode;

int OpcodeInfo(TProtoFunc* tf, Byte* p, Opcode* I, char* xFILE, int xLINE);

#define INFO(tf,p,I)	OpcodeInfo(tf,p,I,__FILE__,__LINE__)

#define NOP	255
#define STACK	-1
#define ARGS	-2
#define VARARGS	-3
