/*
** $Id: $
** load bytecodes from files
** Copyrightnotice
*/

#include <stdio.h>
#include "auxlib.h"
#include "lfunc.h"
#include "lglobal.h"
#include "lmem.h"
#include "lstring.h"
#include "lundump.h"

#define Real double

static TFunc* Main;
static char* Filename;
static void (*SwapNumber)();
static Real (*LoadNumber)(ZIO* Z);

static void unexpectedEOZ(void)
{
 luaL_verror("unexpected end of binary file %s",Filename);
}

static void Unthread(Byte* code, int i, int v)
{
 Byte* p=code+i;
 while (i!=0)
 {
  Word w;
  Get_word(w,p);
  i=w; w=v;
  p[0]=w; p[1]=w>>8;
  p-=i;
 }
}

static int LoadWord(ZIO* Z)
{
 int hi=zgetc(Z);
 int lo=zgetc(Z);
 if (lo==EOZ) unexpectedEOZ();
 return (hi<<8)|lo;
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

static void* LoadBlock(int size, ZIO* Z)
{
 void* b=luaM_malloc(size);
 if (zread(Z,b,size)!=0) unexpectedEOZ();
 return b;
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

static void SwapNop(void* f)
{
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

static Real LoadFloat(ZIO* Z)
{
 float f;
 if (zread(Z,&f,sizeof(f))!=0) unexpectedEOZ();
 SwapNumber(&f);
 return (Real)f;
}

static Real LoadDouble(ZIO* Z)
{
 double f;
 if (zread(Z,&f,sizeof(f))!=0) unexpectedEOZ();
 SwapNumber(&f);
 return (Real)f;
}

#define LoadLong LoadSize

static void LoadGlobals(int longcode, TFunc* tf, ZIO* Z)
{
 int i,n=LoadWord(Z);
 for (i=0; i<n; i++)
 {
  int i= longcode ? LoadLong(Z) : LoadWord(Z);
  char* s=LoadString(Z);
  int v=luaG_findsymbolbyname(s);
  Unthread(tf->code,i,v);
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
 tf->locvars[i].line=-1;			/* flag end of vector */
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
    ttype(o)=LUA_T_FUNCTION;
    tfvalue(o)=NULL;
    break;
   default:					/* cannot happen */
    luaL_verror("bad constant #%d type=%d ('%c')\n",i,c,c);
    break;
  }
 }
}

static void LoadFunction(ZIO* Z)
{
 TFunc* tf=luaF_new();
 int size=LoadSize(Z);
 tf->lineDefined=LoadWord(Z);
 if (IsMain(tf))				/* new main */
 {
  tf->fileName=LoadTString(Z);
  Main=tf;
 }
 else						/* fix reference */
 {
  int i=LoadWord(Z);
  tf->fileName=Main->fileName;
  Main->consts[i].value.tf=tf;
 }
 tf->code=LoadBlock(size,Z);
 LoadGlobals(NotWord(size),tf,Z);
 LoadConstants(tf,Z);
 LoadLocals(tf,Z);
}

static void LoadSignature(ZIO* Z)
{
 char* s=SIGNATURE;
 while (*s!=0 && zgetc(Z)==*s)
  ++s;
 if (*s!=0) luaL_verror("bad signature in binary file %s",Filename);
}

static void LoadHeader(ZIO* Z)
{
 int version,sizeofR;
 LoadSignature(Z);
 version=zgetc(Z);
 if (version>VERSION)
  luaL_verror(
	"binary file %s too new: version=0x%02x; expected at most 0x%02x",
	Filename,version,VERSION);
 if (version<0x31)				/* major change in 3.1 */
  luaL_verror(
	"binary file %s too old: version=0x%02x; expected at least 0x%02x",
	Filename,version,0x31);
 sizeofR=zgetc(Z);				/* test float representation */
 SwapNumber=SwapNop;
 if (sizeofR==sizeof(float))
 {
  float f,tf=TEST_FLOAT;
  LoadNumber=LoadFloat;
  f=LoadNumber(Z);
  if (f!=tf)
  {
   SwapNumber=SwapFloat;
   SwapNumber(&f);
   if (f!=tf)
    luaL_verror("unknown float representation in binary file %s",Filename);
  }
 }
 else if (sizeofR==sizeof(double))
 {
  double f,tf=TEST_FLOAT;
  LoadNumber=LoadDouble;
  f=LoadNumber(Z);
  if (f!=tf)
  {
   SwapNumber=SwapDouble;
   SwapNumber(&f);
   if (f!=tf)
    luaL_verror("unknown float representation in binary file %s",Filename);
  }
 }
 else
  luaL_verror(
       "floats in binary file %s have %d bytes; "
       "expected %d (float) or %d (double)",
       Filename,sizeofR,sizeof(float),sizeof(double));
}

static void LoadChunk(ZIO* Z)
{
 LoadHeader(Z);
 while (zgetc(Z)==ID_FUNCTION) LoadFunction(Z);
 zungetc(Z);
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
 {
  LoadChunk(Z);
  return Main;
 }
 else if (c!=EOZ)
  luaL_verror("%s not a lua binary file",filename);
 return NULL;
}
