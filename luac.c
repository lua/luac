/*
** $Id: luac.c,v 1.35 2002/03/01 01:46:24 lhf Exp lhf $
** Lua compiler (saves bytecodes to files; also list bytecodes)
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#ifdef LUA_DEBUG
#define FREEALL
#else
#define luaB_opentests(L)
#endif

#define PROGNAME	"luac"		/* program name */
#define	OUTPUT		PROGNAME ".out"	/* default output file */

static int listing=0;			/* list bytecodes? */
static int dumping=1;			/* dump bytecodes? */
static int stripping=0;			/* strip debug information? */
static char Output[]={ OUTPUT };	/* default output file name */
static const char* output=Output;	/* output file name */

static void fatal(const char* message)
{
 fprintf(stderr,"%s: %s\n",PROGNAME,message);
 exit(EXIT_FAILURE);
}

static void cannot(const char* name, const char* what, const char* mode)
{
 fprintf(stderr,"%s: cannot %s %sput file ",PROGNAME,what,mode);
 perror(name);
 exit(EXIT_FAILURE);
}

static void usage(const char* message, const char* arg)
{
 if (message!=NULL)
 {
  fprintf(stderr,"%s: ",PROGNAME); fprintf(stderr,message,arg); fprintf(stderr,"\n");
 }
 fprintf(stderr,
 "usage: %s [options] [filenames].  Available options are:\n"
 "  -        process stdin\n"
 "  -l       list\n"
 "  -o file  output file (default is \"" OUTPUT "\")\n"
 "  -p       parse only\n"
 "  -s       strip debug information\n"
 "  -v       show version information\n",
 PROGNAME);
 exit(EXIT_FAILURE);
}

#define	IS(s)	(strcmp(argv[i],s)==0)

static int doargs(int argc, char* argv[])
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
   if (output==NULL || *output==0) usage("`-o' needs argument",NULL);
  }
  else if (IS("-p"))			/* parse only */
   dumping=0;
  else if (IS("-s"))			/* strip debug information */
   stripping=1;
  else if (IS("-v"))			/* show version */
  {
   printf("%s  %s\n",LUA_VERSION,LUA_COPYRIGHT);
   if (argc==2) exit(EXIT_SUCCESS);
  }
  else					/* unknown option */
   usage("unrecognized option `%s'",argv[i]);
 }
 if (i==argc && (listing || !dumping))
 {
  dumping=0;
  argv[--i]=Output;
 }
 return i;
}

static Proto* load(lua_State* L, const char* filename)
{
 if (luaL_loadfile(L,filename)==0)
 {
  const Closure* c=lua_topointer(L,-1);
  return c->l.p;
 }
 else
 {
  fatal(lua_tostring(L,-1));
  return NULL;
 }
}

static Proto* combine(lua_State* L, Proto** P, int n)
{
 if (n==1)
  return P[0];
 else
 {
  int i,pc=0;
  Proto* f=luaF_newproto(L);
  f->source=luaS_newliteral(L,"=(" PROGNAME ")");
  f->maxstacksize=1;
  f->p=P;
  f->sizep=n;
  f->sizecode=2*n+1;
  f->code=luaM_newvector(L,f->sizecode,Instruction);
  for (i=0; i<n; i++)
  {
   f->code[pc++]=CREATE_ABx(OP_CLOSURE,0,i);
   f->code[pc++]=CREATE_ABC(OP_CALL,0,1,1);
  }
  f->code[pc++]=CREATE_ABC(OP_RETURN,0,1,0);
  return f;
 }
}

static void strip(lua_State* L, Proto* f)
{
 int i,n=f->sizep;
#ifdef FREEALL
 luaM_freearray(L, f->lineinfo, f->sizecode, int);
 luaM_freearray(L, f->locvars, f->sizelocvars, struct LocVar);
#endif
 f->lineinfo=NULL;
 f->source=luaS_newliteral(L,"=(none)");
 f->locvars=NULL;
 f->sizelocvars=0;
 for (i=0; i<n; i++) strip(L,f->p[i]);
}

static size_t writer(const void* p, size_t size, size_t n, void* u)
{
 return fwrite(p,size,n,(FILE*)u);
}

int main(int argc, char* argv[])
{
 lua_State* L;
 Proto** P,*f;
 int i=doargs(argc,argv);
 argc-=i; argv+=i;
 if (argc<=0) usage("no input files given",NULL);
 L=lua_open();
 luaB_opentests(L);
 P=luaM_newvector(L,argc,Proto*);
 for (i=0; i<argc; i++)
  P[i]=load(L,IS("-")? NULL : argv[i]);
 f=combine(L,P,argc);
 if (listing) luaU_print(f);
 if (dumping)
 {
  FILE* D=fopen(output,"wb");
  if (D==NULL) cannot(output,"open","out");
  if (stripping) strip(L,f);
  luaU_dump(f,writer,D);
  if (ferror(D)) cannot(output,"write","out");
  fclose(D);
 }
#ifdef FREEALL
 if (argc==1) luaM_freearray(L,P,argc,sizeof(Proto*));
 lua_close(L);
#endif
 return 0;
}
