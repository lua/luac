/*
** $Id: $
** print bytecodes
** Copyrightnotice
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "luac.h"
#include "print.h"

#define LocStr(i)	luaF_getlocalname(tf,i+1,line)

static void PrintConstants(TFunc* tf)
{
 int i;
 int n=tf->nconsts;
 printf("constants (%d):",n);
 for (i=0; i<n; i++)
 {
  TObject* o=tf->consts+i;
  printf("\n\t%d ",i);
  switch (ttype(o))
  {
   case LUA_T_NUMBER:	printf("N %g",nvalue(o)); break;
   case LUA_T_STRING:	printf("S \"%s\"",svalue(o)); break;
   case LUA_T_FUNCTION:	printf("F %p",tfvalue(o)); break;
   default:		printf("? %d",ttype(o)); break;
  }
 }
 printf("\n");
}

static void PrintConstant(TFunc* tf, int n)
{
 if (n<0 || n>=tf->nconsts)
 {
  printf("(bad constant)");
 }
 else
 {
  TObject* o=tf->consts+n;
  switch (ttype(o))
  {
   case LUA_T_NUMBER:	printf("%g",nvalue(o)); break;
   case LUA_T_STRING:	printf("\"%s\"",svalue(o)); break;
   case LUA_T_FUNCTION:	printf("function at %p",tfvalue(o)); break;
   default:		printf("cannot handle ttype(o)=%d",ttype(o)); break;
  }
 }
}

static void PrintCode(TFunc* tf)
{
 Byte* code=tf->code;
 Byte* p=code;
 int line=0;
 while (1)
 {
	int op=*p;
	if (op>=NOPCODES)		/* cannot happen */
	 luaL_verror("bad opcode %d at %d\n",op,(int)(p-code));
	printf("%6d\t%-13s",(int)(p-code),OpCodeName[op]);
	switch (op)
	{
	case ENDCODE:
#if 0
		printf("\t(%d bytes of code)",(int)(p-code)+1);
#endif
		printf("\n");
		return;
	case PUSHNIL:
	case PUSH0:
	case PUSH1:
	case PUSH2:
	case PUSHINDEXED:
	case SETINDEXED0:
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
		if (tf->locvars) printf("\t\t; %s",LocStr(i));
		p++;
		break;
	}
	case SETLOCAL0:
	case SETLOCAL1:
	case SETLOCAL2:
	case SETLOCAL3:
	case SETLOCAL4:
	case SETLOCAL5:
	case SETLOCAL6:
	case SETLOCAL7:
	case SETLOCAL8:
	case SETLOCAL9:
	{
		int i=op-SETLOCAL0;
		if (tf->locvars) printf("\t\t; %s",LocStr(i));
		p++;
		break;
	}
	case PUSHLOCAL:
	case SETLOCAL:
	{
		int i=*(p+1);
		if (tf->locvars) printf("\t%d\t; %s",i,LocStr(i));
		p+=2;
		break;
	}
	case PUSHCONSTANTB:
	case PUSHCONSTANT:
	case PUSHSELF:
	{
		Word w;
		p++;
		if (op==PUSHCONSTANTB) w=*p++; else get_word(w,p);
		printf("\t%d\t; ",w);
		PrintConstant(tf,w);
		break;
	}
	case PUSHNILS:
	case PUSHBYTE:
	case SETINDEXED:
	case SETLIST0:
	case SETMAP:
	case RETCODE:
	case ADJUST:
	case POPS:
	case VARARGS:
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
		if (op==SETLINE) {
			printf("\t; \"%s\":%d",tf->fileName->str,line=w);
		}
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
		printf("\t%d\t; to %d",w,(int)(p-code)+w);
		break;
	}
	case UPJMP:
	case IFFUPJMP:
	{		/* suggested by Norman Ramsey <nr@cs.virginia.edu> */
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d\t; to %d",w,(int)(p-code)-w);
		break;
	}
	case PUSHGLOBAL:
	case SETGLOBAL:
	{
		Word w;
		p++;
		get_word(w,p);
		printf("\t%d\t; %s",w,VarStr(w));
		break;
	}
	case SETLIST:
	case CALLFUNC:
		printf("\t%d %d",*(p+1),*(p+2));
		p+=3;
		break;
	default:			/* cannot happen */
		printf("\tcannot happen:  opcode=%d\n",*p);
	 	luaL_verror("bad opcode %d at %d\n",*p,(int)(p-code));
		break;
	}
	printf("\n");
 }
}

