/*
** $Id: $
** load pre-compiled Lua chunks
** Copyrightnotice
*/

#ifndef lundump_h
#define lundump_h

#include "lobject.h"
#include "lzio.h"

#define ID_CHUNK	27		/* ESC */
#define ID_FUNCTION	'$'
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

TFunc* luaU_undump1(ZIO* Z, char* filename);	/* load one chunk */

#endif
