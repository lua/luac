/*
** $Id: $
** definitions for luac
** Copyrightnotice
*/

#include "auxlib.h"
#include "lfunc.h"
#include "lglobal.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#define VarStr(i)	(luaG_global[i].varname->str)
#define tfvalue(o)	((o)->value.tf)
