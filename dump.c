/*
** $Id: dump.c,v 1.2 1997/12/02 23:18:50 lhf Exp lhf $
** save bytecodes to file
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

static void DumpWord(int i, FILE* D)
{
 Word w=i;
 fputc(w>>8,D);
 fputc(w,D);
}

static void DumpLong(int i, FILE* D)
{
 Word hi=(i>>16)&0x0FFFF;
 Word lo=i&0x0FFFF;
 DumpWord(hi,D);
 DumpWord(lo,D);
}

static void DumpBlock(void* b, int size, FILE* D)
{
 fwrite(b,size,1,D);
}

static void DumpSize(int s, FILE* D)
{
 DumpLong(s,D);
 if (NotWord(s))
  fprintf(stderr,
  "luac: warning: code too long for 16-bit machines (%d bytes)\n",s);
}

static void DumpCode(TProtoFunc* tf, FILE* D)
{
 extern int CodeSize(TProtoFunc*);		/* in print.c */
 int size=CodeSize(tf)+1;
 DumpSize(size,D);
 DumpBlock(tf->code,size,D);
}

static void DumpString(char* s, FILE* D)
{
 if (s==NULL)
  DumpWord(0,D);
 else
 {
  int n=strlen(s)+1;
  if (NotWord(n))
   luaL_verror("string too long (%d bytes): \"%.32s...\"\n",n,s);
  DumpWord(n,D);
  DumpBlock(s,n,D);
 }
}

static void DumpTString(TaggedString* s, FILE* D)
{
 DumpString((s==NULL) ? NULL : s->str,D);
}

static void DumpLocals(TProtoFunc* tf, FILE* D)
{
 int n;
 LocVar* lv;
 for (n=0,lv=tf->locvars; lv && lv->line>=0; lv++) ++n;
 DumpWord(n,D);
 for (lv=tf->locvars; lv && lv->line>=0; lv++)
 {
  DumpWord(lv->line,D);
  DumpTString(lv->varname,D);
 }
}

static void DumpConstants(TProtoFunc* tf, FILE* D)
{
 int i,n=tf->nconsts;
 DumpWord(n,D);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	fputc(ID_NUM,D);
	DumpBlock(&nvalue(o),sizeof(nvalue(o)),D);
	break;
   case LUA_T_STRING:
	fputc(ID_STR,D);
	DumpString(svalue(o),D);
	break;
   case LUA_T_PROTO:
	fputc(ID_FUN,D);
	break;
   default:				/* cannot happen */
#ifdef DEBUG
	luaL_verror("internal error in DumpConstants: "
		"bad constant #%d type=%d\n",i,ttype(o));
#endif
	break;
  }
 }
}

void DumpFunction(TProtoFunc* tf, FILE* D);

static void DumpFunctions(TProtoFunc* tf, FILE* D)
{
 int i,n=tf->nconsts;
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  if (ttype(o)==LUA_T_PROTO)
  {
   fputc(ID_FUNCTION,D);
   DumpWord(i,D);
   DumpFunction(tfvalue(o),D);
  }
 }
 fputc(ID_END,D);
}

void DumpFunction(TProtoFunc* tf, FILE* D)
{
 DumpWord(tf->lineDefined,D);
 DumpTString(tf->fileName,D);
 DumpCode(tf,D);
 DumpConstants(tf,D);
 DumpLocals(tf,D);
 DumpFunctions(tf,D);
}

static void DumpHeader(TProtoFunc* Main, FILE* D)
{
 real r=TEST_FLOAT;
 fputc(ID_CHUNK,D);
 fputs(SIGNATURE,D);
 fputc(VERSION,D);
 fputc(sizeof(r),D);
 fwrite(&r,sizeof(r),1,D);
}

void DumpChunk(TProtoFunc* Main, FILE* D)
{
 DumpHeader(Main,D);
 DumpFunction(Main,D);
}
