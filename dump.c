/*
** dump.c
** print,thread,and saves bytecodes to files
*/

#include <stdio.h>
#include <string.h>
#include "opcode.h"
#include "table.h"
#include "tree.h"
#include "types.h"
#include "dump.h"

static char* GlobalVar(int i)
{
 return lua_table[i].varname->str;
}

static char* GlobalStr(int i)
{
 return lua_constant[i]->str;
}

static void PrintCode(Byte *code, Byte *end)
{
 Byte *p;
 for (p=code; p!=end;)
 {
	OpCode op=(OpCode)*p;
	if (op>SETLINE) op=SETLINE+1;
	printf("%8d   %s",p-code,OpCodeName[op]);
	switch (op)
	{
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
	case PUSHLOCAL8:
	case PUSHLOCAL9:
	case PUSHINDEXED:
	case STORELOCAL0:
	case STORELOCAL1:
	case STORELOCAL2:
	case STORELOCAL3:
	case STORELOCAL4:
	case STORELOCAL5:
	case STORELOCAL6:
	case STORELOCAL7:
	case STORELOCAL8:
	case STORELOCAL9:
	case STOREINDEXED0:
	case ADJUST0:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
	case MINUSOP:
	case NOTOP:
	case POP:
	case RETCODE0:
		p++;
		break;
	case PUSHBYTE:
	case PUSHLOCAL:
	case STORELOCAL:
	case STOREINDEXED:
	case STORELIST0:
	case ADJUST:
	case RETCODE:
		printf("\t%d",*(p+1));
		p+=2;
		break;
	case PUSHWORD:
	case PUSHSELF:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		printf("\t%d",c.w);
		break;
	}
	case PUSHFLOAT:
	{
		CodeFloat c;
		p++;
		get_float(c,p);
		printf("\t%g",c.f);
		break;
	}
	case PUSHSTRING:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		printf("\t%d\t; \"%s\"",c.w,GlobalStr(c.w));
		break;
	}
	case PUSHFUNCTION:
	{
		CodeCode c;
		p++;
		get_code(c,p);
		printf("\t%p",c.tf);
		break;
	}
	case PUSHGLOBAL:
	case STOREGLOBAL:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		printf("\t%d\t; %s",c.w,GlobalVar(c.w));
		break;
	}
	case STORELIST:
	case CALLFUNC:
		printf("\t%d %d",*(p+1),*(p+2));
		p+=3;
		break;
	case STORERECORD:
	{
		int n=*++p;
		printf("\t%d",n);
		p++;
		while (n--)
		{
			CodeWord c;
			printf("\n%8d\tFIELD",p-code);
			get_word(c,p);
			printf("\t%d\t; \"%s\"",c.w,GlobalStr(c.w));
		}
		break;
	}
	default:
		printf("\tcannot happen:  opcode=%d",*p);
		p++;
		break;
	}
	printf("\n");
 }
}

#define marked varindex

static int SawVar(int i, int at)
{
 int old=lua_table[i].varname->marked;
 lua_table[i].varname->marked=at;
 return old;
}

#define marked constindex

static int SawStr(int i, int at)
{
 int old=lua_constant[i]->marked;
 lua_constant[i]->marked=at;
 return old;
}

static void CheckThread(Byte *p, int i)
{
 while (i>0)
 {
  CodeWord c;
  Byte *q;
  q=p+i;
  printf(" %d",i);
  get_word(c,q);
  i=c.w;
 }
 printf("\n");
}

