/*
** $Id: dump.c,v 1.27 2000/04/24 19:32:58 lhf Exp lhf $
** save bytecodes to file
** See Copyright Notice in lua.h
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

#ifdef OLD_ANSI
#define strerror(e)     "(no error message provided by operating system)"
#endif

#define DumpBlock(b,size,D)	fwrite(b,size,1,D)
#define	DumpInt			DumpLong
#define	DumpByte		fputc

static void DumpWord(int i, FILE* D)
{
 int hi= 0x0000FF & (i>>8);
 int lo= 0x0000FF &  i;
 DumpByte(hi,D);
 DumpByte(lo,D);
}

static void DumpLong(long i, FILE* D)
{
 int hi= 0x00FFFF & (i>>16);
 int lo= 0x00FFFF & i;
 DumpWord(hi,D);
 DumpWord(lo,D);
}

static void DumpNumber(Number x, FILE* D, const Proto* tf)
{
 char b[256];
 int n;
 sprintf(b,NUMBER_FMT"%n",x,&n);
 luaU_str2d(L,b,tf->source->str);	/* help lundump not to fail */
 DumpByte(n,D);
 DumpBlock(b,n,D);
}

static void DumpString(TString* s, FILE* D)
{
 if (s==NULL || s->str==NULL)
  DumpLong(0,D);
 else
 {
  long size=s->u.s.len+1;		/* include trailing '\0' */
  DumpLong(size,D);
  DumpBlock(s->str,size,D);
 }
}

static void DumpCode(const Proto* tf, FILE* D)
{
 int size=luaU_codesize(tf);
 Instruction t=TEST_CODE;
 DumpLong(size,D);
 DumpBlock(tf->code,size*sizeof(*tf->code),D);
 DumpBlock(&t,sizeof(t),D);
}

static void DumpLocals(const Proto* tf, FILE* D)
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
   DumpString(v->varname,D);
  }
 }
}

static void DumpFunction(const Proto* tf, FILE* D, int native);

static void DumpConstants(const Proto* tf, FILE* D, int native)
{
 int i,n;
 DumpInt(n=tf->nkstr,D);
 for (i=0; i<n; i++)
  DumpString(tf->kstr[i],D);
 DumpInt(n=tf->nknum,D);
 if (native)
  DumpBlock(tf->knum,n*sizeof(*tf->knum),D);
 else
  for (i=0; i<n; i++)
   DumpNumber(tf->knum[i],D,tf);
 DumpInt(n=tf->nkproto,D);
 for (i=0; i<n; i++)
  DumpFunction(tf->kproto[i],D,native);
}

static void DumpFunction(const Proto* tf, FILE* D, int native)
{
 DumpString(tf->source,D);
 DumpInt(tf->lineDefined,D);
 DumpInt(tf->numparams,D);
 DumpByte(tf->is_vararg,D);
 DumpInt(tf->maxstacksize,D);
 DumpCode(tf,D);
 DumpLocals(tf,D);
 DumpConstants(tf,D,native);
 if (ferror(D))
  luaL_verror(L,"write error" IN ": %s (errno=%d)",INLOC,strerror(errno),errno);
}

static void DumpHeader(FILE* D, int native)
{
 DumpByte(ID_CHUNK,D);
 fputs(SIGNATURE,D);
 DumpByte(VERSION,D);
 DumpByte(sizeof(Instruction),D);
 DumpByte(SIZE_INSTRUCTION,D);
 DumpByte(SIZE_OP,D);
 DumpByte(SIZE_B,D);
 if (native)
 {
  Number f=TEST_NUMBER;
  DumpByte(sizeof(Number),D);
  DumpBlock(&f,sizeof(f),D);
 }
 else
  DumpByte(0,D);
}

void luaU_dumpchunk(const Proto* Main, FILE* D, int native)
{
 DumpHeader(D,native);
 DumpFunction(Main,D,native);
}
