/*
** $Id: dump.c,v 1.23 1999/12/02 18:45:03 lhf Exp lhf $
** save bytecodes to file
** See Copyright Notice in lua.h
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "luac.h"

#ifdef OLD_ANSI
#define strerror(e)     "(no error message provided by operating system)"
#endif

#define DumpBlock(b,size,D)	fwrite(b,size,1,D)
#define	DumpInt			DumpLong

static void DumpWord(int i, FILE* D)
{
 int hi= 0x0000FF & (i>>8);
 int lo= 0x0000FF &  i;
 fputc(hi,D);
 fputc(lo,D);
}

static void DumpLong(long i, FILE* D)
{
 int hi= 0x00FFFF & (i>>16);
 int lo= 0x00FFFF & i;
 DumpWord(hi,D);
 DumpWord(lo,D);
}

static void DumpNumber(real x, FILE* D, int native, const TProtoFunc* tf)
{
 if (native)
  DumpBlock(&x,sizeof(x),D);
 else
 {
  char b[256];
  int n;
  sprintf(b,NUMBER_FMT"%n",x,&n);
  luaU_str2d(L,b,tf->source->str);	/* help lundump not to fail */
  fputc(n,D);
  DumpBlock(b,n,D);
 }
}

static void DumpCode(const TProtoFunc* tf, FILE* D)
{
 int size=luaU_codesize(tf);
 DumpLong(size,D);
 DumpBlock(tf->code,size,D);
}

static void DumpString(const char* s, int size, FILE* D)
{
 if (s==NULL)
  DumpLong(0,D);
 else
 {
  DumpLong(size,D);
  DumpBlock(s,size,D);
 }
}

static void DumpTString(TaggedString* s, FILE* D)
{
 if (s==NULL)
  DumpString(NULL,0,D);
 else
  DumpString(s->str,s->u.s.len+1,D);
}

static void DumpLocals(const TProtoFunc* tf, FILE* D)
{
 if (tf->locvars==NULL)
  DumpInt(0,D);
 else
 {
  const LocVar* v;
  int n=0;
  for (v=tf->locvars; v->line>=0; v++)
   ++n;
  DumpInt(n,D);
  for (v=tf->locvars; v->line>=0; v++)
  {
   DumpInt(v->line,D);
   DumpTString(v->varname,D);
  }
 }
}

static void DumpFunction(const TProtoFunc* tf, FILE* D, int native);

static void DumpConstants(const TProtoFunc* tf, FILE* D, int native)
{
 int i,n=tf->nconsts;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  const TObject* o=tf->consts+i;
  fputc(-ttype(o),D);			/* ttype(o) is negative - ORDER LUA_T */
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	DumpNumber(nvalue(o),D,native,tf);
	break;
   case LUA_T_STRING:
	DumpTString(tsvalue(o),D);
	break;
   case LUA_T_LPROTO:
	DumpFunction(tfvalue(o),D,native);
	break;
   case LUA_T_NIL:
	break;
   default:				/* cannot happen */
	luaU_badconstant(L,"dump",i,o,tf);
	break;
  }
 }
}

static void DumpFunction(const TProtoFunc* tf, FILE* D, int native)
{
 DumpInt(tf->lineDefined,D);
 DumpTString(tf->source,D);
 DumpCode(tf,D);
 DumpLocals(tf,D);
 DumpConstants(tf,D,native);
 if (ferror(D))
  luaL_verror(L,"write error" IN ": %s (errno=%d)",INLOC,strerror(errno),errno);
}

static void DumpHeader(const TProtoFunc* Main, FILE* D, int native)
{
 fputc(ID_CHUNK,D);
 fputs(SIGNATURE,D);
 fputc(VERSION,D);
 if (native)
 {
  fputc(sizeof(real),D);
  DumpNumber(TEST_NUMBER,D,native,Main);
 }
 else
  fputc(0,D);
}

void luaU_dumpchunk(const TProtoFunc* Main, FILE* D, int native)
{
 DumpHeader(Main,D,native);
 DumpFunction(Main,D,native);
}
