#include <stdio.h>
#include "opcode.h"
#include "table.h"
#include "tree.h"
#include "types.h"
#include "dump.h"

static char* GlobalVar(int i)
{
 if (i>=1000) return "--threaded--"; else
 return lua_table[i].varname->str;
}

static char* GlobalStr(int i)
{
 if (i>=1000) return "--threaded--"; else
 return lua_constant[i]->str;
}

void PrintCode(Byte *code, Byte *end)
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
 		printf("\t%d", *(p+1));
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
 		printf("\t%d", c.w);
 		break;
 	}
 	case PUSHFLOAT:
 	{
 		CodeFloat c;
 		p++;
 		get_float(c,p);
 		printf("\t%g", c.f);
 		break;
 	}
 	case PUSHSTRING:
 	{
 		CodeWord c;
 		p++;
 		get_word(c,p);
 		printf("\t%d\t; \"%s\"", c.w, GlobalStr(c.w));
 		break;
 	}
 	case PUSHFUNCTION:
 	{
 		CodeCode c;
 		p++;
 		get_code(c,p);
 		printf("\t%p", c.tf);
 		break;
 	}
 	case PUSHGLOBAL:
 	case STOREGLOBAL:
 	{
 		CodeWord c;
 		p++;
 		get_word(c,p);
 		printf("\t%d\t; %s", c.w, GlobalVar(c.w));
 		break;
 	}
 	case STORELIST:
 	case CALLFUNC:
 		printf("\t%d %d", *(p+1), *(p+2));
 		p+=3;
 		break;
 	case STORERECORD:
 		printf("\t%d", *(p+1));
 		p += *p*sizeof(Word) + 2;
 		break;
 	default:
 		printf("\tcannot happen:  opcode=%d", *p);
 		p++;
 		break;
 	}
 	printf("\n");
 }
}

static int SawVar(int i, int at)
{
 int old=lua_table[i].varname->marked;
 lua_table[i].varname->marked=at+1000;
 return old;
}

static int SawStr(int i, int at)
{
 int old=lua_constant[i]->marked;
 lua_constant[i]->marked=at+1000;
 return old;
}

static void CheckThread(Byte *p, int i)
{
 while (i>=1000)
 {
  CodeWord c;
  Byte *q;
  i-=1000;
  q=p+i;
  printf(" %d",i); 
  get_word(c,q);
  i=c.w;
 }
 printf("\n");
}

void ThreadCode(Byte *code, Byte *end)
{
 Byte *p;
 for (p=code; p!=end;)
 {
 	OpCode op=(OpCode)*p;
	int at=p-code;
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
		c.w=SawStr(c.w,at+1);
		p[-2]=c.m.c1;
		p[-1]=c.m.c2;
 		break;
 	}
 	case PUSHFUNCTION:
 	{
 		CodeCode c;
 		p++;
 		get_code(c,p);
 		printf("\t%p\n", c.tf);
 		break;
 	}
 	case PUSHGLOBAL:
 	case STOREGLOBAL:
 	{
 		CodeWord c;
 		p++;
 		get_word(c,p);
 		c.w=SawVar(c.w,at+1);
		p[-2]=c.m.c1;
		p[-1]=c.m.c2;
 		break;
 	}
 	case STORERECORD:
 		p += *p*sizeof(Word) + 2;
 		break;
 	default:
 		printf("\tcannot happen:  opcode=%d", *p);
 		p++;
 		break;
 	}
 }
 {
  Word i;
  extern Word lua_ntable;
  extern Word lua_nconstant;
  for (i=0; i<lua_ntable; i++)
   if (lua_table[i].varname->marked>=1000)
   {
     printf("$global (%s)",lua_table[i].varname->str);
     CheckThread(code,lua_table[i].varname->marked);
   }
  for (i=0; i<lua_nconstant; i++)
   if (lua_constant[i]->marked>=1000)
   {
     printf("$string (%s)",lua_constant[i]->str);
     CheckThread(code,lua_constant[i]->marked);
   }
 }
}
