/*
** $Id: dump.c,v 1.6 1998/02/06 20:05:39 lhf Exp lhf $
** save bytecodes to file
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

#define NotWord(x)		((unsigned short)x!=x)
#define DumpBlock(b,size,D)	fwrite(b,size,1,D)
#define	DumpNative(t,D)		DumpBlock(&t,sizeof(t),D)

/* LUA_NUMBER */
/* if you change the definition of real, make sure you set ID_NUMBER
* accordingly lundump.h, specially if sizeof(long)!=4.
* for types other than the ones listed below, you'll have to write your own
* dump and undump routines.
*/

#if   ID_NUMBER==ID_REAL4
	#define	DumpNumber	DumpFloat
#elif ID_NUMBER==ID_REAL8
	#define	DumpNumber	DumpDouble
#elif ID_NUMBER==ID_INT4
	#define	DumpNumber	DumpLong
#elif ID_NUMBER==ID_NATIVE
	#define	DumpNumber	DumpNative
#else
	#define	DumpNumber	DumpWhat
#endif

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

/* LUA_NUMBER */
/* assumes sizeof(long)==4 and sizeof(float)==4 (IEEE) */
static void DumpFloat(float f, FILE* D)
{
 long l=*(long*)&f;
 DumpLong(l,D);
}

/* LUA_NUMBER */
/* assumes sizeof(long)==4 and sizeof(double)==8 (IEEE) */
static void DumpDouble(double f, FILE* D)
{
 long* l=(long*)&f;
 int x=1;
 if (*(char*)&x==1)			/* little-endian */
 {
  DumpLong(l[1],D);
  DumpLong(l[0],D);
 }
 else					/* big-endian */
 {
  DumpLong(l[0],D);
  DumpLong(l[1],D);
 }
}

static void DumpCode(TProtoFunc* tf, FILE* D)
{
 extern int CodeSize(TProtoFunc*);	/* in print.c */
 int size=CodeSize(tf);
 if (NotWord(size))
  fprintf(stderr,"luac: warning: "
	"\"%s\":%d code too long for 16-bit machines (%d bytes)\n",
	tf->fileName->str,tf->lineDefined,size);
 DumpLong(size,D);
 DumpBlock(tf->code,size,D);
}

static void DumpString(char* s, FILE* D)
{
 if (s==NULL)
  DumpWord(0,D);
 else
 {
  int size=strlen(s)+1;			/* includes trailing '\0' */
  if (NotWord(size))
   luaL_verror("string too long (%d bytes): \"%.32s...\"\n",size,s);
  DumpWord(size,D);
  DumpBlock(s,size,D);
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

static void DumpFunction(TProtoFunc* tf, FILE* D);

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
	DumpNumber(nvalue(o),D);
	break;
   case LUA_T_STRING:
	fputc(ID_STR,D);
	DumpString(svalue(o),D);
	break;
   case LUA_T_PROTO:
	fputc(ID_FUN,D);
	DumpFunction(tfvalue(o),D);
	break;
   default:				/* cannot happen */
	INTERNAL_ERROR("bad constant");
	break;
  }
 }
}

static void DumpFunction(TProtoFunc* tf, FILE* D)
{
 DumpWord(tf->lineDefined,D);
 DumpTString(tf->fileName,D);
 DumpCode(tf,D);
 DumpLocals(tf,D);
 DumpConstants(tf,D);
}

static void DumpHeader(TProtoFunc* Main, FILE* D)
{
 real t=TEST_NUMBER;
 fputc(ID_CHUNK,D);
 fputs(SIGNATURE,D);
 fputc(VERSION,D);
 fputc(ID_NUMBER,D);
 fputc(sizeof(t),D);
 DumpNumber(t,D);
}

void DumpChunk(TProtoFunc* Main, FILE* D)
{
 DumpHeader(Main,D);
 DumpFunction(Main,D);
}
