/*
** $Id;$
** load luac listings
** See Copyright Notice in lua.h
*/

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define LUA_CORE

#include "lua.h"
#include "lauxlib.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

typedef struct {
 lua_State* L;
 FILE* f;
 const char* name;
 char line[1024];
 char* word;
 char* head;
 int lineno;
 Proto* main;
} LoadState;

static void error (LoadState* S, const char* msg)
{
 luaL_error(S->L,"%s:%d: %s",S->name,S->lineno,msg);
}

static int unquote(LoadState* S)
{
 char *s=S->word;
 char *f,*t;
 int q=*s;
 for (f=s+1,t=s; *f!=q; f++,t++)
 {
  if (*f!='\\')
   *t=*f;
  else
   switch (*++f)
   {
    case 'a':	*t='\a'; break;
    case 'b':	*t='\b'; break;
    case 'f':	*t='\f'; break;
    case 'n':	*t='\n'; break;
    case 'r':	*t='\r'; break;
    case 't':	*t='\t'; break;
    case 'v':	*t='\v'; break;
    default:
    	if (!isdigit(*f))
		*t=*f;
	else
	{
		int c,n;
		sscanf(f,"%3d%n",&c,&n);
		*t=c;
		f+=n-1;
	}
	break;
   }
  }
 *t='\0';
 return t-s;
}

static int NextLine(LoadState* S)
{
 for (;;)
 {
  char* s=fgets(S->line,sizeof(S->line),S->f);
  if (s==NULL) return 0;
  S->lineno++;
  while (isspace(*s)) s++;
  S->head=s;
  if (*s!=0 && *s!=';') return 1;
 }
}

static void LoadLine(LoadState* S)
{
 if (!NextLine(S)) error(S,"unexpected end of file");
}

static char* LoadWord(LoadState* S)
{
 char* s=S->head;
 while (isspace(*s)) s++;
 if (*s==0) error(S,"unexpected end of file");
 S->word=s;
 while (!isspace(*s) && *s!=0) s++;
 if (*s==0) S->head=s; else { *s=0; S->head=s+1; }
 return S->word;
}

static int LoadInt(LoadState* S)
{
 const char* s=LoadWord(S);
 int i=0;
 if (sscanf(s,"%d",&i) || sscanf(s,"(%d",&i) || sscanf(s,"[%d",&i)) return i;
 if (sscanf(s,"#%d",&i)) return i+MAXSTACK;
 error(S,"bad integer");
 return 0;
}

static Proto* LoadAddress(LoadState* S)
{
 void* p;
 const char* s=LoadWord(S);
 if (sscanf(s,"%p",&p)!=1) error(S,"bad address");
 return p;
}

static Instruction LoadOp (LoadState* S, Proto* f)
{
 const char* s=LoadWord(S);
 int i;
 for (i=0; i<NUM_OPCODES; i++)
 {
  if (strcmp(luaP_opnames[i],s)==0)
  {
   Instruction I=0;
   OpCode o=i;
   int a=0;
   int b=0;
   int c=0;
   int bx=0;
   int sbx=0;
   switch (getOpMode(o))
   {
    case iABC:
     a=LoadInt(S);
     if (getBMode(o)!=OpArgN) b=LoadInt(S);
     if (getCMode(o)!=OpArgN) c=LoadInt(S);
     I=CREATE_ABC(o,a,b,c);
     break;
    case iABx:
     a=LoadInt(S);
     bx=LoadInt(S);
     if (getBMode(o)==OpArgK && bx>=MAXSTACK) bx-=MAXSTACK;
     I=CREATE_ABx(o,a,bx);
     break;
    case iAsBx:
     if (o!=OP_JMP) a=LoadInt(S);
     sbx=LoadInt(S);
     if (o==OP_JMP) sbx+=MAXARG_sBx;
     I=CREATE_ABx(o,a,sbx);
     break;
   }
   if (o==OP_CLOSURE)
   {
    LoadWord(S);
    f->p[bx]=LoadAddress(S);
   }
   return I;
  }
 }
 error(S,"bad instruction");
 return 0;
}

static void LoadCode (LoadState* S, Proto* f)
{
 int i,n=f->sizecode;
 f->code=luaM_newvector(S->L,n,Instruction);
 f->lineinfo=luaM_newvector(S->L,n,int);
 f->sizelineinfo=n;
 for (i=0; i<n; i++)
 {
  int *l=f->lineinfo+i;
  LoadLine(S);
  LoadWord(S);
  LoadWord(S);
  *l=0; sscanf(S->word,"[%d]",l);
  f->code[i]=LoadOp(S,f);
 }
}

