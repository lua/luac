/*
** luacc.c
** lua compiler to C string
*/

char* rcs_luacc="$Id$";

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void compile(char* filename);
static void do_compile(FILE* f);

int main(int argc, char* argv[])
{
 int i;
 for (i=1; i<argc; i++) compile(argv[i]);
 printf("void luacc(void)\n{\n");
 for (i=1; i<argc; i++) printf(" lua_dostring(%s);\n",argv[i]);
 printf("}\n");
 return 0;
}

static void compile(char* filename)
{
 FILE* f=fopen(filename,"r");
 if (f==NULL)
 {
  fprintf(stderr,"luacc: cannot open ");
  perror(filename);
  exit(1);
 }
 {
  char* s=filename;
  printf("/* %s */\n",filename);
  for (s=filename; *s!=0; s++) if (!isalnum(*s)) *s='_';
  printf("static char %s[]={\n",filename);
  do_compile(f);
  printf("};\n\n",filename);
 }
 fclose(f);
}

static void do_compile(FILE* f)
{
 putchar('"');
 while (1)
 {
  int c=getc(f);  
  switch (c)
  {
   case EOF:
    putchar('"');
    putchar('\n');
    return;
   case '\n':
    printf("\\n\"\n\"");
    break;
   case '"':
   case '\\':
   case '\'':
    putchar('\\');
   default:
    putchar(c);
    break;
  }
 }
}
