/*
** $Id: lundump.h,v 1.2 1997/12/02 23:18:50 lhf Exp lhf $
** load pre-compiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#include "lobject.h"
#include "lzio.h"

#define ID_CHUNK	27		/* ESC */
#define ID_FUNCTION	'#'
#define ID_END		'$'
#define ID_NUM		'N'
#define ID_STR		'S'
#define ID_FUN		'F'
#define	SIGNATURE	"Lua"
#define	VERSION		0x31		/* last format change was in 3.1 */
#define	TEST_FLOAT	0.123456789e-23	/* a float for testing representation */

#define IsMain(f)	(f->lineDefined==0)
#define NotWord(x)	((Word)x!=x)
#define Get_word(w,p)	w=p[0]+(p[1]<<8)
#define get_word(w,p)	Get_word(w,p), p+=2
#define get_byte(p)	*p++

TFunc* luaU_undump1(ZIO* Z);		/* load one chunk */

#endif