static TString* LoadString (LoadState* S)
{
 const char* s=LoadWord(S);
 return luaS_newlstr(S->L,s,S->head-S->word);
}

static void LoadConstants (LoadState* S, Proto* f)
{
 int i,n;
 n=f->sizek;
 f->k=luaM_newvector(S->L,n,TValue);
 LoadLine(S);
 for (i=0; i<n; i++)
 {
  TValue* o=&f->k[i];
  char* s;
  LoadLine(S);
  LoadWord(S);
  s=LoadWord(S);
  switch (s[0])
  {
   lua_Number x;
   int n;
   default:
   	x=0;
	if (sscanf(s,LUA_NUMBER_SCAN,&x)!=1) error(S,"bad constant");
	setnvalue(o,x);
	break;
   case '\'': case '"':
   	n=unquote(S);
	setsvalue2n(S->L,o,luaS_newlstr(S->L,S->word,n));
	break;
   case 'n':
   	setnilvalue(o);
	break;
  }
 }
}

static void LoadLocals (LoadState* S, Proto* f)
{
 int i,n;
 n=f->sizelocvars;
 f->locvars=luaM_newvector(S->L,n,LocVar);
 LoadLine(S);
 for (i=0; i<n; i++)
 {
  LoadLine(S);
  LoadWord(S);
  f->locvars[i].varname=LoadString(S);
  f->locvars[i].startpc=LoadInt(S)-1;
  f->locvars[i].endpc=LoadInt(S)-1;
 }
}

static void LoadUpvalues (LoadState* S, Proto* f)
{
 int i,n;
 LoadLine(S);
 LoadWord(S);
 n=LoadInt(S);
 if (n!=0 && n!=f->nups) error(S,"bad nupvalues");
 f->upvalues=luaM_newvector(S->L,n,TString*);
 f->sizeupvalues=n;
 for (i=0; i<n; i++)
 {
  LoadLine(S);
  LoadWord(S);
  f->upvalues[i]=LoadString(S);
 }
}

static Proto* LoadFunction (LoadState* S)
{
 Proto* f=luaF_newproto(S->L);
 setptvalue2s(S->L,S->L->top,f); incr_top(S->L);
 LoadWord(S);	if (S->word[0]=='m' && S->main==NULL) S->main=f;
 f->source=LoadString(S);
 f->lineDefined=0;
 f->sizecode=LoadInt(S);
 LoadWord(S);
 LoadWord(S);
 LoadWord(S);
 LoadWord(S);
 printf("%p -> %p\n",f,LoadAddress(S));
 LoadLine(S);
 f->numparams=LoadInt(S);
 f->is_vararg=(S->head[-2]=='+');
				LoadWord(S);
 f->maxstacksize=LoadInt(S);	LoadWord(S);
 f->nups=LoadInt(S);		LoadWord(S);
 f->sizelocvars=LoadInt(S);	LoadWord(S);
 f->sizek=LoadInt(S);		LoadWord(S);
 f->sizep=LoadInt(S);
 f->p=luaM_newvector(S->L,f->sizep,Proto*);
 LoadCode(S,f);
 LoadConstants(S,f);
 LoadLocals(S,f);
 LoadUpvalues(S,f);
 S->L->top--;
 return f;
}

static Proto* LoadChunk (LoadState* S)
{
 Proto* f=NULL;
 while (NextLine(S))
  f=LoadFunction(S);
 return S->main;
}

/*
** load luac listing
*/
static Proto* luaU_unprint (lua_State* L, FILE* f, const char* name)
{
 LoadState S;
 S.L=L;
 S.f=f;
 S.lineno=0;
 S.name=name;
 S.main=NULL;
 return LoadChunk(&S);
}

void unprint(lua_State* L, const char* name)
{
 FILE* f;
 Closure *cl;
 if (name==NULL)
 {
  f=stdin;
  name="stdin";
 }
 else
 {
  f=fopen(name,"r");
  if (f==NULL) luaL_error(L,"cannot open %s: %s",name,strerror(errno));
 }
 cl = luaF_newLclosure(L,0,gt(L));
 cl->l.p = luaU_unprint(L,f,name);
 if (ferror(f)) luaL_error(L,"cannot read %s: %s",name,strerror(errno));
 fclose(f);
 setclvalue(L, L->top, cl);
 incr_top(L);
fflush(stdout);
}
