/*
** $Id: luac.c,v 1.30 2001/06/28 13:55:17 lhf Exp lhf $
** Lua compiler (saves bytecodes to files; also list bytecodes)
** See Copyright Notice in lua.h
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luac.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#define	OUTPUT	"luac.out"		/* default output file */

static void usage(const char* message, const char* arg);
static int doargs(int argc, const char* argv[]);
static Proto* load(const char* filename);
static Proto* combine(Proto** P, int n);
static void strip(Proto* f);
static FILE* efopen(const char* name, const char* mode);
static void cannot(const char* name, const char* what, const char* mode);
static void fatal(const char* message);

static lua_State* L=NULL;
static int listing=0;			/* list bytecodes? */
static int dumping=1;			/* dump bytecodes? */
static int stripping=0;			/* strip debug information? */
static const char* output=OUTPUT;	/* output file name */

/* need this to handle errors during parse or undump */
static int ERRORMESSAGE(lua_State *l)
{
 fatal(lua_tostring(l,1));
 return 0;
}

#define	IS(s)	(strcmp(argv[i],s)==0)

int main(int argc, const char* argv[])
{
 Proto** P,*f;
 int i=doargs(argc,argv);
 argc-=i; argv+=i;
 if (argc<=0) usage("no input files given",NULL);
 L=lua_open(0);
 lua_register(L,l_s(LUA_ERRORMESSAGE),ERRORMESSAGE);
 P=luaM_newvector(L,argc,Proto*);
 for (i=0; i<argc; i++)
  P[i]=load(IS("-")? NULL : argv[i]);
 f=combine(P,argc);
 if (dumping) luaU_optchunk(L,f);
 if (listing) luaU_printchunk(f);
 if (dumping)
 {
  FILE* D;
  if (stripping) strip(f);
  D=efopen(output,"wb");
  luaU_dumpchunk(f,D);
  if (ferror(D)) cannot(output,"write","out");
  fclose(D);
 }
 return 0;
}

static void usage(const char* message, const char* arg)
{
 if (message!=NULL)
 {
  fprintf(stderr,"luac: "); fprintf(stderr,message,arg); fprintf(stderr,"\n");
 }
 fprintf(stderr,
 "usage: luac [options] [filenames].  Available options are:\n"
 "  -        process stdin\n"
 "  -l       list\n"
 "  -o file  output file (default is \"" OUTPUT "\")\n"
 "  -p       parse only\n"
 "  -s       strip debug information\n"
 "  -v       show version information\n"
 );
 exit(1);
}

static int doargs(int argc, const char* argv[])
{
 int i;
 for (i=1; i<argc; i++)
 {
  if (*argv[i]!='-')			/* end of options */
   break;
  else if (IS("-"))			/* end of options; use stdin */
   return i;
  else if (IS("-l"))			/* list */
   listing=1;
  else if (IS("-o"))			/* output file */
  {
   output=argv[++i];
   if (output==NULL) usage(NULL,NULL);
  }
  else if (IS("-p"))			/* parse only */
   dumping=0;
  else if (IS("-s"))			/* strip debug information */
   stripping=1;
  else if (IS("-v"))			/* show version */
  {
   printf("%s  %s\n",LUA_VERSION,LUA_COPYRIGHT);
   if (argc==2) exit(0);
  }
  else					/* unknown option */
   usage("unrecognized option `%s'",argv[i]);
 }
 if (i==argc && (listing || !dumping))
 {
  dumping=0;
  argv[--i]=OUTPUT;
 }
 return i;
}

static Proto* load(const char* filename)
{
 switch (lua_loadfile(L,filename))
 {
  case 0:
  {
   const Closure* c=lua_topointer(L,-1);
   if (errno!=0) cannot(filename,"read","in");
   return c->f.l;
   break;
  }
  case LUA_ERRFILE:
   cannot(filename,"open","in");
   break;
  case LUA_ERRSYNTAX:
   fatal("syntax error");
   break;
  case LUA_ERRRUN:
   fatal("run-time error");
   break;
  case LUA_ERRMEM:
   fatal("not enough memory");
   break;
  case LUA_ERRERR:
   fatal("error in error handling");
   break;
  default:
   fatal("unknown status returned by lua_loadfile");
   break;
 }
 return NULL;
}

static Proto* combine(Proto** P, int n)
{
 if (n==1)
  return P[0];
 else
 {
  int i,pc=0;
  Proto* f=luaF_newproto(L);
  f->source=luaS_new(L,"=(luac)");
  f->maxstacksize=1;
  f->p=P;
  f->sizep=n;
  f->sizecode=2*n+1;
  f->code=luaM_newvector(L,f->sizecode,Instruction);
  for (i=0; i<n; i++)
  {
   f->code[pc++]=CREATE_ABc(OP_CLOSURE,0,i);
   f->code[pc++]=CREATE_ABC(OP_CALL,0,0,0);
  }
  f->code[pc++]=CREATE_ABC(OP_RETURN,0,0,0);
  return f;
 }
}

static void strip(Proto* f)
{
 int i,n=f->sizep;
 f->lineinfo=NULL;
 f->sizelineinfo=0;
 f->source=luaS_new(L,"=(none)");
 f->locvars=NULL;
 f->sizelocvars=0;
 for (i=0; i<n; i++) strip(f->p[i]);
}

static FILE* efopen(const char* name, const char* mode)
{
 FILE* f=fopen(name,mode);
 if (f==NULL) cannot(name,"open",*mode=='r' ? "in" : "out");
 return f;
}

static void cannot(const char* name, const char* what, const char* mode)
{
 fprintf(stderr,"luac: cannot %s %sput file ",what,mode);
 perror(name);
 exit(1);
}

static void fatal(const char* message)
{
 fprintf(stderr,"luac: %s\n",message);
 exit(1);
}

/*
* the code below avoids the parsing modules (lcode, llex, lparser).
* it is useful if you only want to load binary files.
* this works for interpreters like lua.c too.
*/

#ifdef NOPARSER

#include "llex.h"
#include "lparser.h"
#include "lzio.h"

void luaX_init(lua_State *L) {
  UNUSED(L);
}

Proto *luaY_parser(lua_State *L, ZIO *z) {
  UNUSED(z);
  lua_error(L,"parser not loaded");
  return NULL;
}

#endif
