/*
** print.c
** print bytecodes
*/

char* rcs_print="$Id: print.c,v 1.14 1997/06/19 14:56:04 lhf Exp lhf $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#include "print.h"

void LinkFunctions(TFunc* m)
{
 static TFunc* lastF;			/* list of functions seen in code */
 Byte* code=m->code;
 Byte* end=code+m->size;
 Byte* p;
 if (IsMain(m)) lastF=m;
 for (p=code; p!=end;)
 {
	int op=*p;
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
	case VARARGS:
	case STOREMAP:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHSTRING:
	case PUSHGLOBAL:
	case PUSHSELF:
	case STOREGLOBAL:
	case CREATEARRAY:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case CALLFUNC:
	case SETLINE:
	case STORELIST:
		p+=3;
		break;
	case PUSHFLOAT:
		p+=5;			/* assumes sizeof(float)==4 */
		break;
	case PUSHFUNCTION:
	{
		TFunc* tf;
		p++;
		get_code(tf,p);
		tf->marked=at;
		tf->next=NULL;		/* TODO: remove? */
		lastF=lastF->next=tf;
		break;
	}
	case STORERECORD:
	{
		int n=*++p;
		p+=2*n+1;
		break;
	}
	default:			/* cannot happen */
		fprintf(stderr,"luac: bad opcode %d at %d\n",*p,(int)(p-code));
		exit(1);
		break;
	}
 }
}

static LocVar* V=NULL;

static char* LocStr(int i)
{
 if (V==NULL) return ""; else return V[i].varname->str;
}

static void PrintCode(Byte* code, Byte* end)
{
 Byte* p;
 for (p=code; p!=end;)
 {
	int op=*p;
	if (op>=NOPCODES)
	{
	 fprintf(stderr,"luac: bad opcode %d at %d\n",op,(int)(p-code));
	 exit(1);
	}
	printf("%6d\t%s",(int)(p-code),OpCodeName[op]);
	switch (op)
	{
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHINDEXED:
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
	{
		int i=op-PUSHLOCAL0;
		printf("\t\t; %s",LocStr(i));
		p++;
		break;
	}
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
	{
		int i=op-STORELOCAL0;
		printf("\t\t; %s",LocStr(i));
		p++;
		break;
	}
	case PUSHLOCAL:
	case STORELOCAL:
	{
		int i=*(p+1);
		printf("\t%d\t; %s",i,LocStr(i));
		p+=2;
		break;
	}
	case PUSHBYTE:
	case STOREINDEXED:
	case STORELIST0:
	case ADJUST:
	case RETCODE:
	case VARARGS:
	case STOREMAP:
		printf("\t%d",*(p+1));
		p+=2;
		break;
	case PUSHWORD:
	case CREATEARRAY:
	case SETLINE:
	{
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d",w);
		break;
	}
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case IFFJMP:
	{		/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d\t\t; to %d",w,(int)(p-code)+w);
		break;
	}
	case UPJMP:
	case IFFUPJMP:
	{		/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d\t\t; to %d",w,(int)(p-code)-w);
		break;
	}
	case PUSHFLOAT:
	{
		float f;
		p++;
		get_float(f,p);
		printf("\t%g",f);
		break;
	}
	case PUSHSELF:
	case PUSHSTRING:
	{
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d\t; \"%s\"",w,StrStr(w));
		break;
	}
	case PUSHFUNCTION:
	{
		TFunc* tf;
		p++;
		get_code(tf,p);
		printf("\t%p\t; \"%s\":%d",tf,tf->fileName,tf->lineDefined);
		break;
	}
	case PUSHGLOBAL:
	case STOREGLOBAL:
	{
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d\t; %s",w,VarStr(w));
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
			Word w;
			printf("\n%6d\t      FIELD",(int)(p-code));
			get_word(w,p);
			printf("\t%d\t; \"%s\"",w,StrStr(w));
		}
		break;
	}
	default:
		printf("\tcannot happen:  opcode=%d\n",*p);
	 	fprintf(stderr,"luac: bad opcode %d at %d\n",op,(int)(p-code));
		exit(1);
		break;
	}
	printf("\n");
 }
}

void PrintFunction(TFunc* tf, TFunc* Main)
{
 if (IsMain(tf))
  printf("\nmain of \"%s\" (%d bytes at %p)\n",tf->fileName,tf->size,tf);
 else
 {
  Byte* p=Main->code+tf->marked+sizeof(TFunc*);
  printf("\nfunction ");
  switch (*p)
  {
   case STOREGLOBAL:
   {
    Word w;
    p++; get_word(w,p); printf("%s defined at ",VarStr(w));
    break;
   }
   case STOREINDEXED0:			/* try method definition */
   {
    if (p[-11]==PUSHGLOBAL && p[-8]==PUSHSTRING)
    {
     Byte* pp=p;
     Word w;
     p-=11; p++; get_word(w,p); printf("%s:",VarStr(w));
     p=pp;
     p-=8;  p++; get_word(w,p); printf("%s defined at ",StrStr(w));
     p=pp;
    }
    break;
   }
  }
  printf("\"%s\":%d (%d bytes at %p); used at main+%d\n",
	tf->fileName,tf->lineDefined,tf->size,tf,tf->marked);
 }
 V=tf->locvars;
 PrintCode(tf->code,tf->code+tf->size);
}