static void ThreadCode(Byte *code, Byte *end)
{
 Byte *p;
 int i;
 extern Word lua_ntable;
 extern Word lua_nconstant;
#define marked varindex
 for (i=0; i<lua_ntable; i++) lua_table[i].varname->marked=0;
#define marked constindex
 for (i=0; i<lua_nconstant; i++) lua_constant[i]->marked=0;
#undef marked
 for (p=code; p!=end;)
 {
	OpCode op=(OpCode)*p;
	int at=p-code+1;
	switch (op)
	{
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
	case PUSHLOCAL8:
	case PUSHLOCAL9:
	case PUSHINDEXED:
	case STORELOCAL0:
	case STORELOCAL1:
	case STORELOCAL2:
	case STORELOCAL3:
	case STORELOCAL4:
	case STORELOCAL5:
	case STORELOCAL6:
	case STORELOCAL7:
	case STORELOCAL8:
	case STORELOCAL9:
	case STOREINDEXED0:
	case ADJUST0:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
	case MINUSOP:
	case NOTOP:
	case POP:
	case RETCODE0:
		p++;
		break;
	case PUSHBYTE:
	case PUSHLOCAL:
	case STORELOCAL:
	case STOREINDEXED:
	case STORELIST0:
	case ADJUST:
	case RETCODE:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHSELF:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	case STORELIST:
	case CALLFUNC:
		p+=3;
		break;
	case PUSHFLOAT:
		p+=5;
		break;
	case PUSHSTRING:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		c.w=SawStr(c.w,at);
		p[-2]=c.m.c1;
		p[-1]=c.m.c2;
		break;
	}
	case PUSHFUNCTION:
	{
		CodeCode c;
		p++;
		get_code(c,p);
		c.tf->marked=at;
		break;
	}
	case PUSHGLOBAL:
	case STOREGLOBAL:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		c.w=SawVar(c.w,at);
		p[-2]=c.m.c1;
		p[-1]=c.m.c2;
		break;
	}
	case STORERECORD:
	{
		int n=*++p;
		p++;
		while (n--)
		{
			CodeWord c;
			at=p-code;
			get_word(c,p);
			c.w=SawStr(c.w,at);
			p[-2]=c.m.c1;
			p[-1]=c.m.c2;
		}
		break;
	}
	default:
		printf("\tcannot happen:  opcode=%d",*p);
		p++;
		break;
	}
 }
}

static void ThreadVar(Byte *code, Byte *end)
{
 Byte *p;
 int i;
 extern Word lua_ntable;
 extern Word lua_nconstant;
#define marked varindex
 for (i=0; i<lua_ntable; i++) lua_table[i].varname->marked=0;
#define marked constindex
 for (i=0; i<lua_nconstant; i++) lua_constant[i]->marked=0;
#undef marked
 for (p=code; p!=end;)
 {
	OpCode op=(OpCode)*p;
	int at=p-code+1;
	switch (op)
	{
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
	case PUSHLOCAL8:
	case PUSHLOCAL9:
	case PUSHINDEXED:
	case STORELOCAL0:
	case STORELOCAL1:
	case STORELOCAL2:
	case STORELOCAL3:
	case STORELOCAL4:
	case STORELOCAL5:
	case STORELOCAL6:
	case STORELOCAL7:
	case STORELOCAL8:
	case STORELOCAL9:
	case STOREINDEXED0:
	case ADJUST0:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
	case MINUSOP:
	case NOTOP:
	case POP:
	case RETCODE0:
		p++;
		break;
	case PUSHBYTE:
	case PUSHLOCAL:
	case STORELOCAL:
	case STOREINDEXED:
	case STORELIST0:
	case ADJUST:
	case RETCODE:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHSELF:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	case STORELIST:
	case CALLFUNC:
	case PUSHSTRING:
		p+=3;
		break;
	case PUSHFLOAT:
		p+=5;
		break;
	case PUSHFUNCTION:
	{
		CodeCode c;
		p++;
		get_code(c,p);
		c.tf->marked=at;
		break;
	}
	case PUSHGLOBAL:
	case STOREGLOBAL:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		c.w=SawVar(c.w,at);
		p[-2]=c.m.c1;
		p[-1]=c.m.c2;
		break;
	}
	case STORERECORD:
		p += *(p+1)*sizeof(Word) + 2;
		break;
	default:
		printf("\tcannot happen:  opcode=%d",*p);
		p++;
		break;
	}
 }
}

