/*
** $Id: lundump.c,v 1.22 1999/09/09 13:24:52 lhf Exp lhf $
** load bytecodes from files
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <string.h>
#include "lauxlib.h"
#include "lfunc.h"
#include "lmem.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#define	LoadBlock(b,size,Z)	ezread(Z,b,size)

static void unexpectedEOZ (ZIO* Z)
{
 luaL_verror("unexpected end of file in %.255s",zname(Z));
}

static int ezgetc (ZIO* Z)
{
 int c=zgetc(Z);
 if (c==EOZ) unexpectedEOZ(Z);
 return c;
}

static void ezread (ZIO* Z, void* b, int n)
{
 int r=zread(Z,b,n);
 if (r!=0) unexpectedEOZ(Z);
}

static unsigned int LoadWord (ZIO* Z)
{
 unsigned int hi=ezgetc(Z);
 unsigned int lo=ezgetc(Z);
 return (hi<<8)|lo;
}

static unsigned long LoadLong (ZIO* Z)
{
 unsigned long hi=LoadWord(Z);
 unsigned long lo=LoadWord(Z);
 return (hi<<16)|lo;
}

static real LoadNumber (ZIO* Z, int native)
{
 if (native)
 {
  real x;
  LoadBlock(&x,sizeof(x),Z);
  return x;
 }
 else
 {
  char b[256];
  int size=ezgetc(Z);
  LoadBlock(b,size,Z);
  b[size]=0;
  return luaU_str2d(b,zname(Z));
 }
}

static int LoadInt (ZIO* Z, const char* message)
{
 unsigned long l=LoadLong(Z);
 unsigned int i=l;
 if (i!=l) luaL_verror(message,l,zname(Z));
 return i;
}

#define PAD	5			/* two word operands plus opcode */

static Byte* LoadCode (ZIO* Z)
{
 int size=LoadInt(Z,"code too long (%lu bytes) in %.255s");
 Byte* b=luaM_malloc(size+PAD);
 LoadBlock(b,size,Z);
 if (b[size-1]!=ENDCODE) luaL_verror("bad code in %.255s",zname(Z));
 memset(b+size,ENDCODE,PAD);		/* pad code for safety */
 return b;
}

static TaggedString* LoadTString (ZIO* Z)
{
 long size=LoadLong(Z);
 if (size==0)
  return NULL;
 else
 {
  char* s=luaL_openspace(size);
  LoadBlock(s,size,Z);
  return luaS_newlstr(s,size-1);
 }
}

static void LoadLocals (TProtoFunc* tf, ZIO* Z)
{
 int i,n=LoadInt(Z,"too many locals (%lu) in %.255s");
 if (n==0) return;
 tf->locvars=luaM_newvector(n+1,LocVar);
 for (i=0; i<n; i++)
 {
  tf->locvars[i].line=LoadInt(Z,"too many lines (%lu) in %.255s");
  tf->locvars[i].varname=LoadTString(Z);
 }
 tf->locvars[i].line=-1;		/* flag end of vector */
 tf->locvars[i].varname=NULL;
}

static TProtoFunc* LoadFunction (ZIO* Z, int native);

static void LoadConstants (TProtoFunc* tf, ZIO* Z, int native)
{
 int i,n=LoadInt(Z,"too many constants (%lu) in %.255s");
 tf->nconsts=n;
 if (n==0) return;
 tf->consts=luaM_newvector(n,TObject);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  ttype(o)=-ezgetc(Z);			/* ttype(o) is negative - ORDER LUA_T */
  switch (ttype(o))
  {
   case LUA_T_NUMBER:
	nvalue(o)=LoadNumber(Z,native);
	break;
   case LUA_T_STRING:
	tsvalue(o)=LoadTString(Z);
	break;
   case LUA_T_PROTO:
	tfvalue(o)=LoadFunction(Z,native);
	break;
   case LUA_T_NIL:
	break;
   default:				/* cannot happen */
	luaU_badconstant("load",i,o,tf);
	break;
  }
 }
}

static TProtoFunc* LoadFunction (ZIO* Z, int native)
{
 TProtoFunc* tf=luaF_newproto();
 tf->lineDefined=LoadInt(Z,"lineDefined too large (%lu) in %.255s");
 tf->source=LoadTString(Z);
 if (tf->source==NULL) tf->source=luaS_new(zname(Z));
 tf->code=LoadCode(Z);
 LoadLocals(tf,Z);
 LoadConstants(tf,Z,native);
 return tf;
}

static void LoadSignature (ZIO* Z)
{
 const char* s=SIGNATURE;
 while (*s!=0 && ezgetc(Z)==*s)
  ++s;
 if (*s!=0) luaL_verror("bad signature in %.255s",zname(Z));
}

static int LoadHeader (ZIO* Z)
{
 int version,sizeofR,native;
 LoadSignature(Z);
 version=ezgetc(Z);
 if (version>VERSION)
  luaL_verror(
	"%.255s too new: version=0x%02x; expected at most 0x%02x",
	zname(Z),version,VERSION);
 if (version<VERSION0)			/* check last major change */
  luaL_verror(
	"%.255s too old: version=0x%02x; expected at least 0x%02x",
	zname(Z),version,VERSION0);
 sizeofR=ezgetc(Z);
 native=(sizeofR!=0);
 if (native)				/* test number representation */
 {
  if (sizeofR!=sizeof(real))
   luaL_verror("unknown number size in %.255s: read %d; expected %d",
	 zname(Z),sizeofR,(int)sizeof(real));
  else
  {
   real tf=TEST_NUMBER;
   real f=LoadNumber(Z,native);
   if ((long)f!=(long)tf)
    luaL_verror("unknown number format in %.255s: "
	  "read " NUMBER_FMT "; expected " NUMBER_FMT,
	  zname(Z),f,tf);
  }
 }
 return native;
}

static TProtoFunc* LoadChunk (ZIO* Z)
{
 return LoadFunction(Z,LoadHeader(Z));
}

/*
** load one chunk from a file or buffer
** return main if ok and NULL at EOF
*/
TProtoFunc* luaU_undump1 (ZIO* Z)
{
 int c=zgetc(Z);
 if (c==ID_CHUNK)
  return LoadChunk(Z);
 else if (c!=EOZ)
  luaL_verror("%.255s is not a Lua binary file",zname(Z));
 return NULL;
}

/*
* handle constants that cannot happen
*/
void luaU_badconstant (const char* s, int i, const TObject* o, const TProtoFunc* tf)
{
 int t=ttype(o);
 const char* name= (t>0 || t<LUA_T_LINE) ? "?" : luaO_typenames[-t];
 luaL_verror("cannot %s constant #%d: type=%d [%s]" IN,s,i,t,name,INLOC);
}

/*
* convert number from text
*/
double luaU_str2d (const char* b, const char* where)
{
 double x;
 if (!luaO_str2d(b,&x))
  luaL_verror("cannot convert number '%.255s' in %.255s",b,where);
 return x;
}
