/*
** dump.c
** print,thread,and saves bytecodes to files
*/

char *rcs_dump="$Id$";

#include <stdio.h>
#include <string.h>
#include "opcode.h"
#include "table.h"
#include "tree.h"
#include "types.h"
#include "dump.h"

#define VarStr(i)	(lua_table[i].varname->str)
#define VarLoc(i)	(lua_table[i].varname->varindex)
#define StrStr(i)	(lua_constant[i]->str)
#define StrLoc(i)	(lua_constant[i]->constindex)

extern Word lua_ntable;
extern Word lua_nconstant;

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
		printf("\t%d\t; \"%s\"",c.w,StrStr(c.w));
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
		printf("\t%d\t; %s",c.w,VarStr(c.w));
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
			printf("\t%d\t; \"%s\"",c.w,StrStr(c.w));
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

static int SawVar(int i, int at)
{
 int old=VarLoc(i);
 VarLoc(i)=at;
 return old;
}

static int SawStr(int i, int at)
{
 int old=StrLoc(i);
 StrLoc(i)=at;
 return old;
}

static void ThreadCode(Byte *code, Byte *end)
{
 Byte *p;
 int i;
 for (i=0; i<lua_ntable; i++) VarLoc(i)=0;
 for (i=0; i<lua_nconstant; i++) StrLoc(i)=0;
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

static void CheckThreads(Byte *code)
{
 int i;
 for (i=0; i<lua_ntable; i++)
  if (VarLoc(i)!=0)
  {
   printf("%s:",VarStr(i));
   CheckThread(code,VarLoc(i));
  }
 for (i=0; i<lua_nconstant; i++)
  if (StrLoc(i)!=0)
  {
   printf("\"%s\":",StrStr(i));
   CheckThread(code,StrLoc(i));
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
 for (i=0; i<lua_ntable; i++)
  if (VarLoc(i)!=0)
  {
   fputc('V',D);
   DumpWord(VarLoc(i),D);
   DumpString(VarStr(i),D);
  }
 for (i=0; i<lua_nconstant; i++)
  if (StrLoc(i)!=0)
  {
   fputc('S',D);
   DumpWord(StrLoc(i),D);
   DumpString(StrStr(i),D);
  }
}

void DumpFunction(TFunc *tf, FILE *D)
{
 fputc('F',D);
 DumpWord(tf->size,D);
 DumpWord(tf->marked,D);
 DumpWord(tf->lineDefined,D);
 DumpString(tf->fileName,D);
 ThreadCode(tf->code,tf->code+tf->size);
 fwrite(tf->code,tf->size,1,D);
 DumpStrings(D);
CheckThreads(tf->code);
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
