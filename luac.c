/*
** luac.c
** lua compiler (saves bytecodes to files)
*/

char *rcs_luac="$Id: luac.c,v 1.9 1996/02/26 19:43:44 lhf Exp lhf $";

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "luac.h"

static void compile(char *filename);

static int listing=0;
static int dumping=1;
static FILE *D;				/* output file */

static void usage(void)
{
 fprintf(stderr,"usage: luac [-dlpv] [-o filename] file ...\n");
 exit(0);
}

#define	IS(s)	(strcmp(argv[i],s)==0)

int main(int argc, char *argv[])
{
 char *d="luac.out";			/* default output file */
 int i;
 for (i=1; i<argc; i++)
 {
  if (argv[i][0]!='-')			/* end of options */
   break;
  else if (IS("-"))			/* use stdin */
   break;
  else if (IS("-d"))			/* debug */
   lua_debug=1;
  else if (IS("-l"))			/* list */
   listing=1;
  else if (IS("-o"))			/* output file */
   d=argv[++i];
  else if (IS("-p"))			/* parse only (for timing purposes) */
   dumping=0;
  else if (IS("-v"))			/* show version */
   printf("%s  %s\n(written by %s)\n\n",LUA_VERSION,LUA_COPYRIGHT,LUA_AUTHORS);
  else					/* unknown option */
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
 for (i=1; i<argc; i++) compile(IS("-")? NULL : argv[i]);
 fclose(D);
 return 0;
}

static void dump(TFunc *tf)
{
 if (listing) PrintFunction(tf);
 DumpFunction(tf,D);
 luaI_free(tf->code);
 luaI_free(tf->locvars);
}

static void do_dump(TFunc *tf)		/* only for tf=main */
{
 DumpHeader(D);
 dump(tf);			/* thread main and build function list */
 for (tf=tf->next; tf!=NULL; tf=tf->next)
  dump(tf);
}

static void do_compile(void)
{
 TFunc tf;
 extern jmp_buf *errorJmp;
 jmp_buf E;

 luaI_initTFunc(&tf);
 tf.fileName = lua_parsedfile;
 errorJmp=&E; if (setjmp(E)) exit(1);	/* syntax error */
 lua_parse(&tf);
 if (dumping) do_dump(&tf);
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
