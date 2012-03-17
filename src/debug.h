/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  debug.h:  gbonds debug module
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/*
 * This file is based on gedit-debug.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <glib.h>

G_BEGIN_DECLS

/*
 * Set an environmental var of the same name to turn on
 * debugging output. Setting GBONDS_DEBUG will turn on all
 * sections.
 */

typedef enum {
	GBONDS_DEBUG_NONE         = 0,
	GBONDS_DEBUG_VIEW         = 1 << 0,
	GBONDS_DEBUG_PRINT        = 1 << 1,
	GBONDS_DEBUG_PREFS        = 1 << 2,
	GBONDS_DEBUG_FILE         = 1 << 3,
	GBONDS_DEBUG_DOC          = 1 << 4,
	GBONDS_DEBUG_XML          = 1 << 5,
	GBONDS_DEBUG_TABLE        = 1 << 6,
	GBONDS_DEBUG_UPDATE       = 1 << 7,
	GBONDS_DEBUG_UNDO         = 1 << 8,
	GBONDS_DEBUG_RECENT       = 1 << 9,
	GBONDS_DEBUG_COMMANDS     = 1 << 10,
	GBONDS_DEBUG_WINDOW       = 1 << 11,
	GBONDS_DEBUG_UI           = 1 << 12,
} gbDebugSection;

#ifndef __GNUC__
#define __FUNCTION__   ""
#endif

#define	DEBUG_VIEW	GBONDS_DEBUG_VIEW,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PRINT	GBONDS_DEBUG_PRINT,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PREFS	GBONDS_DEBUG_PREFS,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_FILE	GBONDS_DEBUG_FILE,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_DOC	GBONDS_DEBUG_DOC,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_XML	GBONDS_DEBUG_XML,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_TABLE	GBONDS_DEBUG_TABLE,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UPDATE	GBONDS_DEBUG_UPDATE,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UNDO	GBONDS_DEBUG_UNDO,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_RECENT	GBONDS_DEBUG_RECENT,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_COMMANDS	GBONDS_DEBUG_COMMANDS,__FILE__, __LINE__, __FUNCTION__
#define	DEBUG_WINDOW    GBONDS_DEBUG_WINDOW,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UI        GBONDS_DEBUG_UI,      __FILE__, __LINE__, __FUNCTION__

void gb_debug_init (void);

void gb_debug      (gbDebugSection  section,
		    const gchar    *file,
		    gint            line,
		    const gchar    *function,
		    const gchar    *format,
		    ...);

G_END_DECLS

#endif /* __DEBUG_H__ */
