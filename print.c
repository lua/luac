/*
** print.c
** print bytecodes
*/

char *rcs_print="$Id$";

#include <stdio.h>
#include <string.h>
#include "luac.h"
#include "print.h"

#if 0
#define VarStr(i)	""
#define StrStr(i)	""
#endif

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

void PrintFunction(TFunc *tf)
{
 if (tf->lineDefined==0)
  printf("\nmain of \"%s\" (%d bytes)\n",tf->fileName,tf->size);
 else
  printf("\nfunction \"%s\":%d (%d bytes); used at main+%d\n",
	tf->fileName,tf->lineDefined,tf->size,tf->marked);
 PrintCode(tf->code,tf->code+tf->size);
}
