/*
** $Id: opcode.c,v 1.9 1999/05/25 19:58:55 lhf Exp lhf $
** opcode information
** See Copyright Notice in lua.h
*/

#include "luac.h"

enum {					/* for Opcode.args */
 ARGS_NONE,
 ARGS_B,
 ARGS_W,
 ARGS_BB,
 ARGS_WB
};

static const Opcode Info[]=		/* ORDER lopcodes.h */
{
#include "opcode.h"
};

static const Opcode Fake[]=		/* ORDER luac.h */
{
{ "NOP", NOP, NOP, ARGS_NONE, -1, -1 },
{ "STACK", STACK, STACK, ARGS_B, -1, -1 },
{ "ARGS", ARGS, ARGS, ARGS_B, -1, -1 },
{ "VARARGS", VARARGS, VARARGS, ARGS_B, -1, -1 },
};

#define NOPCODES	(sizeof(Info)/sizeof(Info[0]))

int luaU_opcodeinfo(const TProtoFunc* tf, const Byte* p, Opcode* I, const char* xFILE, int xLINE)
{
 Opcode OP;
 const Byte* code=tf->code;
 int op=*p;
 int size=1;
 if (p==code)				/* first byte is STACK */
 {
  OP=Fake[-STACK];
  OP.arg=op;
 }
 else if (p==code+1)			/* second byte is ARGS or VARARGS */
 {
  if (op<ZEROVARARG)
  {
   OP=Fake[-ARGS];
   OP.arg=op;
  }
  else
  {
   OP=Fake[-VARARGS];
   OP.arg=op-ZEROVARARG;
  }
 }
 else if (op==NOP)			/* NOP is fake */
 {
  OP=Fake[0];
 }
 else if (op>=NOPCODES)			/* cannot happen */
 {
  luaL_verror("[%s:%d] bad opcode %d at pc=%d" IN,
	xFILE,xLINE,op,(int)(p-code),INLOC);
  return 0;
 }
 else					/* ordinary opcode */
 {
  OP=Info[op];
  switch (OP.args)
  {
   case ARGS_NONE:	size=1;
    break;
   case ARGS_B:		size=2;	OP.arg=p[1];
    break;
   case ARGS_W:		size=3;	OP.arg=(p[1]<<8)+p[2];
    break;
   case ARGS_BB:	size=3;	OP.arg=p[1];		OP.arg2=p[2];
    break;
   case ARGS_WB:	size=4;	OP.arg=(p[1]<<8)+p[2];	OP.arg2=p[3];
    break;
   default:				/* cannot happen */
    luaL_verror("[%s:%d] bad args %d for %s at pc=%d" IN,
	__FILE__,__LINE__,OP.args,OP.name,(int)(p-code),INLOC);
    break;
  }
 }
 *I=OP;
 return size;
}

int luaU_codesize(const TProtoFunc* tf)
{
 const Byte* code=tf->code;
 const Byte* p=code;
 for (;;)
 {
  Opcode OP;
  p+=INFO(tf,p,&OP);
  if (OP.op==ENDCODE) break;
 }
 return p-code;
}
