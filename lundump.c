/*
** $Id$
** load bytecodes from files
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include "lauxlib.h"
#include "lfunc.h"
#include "lmem.h"
#include "lstring.h"
#include "lundump.h"

#define Real real

static char* Filename;
static int SwapNumber=0;
static int LoadFloat=1;

static void unexpectedEOZ(void)
{
 luaL_verror("unexpected end of binary file %s",Filename);
}

static int LoadWord(ZIO* Z)
{
 int hi=zgetc(Z);
 int lo=zgetc(Z);
 if (lo==EOZ) unexpectedEOZ();
 return (hi<<8)|lo;
}

static void* LoadBlock(int size, ZIO* Z)
{
 void* b=luaM_malloc(size);
 if (zread(Z,b,size)!=0) unexpectedEOZ();
 return b;
}

static int LoadSize(ZIO* Z)
{
 int hi=LoadWord(Z);
 int lo=LoadWord(Z);
 int s=(hi<<16)|lo;
 if (hi!=0 && s==lo)
  luaL_verror("code too long (%ld bytes)",(hi<<16)|(long)lo);
 return s;
}

static char* LoadString(ZIO* Z)
{
 int size=LoadWord(Z);
 if (size==0)
  return NULL;
 else
 {
  char* b=luaM_buffer(size);
  if (zread(Z,b,size)!=0) unexpectedEOZ();
  return b;
 }
}

static TaggedString* LoadTString(ZIO* Z)
{
 char* s=LoadString(Z);
 return (s==NULL) ? NULL : luaS_new(s);
}

static void SwapFloat(float* f)
{
 Byte* p=(Byte*)f;
 Byte* q=p+sizeof(float)-1;
 Byte t;
 t=*p; *p++=*q; *q--=t;
 t=*p; *p++=*q; *q--=t;
}

static void SwapDouble(double* f)
{
 Byte* p=(Byte*)f;
 Byte* q=p+sizeof(double)-1;
 Byte t;
 t=*p; *p++=*q; *q--=t;
 t=*p; *p++=*q; *q--=t;
 t=*p; *p++=*q; *q--=t;
 t=*p; *p++=*q; *q--=t;
}

static Real LoadNumber(ZIO* Z)
{
 if (LoadFloat)
 {
  float f;
  if (zread(Z,&f,sizeof(f))!=0) unexpectedEOZ();
  if (SwapNumber) SwapFloat(&f);
  return f;
 }
 else
 {
  double f;
  if (zread(Z,&f,sizeof(f))!=0) unexpectedEOZ();
  if (SwapNumber) SwapDouble(&f);
  return f;
 }
}

static void LoadLocals(TFunc* tf, ZIO* Z)
{
 int i,n=LoadWord(Z);
 if (n==0) return;
 tf->locvars=luaM_newvector(n+1,LocVar);
 for (i=0; i<n; i++)
 {
  tf->locvars[i].line=LoadWord(Z);
  tf->locvars[i].varname=LoadTString(Z);
 }
 tf->locvars[i].line=-1;		/* flag end of vector */
 tf->locvars[i].varname=NULL;
}

static void LoadConstants(TFunc* tf, ZIO* Z)
{
 int i,n=LoadWord(Z);
 tf->nconsts=n;
 if (n==0) return;
 tf->consts=luaM_newvector(n,TObject);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  int c=zgetc(Z);
  switch (c)
  {
   case ID_NUM:
	ttype(o)=LUA_T_NUMBER;
	nvalue(o)=LoadNumber(Z);
	break;
   case ID_STR:
	ttype(o)=LUA_T_STRING;	
	tsvalue(o)=LoadTString(Z);
	break;
   case ID_FUN:
	ttype(o)=LUA_T_PROTO;
	tfvalue(o)=NULL;
	break;
   default:				/* cannot happen */
	luaL_verror("internal error in LoadConstants: "
		"bad constant #%d type=%d ('%c')\n",i,c,c);
	break;
  }
 }
}

static TFunc* LoadFunction(TaggedString* chunkname, ZIO* Z);

static void LoadFunctions(TFunc* tf, ZIO* Z)
{
 while (zgetc(Z)==ID_FUNCTION)
 {
  int i=LoadWord(Z); 
  TFunc* t=LoadFunction(tf->fileName,Z);
  TObject* o=tf->consts+i;
  tfvalue(o)=t;
 }
}

static TFunc* LoadFunction(TaggedString* chunkname, ZIO* Z)
{
 TFunc* tf=luaF_newproto();
 tf->fileName=chunkname;
 tf->lineDefined=LoadWord(Z);
 tf->code=LoadBlock(LoadSize(Z),Z);
 LoadConstants(tf,Z);
 LoadLocals(tf,Z);
 LoadFunctions(tf,Z);
 return tf;
}

static void LoadSignature(ZIO* Z)
{
 char* s=SIGNATURE;
 while (*s!=0 && zgetc(Z)==*s)
  ++s;
 if (*s!=0) luaL_verror("bad signature in binary file %s",Filename);
}

static TaggedString* LoadHeader(ZIO* Z)
{
 int version,sizeofR;
 LoadSignature(Z);
 version=zgetc(Z);
 if (version>VERSION)
  luaL_verror(
	"binary file %s too new: version=0x%02x; expected at most 0x%02x",
	Filename,version,VERSION);
 if (version<0x31)			/* major change in 3.1 */
  luaL_verror(
	"binary file %s too old: version=0x%02x; expected at least 0x%02x",
	Filename,version,0x31);
 sizeofR=zgetc(Z);			/* test float representation */
 if (sizeofR==sizeof(float))
 {
  float f,tf=TEST_FLOAT;
  if (zread(Z,&f,sizeof(f))!=0) unexpectedEOZ();
  if (f!=tf)
  {
   SwapFloat(&f);
   if (f!=tf)
    luaL_verror("unknown float representation in binary file %s",Filename);
   SwapNumber=1;
  }
  LoadFloat=1;
 }
 else if (sizeofR==sizeof(double))
 {
  double f,tf=TEST_FLOAT;
  if (zread(Z,&f,sizeof(f))!=0) unexpectedEOZ();
  if (f!=tf)
  {
   SwapDouble(&f);
   if (f!=tf)
    luaL_verror("unknown float representation in binary file %s",Filename);
   SwapNumber=1;
  }
  LoadFloat=0;
 }
 else
  luaL_verror(
       "floats in binary file %s have %d bytes; "
       "expected %d (float) or %d (double)",
       Filename,sizeofR,sizeof(float),sizeof(double));
 return LoadTString(Z);
}

static TFunc* LoadChunk(ZIO* Z)
{
 return LoadFunction(LoadHeader(Z),Z);
}

/*
** load one chunk from a file or buffer
** return main if ok and NULL at EOF
*/
TFunc* luaU_undump1(ZIO* Z, char* filename)
{
 int c=zgetc(Z);
 Filename=filename;
 if (c==ID_CHUNK)
  return LoadChunk(Z);
 else if (c!=EOZ)
  luaL_verror("%s is not a lua binary file",filename);
 return NULL;
}
