/*
** $Id: luac.c,v 1.21 2000/01/28 17:51:09 lhf Exp lhf $
** lua compiler (saves bytecodes to files; also list binary files)
** See Copyright Notice in lua.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lparser.h"
#include "lstate.h"
#include "lzio.h"
#include "luac.h"

#define	OUTPUT	"luac.out"		/* default output file */

static FILE* efopen(const char* name, const char* mode);
static void doit(int undump, const char* filename);
static void defineglobal(const char* name, int define);

static int listing=0;			/* list bytecodes? */
static int debugging=0;			/* emit debug information? */
static int dumping=1;			/* dump bytecodes? */
static int undumping=0;			/* undump bytecodes? */
static int optimizing=0;		/* optimize? */
static int parsing=0;			/* parse only? */
static int testing=0;			/* test integrity? */
static int verbose=0;			/* tell user what is done */
static int native=0;			/* save numbers in native format? */
static FILE* D;				/* output file */

static void usage(const char* message, const char* arg)
{
 if (message!=NULL)
 {
  fprintf(stderr,"luac: "); fprintf(stderr,message,arg); fprintf(stderr,"\n");
 }
 fprintf(stderr,
 "usage: luac [options] [filenames].  Available options are:\n"
 "  -        compile stdin\n"
 "  -c       compile (default)\n"
 "  -d       generate debugging information\n"
 "  -D name  predefine `name' for conditional compilation\n"
 "  -l       list (default for -u)\n"
 "  -n       save numbers in native format (file may not be portable)\n"
 "  -o file  output file for -c (default is \"" OUTPUT "\")\n"
 "  -O       optimize\n"
 "  -p       parse only\n"
 "  -q       quiet (default for -c)\n"
 "  -t       test code integrity\n"
 "  -u       undump\n"
 "  -U name  undefine `name' for conditional compilation\n"
 "  -v       show version information\n"
 "  -V       verbose\n"
 );
 exit(1);
}

static void argument(const char* op)
{
 usage("missing argument to `%s'",op);
}

#define	IS(s)	(strcmp(argv[i],s)==0)

int main(int argc, char* argv[])
{
 const char* d=OUTPUT;			/* output file name */
 int i;
 L=lua_newstate(NULL);
 for (i=1; i<argc; i++)
 {
  if (*argv[i]!='-')			/* end of options */
   break;
  else if (IS("-"))			/* end of options; use stdin */
   break;
  else if (IS("-c"))			/* compile (and dump) */
  {
   dumping=1;
   undumping=0;
  }
  else if (IS("-D"))			/* $define */
  {
   if (++i>=argc) argument(argv[i-1]);
   defineglobal(argv[i],1);
  }
  else if (IS("-d"))			/* debug */
   debugging=1;
  else if (IS("-l"))			/* list */
   listing=1;
  else if (IS("-n"))			/* native */
   native=1;
  else if (IS("-o"))			/* output file */
  {
   if (++i>=argc) argument(argv[i-1]);
   d=argv[i];
  }
  else if (IS("-O"))			/* optimize */
   optimizing=1; 
  else if (IS("-p"))			/* parse only */
  {
   dumping=0;
   parsing=1;
  }
  else if (IS("-q"))			/* quiet */
   listing=0;
  else if (IS("-t"))			/* test */
  {
   testing=1;
   if (argc==2) { dumping=0; undumping=1; }
  }
  else if (IS("-u"))			/* undump */
  {
   dumping=0;
   undumping=1;
   listing=1;
  }
  else if (IS("-U"))			/* undefine */
  {
   if (++i>=argc) argument(argv[i-1]);
   defineglobal(argv[i],0);
  }
  else if (IS("-v"))			/* show version */
  {
   printf("%s  %s\n", LUA_VERSION,LUA_COPYRIGHT);
   if (argc==2) return 0;
  }
  else if (IS("-V"))			/* verbose */
   verbose=1;
  else					/* unknown option */
   usage("unrecognized option `%s'",argv[i]);
 }
 --i;					/* fake new argv[0] */
 argc-=i;
 argv+=i;
 if (dumping || parsing)
 {
  if (argc<2) usage("no input files given",NULL);
  if (dumping)
  {
   for (i=1; i<argc; i++)		/* play safe with output file */
    if (IS(d)) luaL_verror(L,"will not overwrite input file \"%s\"",d);
   D=efopen(d,"wb");			/* must open in binary mode */
  }
  for (i=1; i<argc; i++) doit(0,IS("-")? NULL : argv[i]);
  if (dumping) fclose(D);
 }
 if (undumping)
 {
  if (argc<2)
   doit(1,OUTPUT);
  else
   for (i=1; i<argc; i++) doit(1,IS("-")? NULL : argv[i]);
 }
 return 0;
}

static void do_compile(ZIO* z)
{
 Proto* Main;
 if (optimizing) L->debug=0;
 if (debugging)  L->debug=1;
 Main=luaY_parser(L,z);
 if (optimizing) luaU_optchunk(Main);
 if (listing) luaU_printchunk(Main);
 if (testing) luaU_testchunk(Main);
 if (dumping) luaU_dumpchunk(Main,D,native);
}

static void do_undump(ZIO* z)
{
 for (;;)
 {
  Proto* Main=luaU_undump1(L,z);
  if (Main==NULL) break;
  if (optimizing) luaU_optchunk(Main);
  if (listing) luaU_printchunk(Main);
  if (testing) luaU_testchunk(Main);
 }
}

static void doit(int undump, const char* filename)
{
 FILE* f= (filename==NULL) ? stdin : efopen(filename, undump ? "rb" : "r");
 ZIO z;
 char source[255+2];			/* +2 for '@' and '\0' */
 luaL_filesource(source,filename,sizeof(source));
 luaZ_Fopen(&z,f,source);
 if (verbose) fprintf(stderr,"%s\n",source+1);
 if (undump) do_undump(&z); else do_compile(&z);
 if (f!=stdin) fclose(f);
}

static void defineglobal(const char* name, int define)
{
 GlobalVar* s=luaS_assertglobalbyname(L,name);
 if (define)
 {
  s->value.ttype=TAG_NUMBER;
  s->value.value.n=1;
 }
 else
  s->value.ttype=TAG_NIL;
}

static FILE* efopen(const char* name, const char* mode)
{
 FILE* f=fopen(name,mode);
 if (f==NULL)
 {
  fprintf(stderr,"luac: cannot open %sput file ",*mode=='r' ? "in" : "out");
  perror(name);
  exit(1);
 }
 return f; 
}

void luaU_optchunk(Proto* Main)
{
 UNUSED(Main);
 fprintf(stderr,"luac: -O not operational in this alpha version\n");
 exit(1);
}

void luaU_testchunk(const Proto* Main)
{
 UNUSED(Main);
 fprintf(stderr,"luac: -t not operational in this alpha version\n");
 exit(1);
}