static void ThreadStr(Byte *code, Byte *end)
{
 Byte *p;
 int i;
 extern Word lua_ntable;
 extern Word lua_nconstant;
 for (i=0; i<lua_ntable; i++) lua_table[i].varname->marked=0;
 for (i=0; i<lua_nconstant; i++) lua_constant[i]->marked=0;
 for (p=code; p!=end;)
 {
	OpCode op=(OpCode)*p;
	int at=p-code+1;
	switch (op)
	{
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
	case PUSHLOCAL8:
	case PUSHLOCAL9:
	case PUSHINDEXED:
	case STORELOCAL0:
	case STORELOCAL1:
	case STORELOCAL2:
	case STORELOCAL3:
	case STORELOCAL4:
	case STORELOCAL5:
	case STORELOCAL6:
	case STORELOCAL7:
	case STORELOCAL8:
	case STORELOCAL9:
	case STOREINDEXED0:
	case ADJUST0:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
	case MINUSOP:
	case NOTOP:
	case POP:
	case RETCODE0:
		p++;
		break;
	case PUSHBYTE:
	case PUSHLOCAL:
	case STORELOCAL:
	case STOREINDEXED:
	case STORELIST0:
	case ADJUST:
	case RETCODE:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHSELF:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	case STORELIST:
	case CALLFUNC:
	case PUSHGLOBAL:
	case STOREGLOBAL:
		p+=3;
		break;
	case PUSHFLOAT:
		p+=5;
		break;
	case PUSHSTRING:
	{
		CodeWord c;
		p++;
		get_word(c,p);
		c.w=SawStr(c.w,at);
		p[-2]=c.m.c1;
		p[-1]=c.m.c2;
		break;
	}
	case PUSHFUNCTION:
	{
		CodeCode c;
		p++;
		get_code(c,p);
		c.tf->marked=at;
		break;
	}
	case STORERECORD:
	{
		int n=*++p;
		p++;
		while (n--)
		{
			CodeWord c;
			at=p-code;
			get_word(c,p);
			c.w=SawStr(c.w,at);
			p[-2]=c.m.c1;
			p[-1]=c.m.c2;
		}
		break;
	}
	default:
		printf("\tcannot happen:  opcode=%d",*p);
		p++;
		break;
	}
 }
}

static void DumpFunctions(Byte *code, Byte *end)
{
 Byte *p;
 for (p=code; p!=end;)
 {
	OpCode op=(OpCode)*p;
	switch (op)
	{
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHLOCAL0:
	case PUSHLOCAL1:
	case PUSHLOCAL2:
	case PUSHLOCAL3:
	case PUSHLOCAL4:
	case PUSHLOCAL5:
	case PUSHLOCAL6:
	case PUSHLOCAL7:
	case PUSHLOCAL8:
	case PUSHLOCAL9:
	case PUSHINDEXED:
	case STORELOCAL0:
	case STORELOCAL1:
	case STORELOCAL2:
	case STORELOCAL3:
	case STORELOCAL4:
	case STORELOCAL5:
	case STORELOCAL6:
	case STORELOCAL7:
	case STORELOCAL8:
	case STORELOCAL9:
	case STOREINDEXED0:
	case ADJUST0:
	case EQOP:
	case LTOP:
	case LEOP:
	case GTOP:
	case GEOP:
	case ADDOP:
	case SUBOP:
	case MULTOP:
	case DIVOP:
	case POWOP:
	case CONCOP:
	case MINUSOP:
	case NOTOP:
	case POP:
	case RETCODE0:
		p++;
		break;
	case PUSHBYTE:
	case PUSHLOCAL:
	case STORELOCAL:
	case STOREINDEXED:
	case STORELIST0:
	case ADJUST:
	case RETCODE:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHSELF:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	case STORELIST:
	case CALLFUNC:
	case PUSHSTRING:
	case PUSHGLOBAL:
	case STOREGLOBAL:
		p+=3;
		break;
	case PUSHFLOAT:
		p+=5;
		break;
	case PUSHFUNCTION:
	{
		CodeCode c;
		p++;
		get_code(c,p);
		dump(c.tf);
		break;
	}
	case STORERECORD:
		p += *(p+1)*sizeof(Word) + 2;
		break;
	default:
		printf("\tcannot happen:  opcode=%d",*p);
		p++;
		break;
	}
 }
}

