/*
** $Id: dump.c,v 1.31 2001/03/15 17:29:16 lhf Exp lhf $
** save bytecodes to file
** See Copyright Notice in lua.h
*/

#ifdef WRITETO
#define STATIC	static
#else
#include <stdio.h>
#include "luac.h"
#define WRITETO	FILE*
#define WRITE1	fputc
#define WRITE	fwrite
#define STATIC
#endif

#include "lobject.h"
#include "lundump.h"

#define	DumpByte(b,D)		WRITE1(b,D)
#define DumpBlock(b,size,D)	WRITE(b,size,1,D)
#define DumpVector(b,n,size,D)	WRITE(b,size,n,D)
#define DumpLiteral(s,D)	WRITE("" s,(sizeof(s))-1,1,D)
#define DumpChunk		luaU_dumpchunk

static void DumpInt(int x, WRITETO D)
{
 DumpBlock(&x,sizeof(x),D);
}

static void DumpSize(size_t x, WRITETO D)
{
 DumpBlock(&x,sizeof(x),D);
}

static void DumpNumber(lua_Number x, WRITETO D)
{
 DumpBlock(&x,sizeof(x),D);
}

static void DumpString(TString* s, WRITETO D)
{
 if (s==NULL || getstr(s)==NULL)
  DumpSize(0,D);
 else
 {
  size_t size=s->len+1;			/* include trailing '\0' */
  DumpSize(size,D);
  DumpBlock(getstr(s),size,D);
 }
}

static void DumpCode(const Proto* tf, WRITETO D)
{
 DumpInt(tf->sizecode,D);
 DumpVector(tf->code,tf->sizecode,sizeof(*tf->code),D);
}

static void DumpLocals(const Proto* tf, WRITETO D)
{
 int i,n=tf->sizelocvars;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  DumpString(tf->locvars[i].varname,D);
  DumpInt(tf->locvars[i].startpc,D);
  DumpInt(tf->locvars[i].endpc,D);
 }
}

static void DumpLines(const Proto* tf, WRITETO D)
{
 DumpInt(tf->sizelineinfo,D);
 DumpVector(tf->lineinfo,tf->sizelineinfo,sizeof(*tf->lineinfo),D);
}

static void DumpFunction(const Proto* tf, WRITETO D);

static void DumpConstants(const Proto* tf, WRITETO D)
{
 int i,n;
 DumpInt(n=tf->sizekstr,D);
 for (i=0; i<n; i++)
  DumpString(tf->kstr[i],D);
 DumpInt(tf->sizeknum,D);
 DumpVector(tf->knum,tf->sizeknum,sizeof(*tf->knum),D);
 DumpInt(n=tf->sizekproto,D);
 for (i=0; i<n; i++)
  DumpFunction(tf->kproto[i],D);
}

static void DumpFunction(const Proto* tf, WRITETO D)
{
 DumpString(tf->source,D);
 DumpInt(tf->lineDefined,D);
 DumpInt(tf->nupvalues,D);
 DumpInt(tf->numparams,D);
 DumpByte(tf->is_vararg,D);
 DumpInt(tf->maxstacksize,D);
 DumpLocals(tf,D);
 DumpLines(tf,D);
 DumpConstants(tf,D);
 DumpCode(tf,D);
}

static void DumpHeader(WRITETO D)
{
 DumpByte(ID_CHUNK,D);
 DumpLiteral(SIGNATURE,D);
 DumpByte(VERSION,D);
 DumpByte(luaU_endianness(),D);
 DumpByte(sizeof(int),D);
 DumpByte(sizeof(size_t),D);
 DumpByte(sizeof(Instruction),D);
 DumpByte(SIZE_INSTRUCTION,D);
 DumpByte(SIZE_OP,D);
 DumpByte(SIZE_B,D);
 DumpByte(sizeof(lua_Number),D);
 DumpNumber(TEST_NUMBER,D);
}

STATIC void DumpChunk(const Proto* Main, WRITETO D)
{
 DumpHeader(D);
 DumpFunction(Main,D);
}
