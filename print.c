/*
** $Id: print.c,v 1.35 2001/07/19 14:34:06 lhf Exp $
** print bytecodes
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>

#include "luac.h"
#include "ldebug.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"

#define Sizeof(x)	((int)sizeof(x))

static void PrintString(const Proto* f, int n)
{
 const char* s=svalue(&f->k[n]);
 putchar('"');
 for (; *s; s++)
 {
  switch (*s)
  {
   case '"': printf("\\\""); break;
   case '\a': printf("\\a"); break;
   case '\b': printf("\\b"); break;
   case '\f': printf("\\f"); break;
   case '\n': printf("\\n"); break;
   case '\r': printf("\\r"); break;
   case '\t': printf("\\t"); break;
   case '\v': printf("\\v"); break;
   default: putchar(*s); break;
  }
 }
 putchar('"');
}

static void PrintConstant(const Proto* f, int i)
{
 const TObject* o=&f->k[i];
 switch (ttype(o))
 {
  case LUA_TNUMBER:
	printf(LUA_NUMBER_FMT,nvalue(o));
	break;
  case LUA_TSTRING:
	PrintString(f,i);
	break;
  case LUA_TNIL:
	printf("nil");
	break;
  default:				/* cannot happen */
	printf("? type=%d",ttype(o));
	break;
 }
}

static void PrintLocal(const Proto* f, int n, int pc)
{
 const char* s=luaF_getlocalname(f,n+1,pc);
 if (s!=NULL) printf("\t; $%d [%s]",n,s);
}

static void PrintCode(const Proto* f)
{
 const Instruction* code=f->code;
 int pc,n=f->sizecode;
 for (pc=0; pc<n; pc++)
 {
  Instruction i=code[pc];
  OpCode o=GET_OPCODE(i);
  int a=GETARG_A(i);
  int b=GETARG_B(i);
  int c=GETARG_C(i);
  int bc=GETARG_Bc(i);
  int sbc=GETARG_sBc(i);
  int line=luaG_getline(f->lineinfo,pc,1,NULL);
#if 0
  printf("%0*lX",Sizeof(i)*2,i);
#endif
  printf("\t%d\t",pc+1);
  if (line>=0) printf("[%d]\t",line); else printf("[-]\t");
  printf("%-11s\t",luaP_opnames[o]);
  switch (getOpMode(o))
  {
   case iABC:	printf("%d %d %d",a,b,c); break;
   case iABc:	printf("%d %d",a,bc); break;
   case iAsBc:	printf("%d %d",a,sbc); break;
  }
  switch (o)
  {
   case OP_LOADK:
   case OP_GETGLOBAL:
   case OP_SETGLOBAL:
    printf("\t; "); PrintConstant(f,bc);
    break;
   case OP_GETTABLE:
   case OP_SETTABLE:
   case OP_SELF:
   case OP_ADD:
   case OP_SUB:
   case OP_MUL:
   case OP_DIV:
   case OP_POW:
   case OP_TESTEQ:
   case OP_TESTNE:
   case OP_TESTLT:
   case OP_TESTLE:
   case OP_TESTGT:
   case OP_TESTGE:
    if (c>=MAXSTACK) { printf("\t; "); PrintConstant(f,c-MAXSTACK); }
    break;
   case OP_JMP:
   case OP_FORLOOP:
   case OP_TFORLOOP:
    printf("\t; to %d",sbc+pc+2);
    break;
   case OP_CLOSURE:
    printf("\t; %p",f->p[bc]);
    break;
   default:
    break;
  }
  printf("\n");
 }
}

#define IsMain(f)	(f->lineDefined==0)

#define SS(x)	(x==1)?"":"s"
#define S(x)	x,SS(x)

static void PrintHeader(const Proto* f)
{
 printf("\n%s <%d:%s> (%d instruction%s, %d bytes at %p)\n",
 	IsMain(f)?"main":"function",f->lineDefined,getstr(f->source),
	S(f->sizecode),f->sizecode*Sizeof(Instruction),f);
 printf("%d%s param%s, %d stack%s, %d upvalue%s, ",
	f->numparams,f->is_vararg?"+":"",SS(f->numparams),S(f->maxstacksize),
	S(f->nupvalues));
 printf("%d local%s, %d constant%s, %d function%s, %d line%s\n",
	S(f->sizelocvars),S(f->sizek),S(f->sizep),S(f->sizelineinfo));
}

static void PrintConstants(const Proto* f)
{
 int i,n=f->sizek;
 printf("constants (%d) for %p:\n",n,f);
 for (i=0; i<n; i++)
 {
  printf("\t%d\t",i);
  PrintConstant(f,i);
  printf("\n");
 }
}

static void PrintLocals(const Proto* f)
{
 int i,n=f->sizelocvars;
 printf("locals (%d) for %p:\n",n,f);
 for (i=0; i<n; i++)
 {
  printf("\t%d\t%s\t%d\t%d\n",
  i,getstr(f->locvars[i].varname),f->locvars[i].startpc,f->locvars[i].endpc);
 }
}

#define PrintFunction luaU_printchunk

void PrintFunction(const Proto* f)
{
 int i,n=f->sizep;
 PrintHeader(f);
 PrintCode(f);
 PrintConstants(f);
 PrintLocals(f);
 for (i=0; i<n; i++) PrintFunction(f->p[i]);
}