static void DumpWord(int i, FILE *D)
{
 Word w=i;
 fwrite(&w,sizeof(w),1,D);
}

static void DumpString(char *s, FILE *D)
{
 Word w;
 int n=strlen(s)+1;
 if (n>0xFFFF)
 {
  fprintf(stderr,"luac: string too long: \"%.32s...\"\n",s);
  exit(1);
 }
 w=n;
 fwrite(&w,sizeof(w),1,D);
 fwrite(s,n,1,D);
}

static void DumpStrings(FILE *D)
{
 int i;
 extern Word lua_ntable;
 extern Word lua_nconstant;
#define marked varindex
 for (i=0; i<lua_ntable; i++)
  if (lua_table[i].varname->marked>0)
  {
   fputc('V',D);
   DumpWord(lua_table[i].varname->marked,D);
   DumpString(lua_table[i].varname->str,D);
  }
#define marked constindex
 for (i=0; i<lua_nconstant; i++)
  if (lua_constant[i]->marked>0)
  {
   fputc('S',D);
   DumpWord(lua_constant[i]->marked,D);
   DumpString(lua_constant[i]->str,D);
  }
}

static void DumpThreads(Byte *code, FILE *D)
{
 int i;
 extern Word lua_ntable;
 extern Word lua_nconstant;
#define marked varindex
 for (i=0; i<lua_ntable; i++)
  if (lua_table[i].varname->marked>0)
  {
   printf("%s:",lua_table[i].varname->str);
   CheckThread(code,lua_table[i].varname->marked);
  }
#define marked constindex
 for (i=0; i<lua_nconstant; i++)
  if (lua_constant[i]->marked>0)
  {
   printf("\"%s\":",lua_constant[i]->str);
   CheckThread(code,lua_constant[i]->marked);
  }
}

#undef marked
void DumpFunction(TFunc *tf, FILE *D)
{
 fputc('F',D);
 DumpWord(tf->size,D);
 DumpWord(tf->marked,D);
 DumpWord(tf->lineDefined,D);
 DumpString(tf->fileName,D);
#if 1
 ThreadCode(tf->code,tf->code+tf->size);
#else
 ThreadVar(tf->code,tf->code+tf->size);
 DumpThreads(tf->code,D);
 ThreadStr(tf->code,tf->code+tf->size);
#endif
 fwrite(tf->code,tf->size,1,D);
 DumpStrings(D);
 DumpThreads(tf->code,D);
 DumpFunctions(tf->code,tf->code+tf->size);
}

void DumpHeader(FILE *D)
{
 Word w=0x1234;
 float f=1.234567890e-23;
 fputs("\033LUA\002\003",D);		/* <ESC> "LUA" version */
 fwrite(&w,sizeof(w),1,D);		/* a word for testing byte ordering */
 fwrite(&f,sizeof(f),1,D);		/* a float for testing byte ordering */
}

void PrintFunction(TFunc *tf)
{
 if (tf->lineDefined==0)
  printf("\nmain function in file \"%s\" (%d bytes)\n",tf->fileName,tf->size);
 else
  printf("\nfunction defined in file \"%s\" at line %d; used in main at offset %d (%d bytes)\n",tf->fileName,tf->lineDefined,tf->marked,tf->size);
 PrintCode(tf->code,tf->code+tf->size);
}
