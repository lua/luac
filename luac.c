/*
** luac.c
** lua compiler (saves bytecodes to files)
*/

char *rcs_luac="$Id: luac.c,v 1.4 1996/02/22 00:35:43 lhf Exp lhf $";

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "lua.h"
#include "tree.h"
#include "func.h"
#include "inout.h"
#include "mem.h"
#include "opcode.h"

void DumpHeader(FILE *D);
void DumpFunction(TFunc *tf, FILE *D);
void PrintFunction(TFunc *tf);

static void compile(char *filename);

static int listing=0;
static FILE *D;

#define	IS(s)	(strcmp(argv[i],s)==0)

static void usage(void)
{
 fprintf(stderr,"usage: luac [-v] [-l] [-o filename] file ...\n");
 exit(0);
}

int main(int argc, char *argv[])
{
 char *d="luac.out";
 int i;
 for (i=1; i<argc; i++)
 {
  if (argv[i][0]!='-')			/* end of options */
   break;
  else if (IS("-"))			/* use stdin */
   break;
  else if (IS("-v"))			/* show version */
   printf("%s  %s\n(written by %s)\n\n",LUA_VERSION,LUA_COPYRIGHT,LUA_AUTHORS);
  else if (IS("-l"))			/* list */
   listing=1;
  else if (IS("-o"))			/* output file */
   d=argv[++i];
  else
   usage();
 }
 --i;					/* fake new argv[0] */
 argc-=i;
 argv+=i;
 if (argc<2) usage();
 D=fopen(d,"wb");			/* must be binary mode */
 if (D==NULL)
 {
  fprintf(stderr,"luac: cannot open ");
  perror(d);
  exit(1);
 }
 DumpHeader(D);
 for (i=1; i<argc; i++) compile(IS("-")? NULL : argv[i]);
 fclose(D);
 return 0;
}

static void dump(TFunc *tf)
{
 if (listing) PrintFunction(tf);
 DumpFunction(tf,D);
 luaI_free(tf->code);
 if (tf->locvars) luaI_free(tf->locvars);
}

static void do_dump(TFunc *tf)
{
 tf->next=NULL;
 dump(tf);
 for (tf=tf->next; tf!=NULL; tf=tf->next)
  dump(tf);
}

static void do_compile(void)
{
 TFunc tf;
 extern jmp_buf *errorJmp;
 jmp_buf myErrorJmp;
 jmp_buf *oldErr = errorJmp;
 errorJmp = &myErrorJmp;
 luaI_initTFunc(&tf);
 tf.fileName = lua_parsedfile;
 if (setjmp(myErrorJmp) == 0)
 {
  lua_parse(&tf);
  tf.marked=0;
  do_dump(&tf);
 }
 else					/* syntax error */
 {
  exit(1);
 }
 errorJmp = oldErr;
}

static void compile(char *filename)
{
 if (lua_openfile(filename))
 {
  fprintf(stderr,"luac: cannot open ");
  perror(filename);
  exit(1);
 }
 do_compile();
 lua_closefile();
}
