/*
** $Id: $
** lua compiler (saves bytecodes to files; also list binary files)
** Copyrightnotice
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#include "lmem.h"
#include "lparser.h"
#include "lzio.h"
#include "luadebug.h"

void PrintFunction(TFunc* tf, TFunc* Main);
void DumpHeader(FILE* D);
void DumpFunction(TFunc* tf, FILE* D);

static void doit(int undump, char* filename);

static int listing=0;			/* list bytecodes? */
static int dumping=1;			/* dump bytecodes? */
static int undumping=0;			/* undump bytecodes? */
static FILE* D;				/* output file */

static void usage(void)
{
 fprintf(stderr,
 "usage: luac [-c | -u] [-d] [-l] [-p] [-q] [-v] [-o output] file ...\n"
 " -c\tcompile (default)\n"
 " -u\tundump\n"
 " -d\tgenerate debugging information\n"
 " -l\tlist (default for -u)\n"
 " -o\toutput file for -c (default \"luac.out\")\n"
 " -p\tparse only\n"
 " -q\tquiet (default for -c)\n"
 " -v\tshow version information\n"
 );
 exit(1);
}

#define	IS(s)	(strcmp(argv[i],s)==0)

int main(int argc, char* argv[])
{
 char* d="luac.out";			/* default output file */
 int i;
 for (i=1; i<argc; i++)
 {
  if (argv[i][0]!='-')			/* end of options */
   break;
  else if (IS("-"))			/* use stdin */
   break;
  else if (IS("-c"))			/* compile (and dump) */
  {
   dumping=1;
   undumping=0;
  }
  else if (IS("-d"))			/* debug */
   lua_debug=1;
  else if (IS("-l"))			/* list */
   listing=1;
  else if (IS("-o"))			/* output file */
   d=argv[++i];
  else if (IS("-p"))			/* parse only (for timing purposes) */
   dumping=0;
  else if (IS("-q"))			/* quiet */
   listing=0;
  else if (IS("-u"))			/* undump */
  {
   dumping=0;
   undumping=1;
   listing=1;
  }
  else if (IS("-v"))			/* show version */
   printf("%s  %s\n(written by %s)\n\n",LUA_VERSION,LUA_COPYRIGHT,LUA_AUTHORS);
  else					/* unknown option */
   usage();
 }
 --i;					/* fake new argv[0] */
 argc-=i;
 argv+=i;
 if (dumping)
 {
  if (argc<2) usage();
  for (i=1; i<argc; i++)		/* play safe with output file */
   if (IS(d)) luaL_verror("will not overwrite input file \"%s\"\n",d);
  D=fopen(d,"wb");			/* must open in binary mode */
  if (D==NULL)
  {
   fprintf(stderr,"luac: cannot open output file ");
   perror(d);
   exit(1);
  }
  for (i=1; i<argc; i++) doit(0,IS("-")? NULL : argv[i]);
  fclose(D);
 }
 if (undumping)
 {
  if (argc<2)
   doit(1,"luac.out");
  else
   for (i=1; i<argc; i++) doit(1,IS("-")? NULL : argv[i]);
 }
 return 0;
}

static TFunc** F=NULL;
static int nF=0;
static int mF=0;

static void addfunction(TFunc* tf)
{
 if (nF>=mF)
  mF=luaM_growvector(&F,mF,TFunc*,"function table overflow",MAX_WORD);
 F[nF++]=tf;
}

static void findfunctions(TFunc* tf)
{
 int i;
 int n=tf->nconsts;
 nF=0;
 addfunction(tf);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  if (ttype(o)==LUA_T_FUNCTION)
  {
   tfvalue(o)->marked=i;
   addfunction(tfvalue(o));
  }
 }
}

static void do_dump(TFunc* Main)
{
 int i;
 findfunctions(Main);
 if (listing)
 { 
  for (i=0; i<nF; i++) PrintFunction(F[i],Main);
 }
 if (dumping)
 {
  DumpHeader(D);
  for (i=0; i<nF; i++) DumpFunction(F[i],D);
 }
}

static void do_compile(ZIO* z, char* chunkname)
{
 TFunc* tf=luaY_parser(z,chunkname);
 do_dump(tf);
}

static void do_undump(ZIO* z, char* filename)
{
 TFunc* Main;
 while ((Main=luaU_undump1(z,filename)))
 {
  int i;
  findfunctions(Main);
  if (listing) for (i=0; i<nF; i++) PrintFunction(F[i],Main);
 }
}

static void doit(int undump, char* filename)
{
 FILE* f= (filename==NULL) ? stdin : fopen(filename, undump ? "rb" : "r");
 if (f==NULL)
 {
  fprintf(stderr,"luac: cannot open ");
  perror(filename);
  exit(1);
 }
 else
 {
  ZIO z;
  zFopen(&z,f);
  if (filename==NULL) filename="(stdin)";
  if (undump) do_undump(&z,filename); else do_compile(&z,filename);
  if (f!=stdin) fclose(f);
 }
}

/*
* the functions below avoid linking luac with 
* lapi.o lauxlib.o lbuiltin.o ldo.o lgc.o ltable.o ltm.o lvm.o
*/

/* simplified from lbuiltin.c */
void luaB_predefine(void)
{
}

/* simplified from ldo.c */
void lua_error(char* s)
{
 if (s) fprintf(stderr,"luac: %s\n",s);
 exit(1);
}

/* copied from lauxlib.c */
void luaL_verror (char *fmt, ...)
{
  char buff[500];
  va_list argp;
  va_start(argp, fmt);
  vsprintf(buff, fmt, argp);
  va_end(argp);
  lua_error(buff);
}
