/*
** luau.c
** lua decompiler (loads bytecodes from files)
*/

char* rcs_luau="$Id: luau.c,v 1.1 1996/03/08 21:46:20 lhf Exp lhf $";

#include <stdio.h>
#include <string.h>
#include "luac.h"

static void undump(char* filename);

static int listing=0;			/* list bytecodes? */

static void usage(void)
{
 fprintf(stderr,"usage: luau [-dlv] [-o output] file ...\n");
 exit(0);
}

#define	IS(s)	(strcmp(argv[i],s)==0)

int main(int argc, char* argv[])
{
 char* d="luau.out";			/* default output file */
 int i;
 for (i=1; i<argc; i++)
 {
  if (argv[i][0]!='-')			/* end of options */
   break;
  else if (IS("-"))			/* use stdin */
   break;
  else if (IS("-l"))			/* list */
   listing=1;
  else if (IS("-o"))			/* output file */
   d=argv[++i];
  else if (IS("-v"))			/* show version */
   printf("%s  %s\n(written by %s)\n\n",LUA_VERSION,LUA_COPYRIGHT,LUA_AUTHORS);
  else					/* unknown option */
   usage();
 }
 --i;					/* fake new argv[0] */
 argc-=i;
 argv+=i;
 if (argc<2) usage();
 for (i=1; i<argc; i++) undump(IS("-")? NULL : argv[i]);
 return 0;
}

static void do_undump(FILE* f)
{
 TFunc* m;
 while ((m=luaI_undump1(f)))
 {
  if (listing)
  {
   TFunc* tf;
   for (tf=m; tf!=NULL; tf=tf->next)
    PrintFunction(tf);
   }
   luaI_freefunc(m);			/* TODO: free others */
 }
}

static void undump(char* fn)
{
 FILE* f=fopen(fn,"rb");		/* must open in binary mode */
 if (f==NULL)
 {
  fprintf(stderr,"luau: cannot open ");
  perror(fn);
  exit(1);
 }
 do_undump(f);
 fclose(f);
}
