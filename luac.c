/*
** $Id: luac.c,v 1.33 2001/11/29 01:00:34 lhf Exp lhf $
** Lua compiler (saves bytecodes to files; also list bytecodes)
** See Copyright Notice in lua.h
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
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
static Proto* load(lua_State* L, const char* filename);
static Proto* combine(lua_State* L, Proto** P, int n);
static void strip(lua_State* L, Proto* f);
static void cannot(const char* name, const char* what, const char* mode);
static void fatal(const char* message);

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
 lua_State* L;
 Proto** P,*f;
 int i=doargs(argc,argv);
 argc-=i; argv+=i;
 if (argc<=0) usage("no input files given",NULL);
 L=lua_open();
#ifdef LUA_DEBUG
#define FREEALL
 luaB_opentests(L);
#endif
 lua_register(L,LUA_ERRORMESSAGE,ERRORMESSAGE);
 P=luaM_newvector(L,argc,Proto*);
 for (i=0; i<argc; i++)
  P[i]=load(L,IS("-")? NULL : argv[i]);
 f=combine(L,P,argc);
 if (listing) luaU_printchunk(f);
 if (dumping)
 {
  FILE* D=fopen(output,"wb");
  if (D==NULL) cannot(output,"open","out");
  if (stripping) strip(L,f);
  luaU_dumpchunk(f,D);
  if (ferror(D)) cannot(output,"write","out");
  fclose(D);
 }
#ifdef FREEALL
 if (argc==1) luaM_freearray(L,P,argc,sizeof(Proto*));
 lua_close(L);
#endif
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

static Proto* load(lua_State* L, const char* filename)
{
 int code=lua_loadfile(L,filename);
 switch (code)
 {
  case 0:
  if (errno!=0) cannot(filename,"read","in");
  {
   const Closure* c=lua_topointer(L,-1);
   return c->l.p;
  }
  case LUA_ERRFILE:
   cannot(filename,"open","in");
   break;
  default:
   fatal(luaL_errstr(code));
   break;
 }
 return NULL;
}

static Proto* combine(lua_State* L, Proto** P, int n)
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

static void strip(lua_State* L, Proto* f)
{
 int i,n=f->sizep;
#ifdef FREEALL
 luaM_freearray(L, f->lineinfo, f->sizelineinfo, int);
 luaM_freearray(L, f->locvars, f->sizelocvars, struct LocVar);
#endif
 f->lineinfo=NULL;
 f->sizelineinfo=0;
 f->source=luaS_new(L,"=(none)");
 f->locvars=NULL;
 f->sizelocvars=0;
 for (i=0; i<n; i++) strip(L,f->p[i]);
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
