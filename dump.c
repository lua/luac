/*
** $Id: dump.c,v 1.32 2001/06/28 13:55:17 lhf Exp lhf $
** save bytecodes
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
#include "lopcodes.h"
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
  size_t size=s->tsv.len+1;		/* include trailing '\0' */
  DumpSize(size,D);
  DumpBlock(getstr(s),size,D);
 }
}

static void DumpCode(const Proto* f, WRITETO D)
{
 DumpInt(f->sizecode,D);
 DumpVector(f->code,f->sizecode,sizeof(*f->code),D);
}

static void DumpLocals(const Proto* f, WRITETO D)
{
 int i,n=f->sizelocvars;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  DumpString(f->locvars[i].varname,D);
  DumpInt(f->locvars[i].startpc,D);
  DumpInt(f->locvars[i].endpc,D);
 }
}

static void DumpLines(const Proto* f, WRITETO D)
{
 DumpInt(f->sizelineinfo,D);
 DumpVector(f->lineinfo,f->sizelineinfo,sizeof(*f->lineinfo),D);
}

static void DumpFunction(const Proto* f, const TString* p, WRITETO D);

static void DumpConstants(const Proto* f, WRITETO D)
{
 int i,n;
 DumpInt(n=f->sizek,D);
 for (i=0; i<n; i++)
 {
  const TObject* o=&f->k[i];
  DumpByte(ttype(o),D);
  switch (ttype(o))
  {
   case LUA_TNUMBER:
	DumpNumber(nvalue(o),D);
	break;
   case LUA_TSTRING:
	DumpString(tsvalue(o),D);
	break;
   default:
	lua_assert(0);			/* cannot happen */
	break;
  }
 }
 DumpInt(n=f->sizep,D);
 for (i=0; i<n; i++) DumpFunction(f->p[i],f->source,D);
}

static void DumpFunction(const Proto* f, const TString* p, WRITETO D)
{
 DumpString((f->source==p) ? NULL : f->source,D);
 DumpInt(f->lineDefined,D);
 DumpInt(f->nupvalues,D);
 DumpInt(f->numparams,D);
 DumpByte(f->is_vararg,D);
 DumpInt(f->maxstacksize,D);
 DumpLocals(f,D);
 DumpLines(f,D);
 DumpConstants(f,D);
 DumpCode(f,D);
}

static void DumpHeader(WRITETO D)
{
 DumpLiteral(LUA_SIGNATURE,D);
 DumpByte(VERSION,D);
 DumpByte(luaU_endianness(),D);
 DumpByte(sizeof(int),D);
 DumpByte(sizeof(size_t),D);
 DumpByte(sizeof(Instruction),D);
 DumpByte(SIZE_OP,D);
 DumpByte(SIZE_A,D);
 DumpByte(SIZE_B,D);
 DumpByte(SIZE_C,D);
 DumpByte(sizeof(lua_Number),D);
 DumpNumber(TEST_NUMBER,D);
}

STATIC void DumpChunk(const Proto* Main, WRITETO D)
{
 DumpHeader(D);
 DumpFunction(Main,NULL,D);
}