#undef LocStr

static void PrintLocals(LocVar* v, int n)
{
 int i=0;
 if (v==NULL || v->varname==NULL) return;
 if (n>0)
 {
  printf("params:");
  for (i=0; i<n; v++,i++) printf(" %s",v->varname->str,i,v->line);
  printf("\n");
 }
 if (v->varname!=NULL)
 {
  printf("locals:");
  for (; v->line>=0; v++)
  {
   if (v->varname==NULL)
   {
    printf(")");
    --i;
   }
   else
    printf(" (%s",v->varname->str,i++,v->line);
  }
  printf("\n");
 }
}

static Byte* FindFunction(TFunc* tf, TFunc* Main)
{
 Byte* code=Main->code;
 Byte* p=code;
 while (1)
 {
	int op=*p;
	switch (op)
	{
	case ENDCODE:
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
	case SETLOCAL0:
	case SETLOCAL1:
	case SETLOCAL2:
	case SETLOCAL3:
	case SETLOCAL4:
	case SETLOCAL5:
	case SETLOCAL6:
	case SETLOCAL7:
	case SETLOCAL8:
	case SETLOCAL9:
	case SETINDEXED0:
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
	case RETCODE0:
		p++;
		break;
	case PUSHNILS:
	case PUSHBYTE:
	case PUSHLOCAL:
	case SETLOCAL:
	case SETINDEXED:
	case SETLIST0:
	case SETMAP:
	case RETCODE:
	case ADJUST:
	case POPS:
	case VARARGS:
		p+=2;
		break;
	case PUSHWORD:
	case PUSHGLOBAL:
	case PUSHSELF:
	case CREATEARRAY:
	case SETGLOBAL:
	case ONTJMP:
	case ONFJMP:
	case JMP:
	case UPJMP:
	case IFFJMP:
	case IFFUPJMP:
	case SETLINE:
	case SETLIST:
	case CALLFUNC:
		p+=3;
		break;
	case PUSHCONSTANTB:
	case PUSHCONSTANT:
	{
		Word w;
		TObject* o;
		p++;
		if (op==PUSHCONSTANTB) w=*p++; else get_word(w,p);
		o=Main->consts+w;
		if (ttype(o)==LUA_T_FUNCTION && tfvalue(o)==tf) return p;
		break;
	}
	default:			/* cannot happen */
		luaL_verror("bad opcode %d at %d\n",*p,(int)(p-code));
		break;
	}
 }
}

static void PrintHeader(TFunc* tf, TFunc* Main)
{
 if (IsMain(tf))
  printf("\nmain of \"%s\" (%d bytes at %p)\n",tf->fileName->str,0,tf);
 else
 {
  Byte* p=FindFunction(tf,Main);
  Byte* op=p;
  printf("\nfunction ");
  switch (*p)				/* try to get name */
  {
   case SETGLOBAL:
   {
    Word w;
    p++; get_word(w,p); printf("%s ",VarStr(w));
    break;
   }
   case SETINDEXED0:			/* try method definition */
   {
    Word w;
    int t=-1,m=-1;
    if (p[-4]==PUSHCONSTANTB && p[-7]==PUSHGLOBAL)
    {
     p=op-7; p++; get_word(t,p);
     p=op-4; p++; m=*p;
    }
    if (t>=0 && m>=0)
    {
     int c=(tf->locvars && tf->locvars->varname &&
     strcmp(tf->locvars->varname->str,"self")==0)
		? ':' : '.';
     printf("%s%c%s ",VarStr(t),c,svalue(&Main->consts[m]));
    }
    break;
   }
  }
  printf("defined at \"%s\":%d (%d bytes at %p); constant %d of main\n",
	tf->fileName->str,tf->lineDefined,0,tf,tf->marked);
 }
}

void PrintFunction(TFunc* tf, TFunc* Main)
{
 int n=0;
 PrintHeader(tf,Main);
 if (!IsMain(tf))			/* get number of parameters */
 {
  Byte* p=tf->code;
  while (*p==SETLINE) p+=3;
  if (*p==ADJUST) n=p[1];
 }
 PrintLocals(tf->locvars,n);
 PrintCode(tf);
#if 0
 PrintConstants(tf);
#endif
}
