/*
** luac.c
** lua compiler (saves bytecodes in files)
*/

char *rcs_luac="$Id: luac.c,v 1.1 1996/02/20 16:39:40 lhf Exp lhf $";

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "lua.h"
#include "tree.h"
#include "inout.h"
#include "func.h"
#include "opcode.h"

void PrintCode(Byte *code, Byte *end);
static void compile(char *filename);

extern jmp_buf *errorJmp;

int main(int argc, char *argv[])
{
 if (argc < 2)
  compile(NULL);  /* executes stdin as a file */
 else
 {
  int i;
  for (i=1; i<argc; i++)
  {
   if (strcmp(argv[i], "-") == 0)
    compile(NULL);  /* executes stdin as a file */
   else if (strcmp(argv[i], "-v") == 0)
    printf("%s  %s\n(written by %s)\n\n",
            LUA_VERSION, LUA_COPYRIGHT, LUA_AUTHORS);
   else
    compile(argv[i]);
  }
 }
 return 0;
}

static void dump(TFunc *tf)
{
  printf("dump: %s:%d %d bytes\n",tf->fileName,tf->lineDefined,tf->size);
  PrintCode(tf->code,tf->code+tf->size);
  ThreadCode(tf->code,tf->code+tf->size);
  PrintCode(tf->code,tf->code+tf->size);
}

static void do_compile(void)
{
  TFunc tf;
  jmp_buf myErrorJmp;
  jmp_buf *oldErr = errorJmp;
  errorJmp = &myErrorJmp;
  luaI_initTFunc(&tf);
  tf.fileName = lua_parsedfile;
  if (setjmp(myErrorJmp) == 0)
  {
    lua_parse(&tf);
    dump(&tf);
  }
  else				/* syntax error */
  {
    exit(1);
  }
  errorJmp = oldErr;
  if (tf.code)
    luaI_free(tf.code);
}

static void compile(char *filename)
{
  if (lua_openfile(filename)) return;
  do_compile();
  lua_closefile();
}
