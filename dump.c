/*
** $Id: $
** save bytecodes to file
** Copyrightnotice
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"

/*
** VarLoc(i) is the place of the last occurence of global #i.
** It must be an integer to support long codes.
** Using the "marked" field of the variable name is ok for luac,
** but it is NOT SAFE if dump.c is used during runtime.
*/

#define VarLoc(i)	(luaG_global[i].varname->marked)

static int SawVar(int i, int at)
{
 int old=VarLoc(i);
 int d=at-old;
 VarLoc(i)=at;
 if (old==0) return 0;
 if (NotWord(d))
  luaL_verror("occurences of global #%d (%s) too far apart (%d-%d=%d)\n",
	i,VarStr(i),at,old,d);
 return d;
}

static int Thread(TFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 int i;
 for (i=0; i<luaG_nglobal; i++) VarLoc(i)=0;
 while (1)
 {
	int op=*p;
	int at=p-code+1;
	switch (op)
	{
	case ENDCODE:	return at;
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
	case PUSHLOCAL8:
	case PUSHLOCAL9:
	case PUSHINDEXED:
	case SETLOCAL0:
	case SETLOCAL1:
	case SETLOCAL2:
	case SETLOCAL3:
	case SETLOCAL4:
	case SETLOCAL5:
	case SETLOCAL6:
	case SETLOCAL7:
	case SETLOCAL8:
	case SETLOCAL9:
	case SETINDEXED0:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
	case MINUSOP:
	case NOTOP:
	case RETCODE0:
		p++;
		break;
	case PUSHNILS:
	case PUSHBYTE:
	case PUSHCONSTANTB:
	case PUSHLOCAL:
	case SETLOCAL:
	case SETINDEXED:
	case SETLIST0:
	case SETMAP:
	case RETCODE:
	case ADJUST:
	case POPS:
	case VARARGS:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHCONSTANT:
	case PUSHSELF:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	case SETLIST:
	case CALLFUNC:
		p+=3;
		break;
	case PUSHGLOBAL:
	case SETGLOBAL:
	{
		Word w;
		p++;
		get_word(w,p);
		w=SawVar(w,at);
		p[-2]=w; p[-1]=w>>8;
		break;
	}
	default:			/* cannot happen */
		luaL_verror("bad opcode %d at %d\n",*p,(int)(p-code));
		break;
	}
 }
}

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

static void DumpGlobals(int longcode, TFunc* tf, FILE* D)
{
 int i,n=0;
 for (i=0; i<luaG_nglobal; i++) if (VarLoc(i)!=0) ++n;
 DumpWord(n,D);
 for (i=0; i<luaG_nglobal; i++)
 {
  if (VarLoc(i)!=0)
  {
   if (longcode) DumpLong(VarLoc(i),D); else DumpWord(VarLoc(i),D);
   DumpString(VarStr(i),D);
  }
 }
}

static void DumpLocals(TFunc* tf, FILE* D)
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

static void DumpConstants(TFunc* tf, FILE* D)
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
   case LUA_T_FUNCTION:
	fputc(ID_FUN,D);
	break;
   default:				/* cannot happen */
	luaL_verror("bad constant #%d type=%d\n",i,ttype(o));
	break;
  }
 }
}

void DumpFunction(TFunc* tf, FILE* D)
{
 int size=Thread(tf);
 fputc(ID_FUNCTION,D);
 DumpSize(size,D);
 DumpWord(tf->lineDefined,D);
 if (IsMain(tf))
  DumpTString(tf->fileName,D);
 else
  DumpWord(tf->marked,D);
 DumpBlock(tf->code,size,D);
 DumpGlobals(NotWord(size),tf,D);
 DumpConstants(tf,D);
 DumpLocals(tf,D);
}

void DumpHeader(FILE* D)
{
 real r=TEST_FLOAT;
 fputc(ID_CHUNK,D);
 fputs(SIGNATURE,D);
 fputc(VERSION,D);
 fputc(sizeof(r),D);
 fwrite(&r,sizeof(r),1,D);
}
