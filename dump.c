/*
** dump.c
** thread and save bytecodes to file
*/

char* rcs_dump="$Id: dump.c,v 1.9 1996/03/01 03:41:30 lhf Exp lhf $";

#include <stdio.h>
#include <string.h>
#include "luac.h"

static TFunc* lastF=NULL;		/* list of functions seen in code */

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

static void ThreadCode(Byte* code, Byte* end)
{
 Byte* p;
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
		c.tf->next=NULL;	/* TODO: remove? */
		lastF=lastF->next=c.tf;
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
		fprintf(stderr,"luac: cannot happen:  opcode=%d",*p);
		exit(1);
		break;
	}
 }
}

#ifdef CHECKTHREAD
static void CheckThread(Byte* code, int i)
{
 while (i!=0)
 {
  CodeWord c;
  Byte* p=code+i;
  printf(" %d",i);
  get_word(c,p);
  i=c.w;
 }
 printf("\n");
}

static void CheckThreads(Byte* code)
{
 int i;
 printf("-- debug: var threads\n");
 for (i=0; i<lua_ntable; i++)
  if (VarLoc(i)!=0)
  {
   printf("%s:",VarStr(i));
   CheckThread(code,VarLoc(i));
  }
 printf("-- debug: str threads\n");
 for (i=0; i<lua_nconstant; i++)
  if (StrLoc(i)!=0)
  {
   printf("\"%s\":",StrStr(i));
   CheckThread(code,StrLoc(i));
  }
}
#endif

static void DumpWord(int i, FILE* D)
{
 Word w=i;
 fwrite(&w,sizeof(w),1,D);
}

static void DumpBlock(char* b, int size, FILE* D)
{
 fwrite(b,size,1,D);
}

static void DumpString(char* s, FILE* D)
{
 int n=strlen(s)+1;
 if ((Word)n != n)
 {
  fprintf(stderr,"luac: string too long: \"%.32s...\"\n",s);
  exit(1);
 }
 DumpWord(n,D);
 DumpBlock(s,n,D);
}

static void DumpStrings(FILE* D)
{
 int i;
 for (i=0; i<lua_ntable; i++)
 {
  if (VarLoc(i)!=0)
  {
   fputc(ID_VAR,D);
   DumpWord(VarLoc(i),D);
   DumpString(VarStr(i),D);
  }
  VarLoc(i)=i;
 }
 for (i=0; i<lua_nconstant; i++)
 {
  if (StrLoc(i)!=0)
  {
   fputc(ID_STR,D);
   DumpWord(StrLoc(i),D);
   DumpString(StrStr(i),D);
  }
  StrLoc(i)=i;
 }
}

void DumpFunction(TFunc* tf, FILE* D)
{
 lastF=tf;
 ThreadCode(tf->code,tf->code+tf->size);
 fputc(ID_FUN,D);
 DumpWord(tf->size,D);			/* TODO: test if too long? */
 DumpWord(tf->lineDefined,D);
 if (IsMain(tf))
  DumpString(tf->fileName,D);
 else
  DumpWord(tf->marked,D);
 DumpBlock(tf->code,tf->size,D);
 DumpStrings(D);
#ifdef CHECKTHREAD
CheckThreads(tf->code);			/* TODO: remove */
#endif
}

void DumpHeader(FILE* D)
{
 Word w=TEST_WORD;
 float f=TEST_FLOAT;
 fputc(ID_CHUNK,D);
 fputs(SIGNATURE,D);
 fputc(VERSION,D);
 fwrite(&w,sizeof(w),1,D);		/* a word for testing byte ordering */
 fwrite(&f,sizeof(f),1,D);		/* a float for testing byte ordering */
}
