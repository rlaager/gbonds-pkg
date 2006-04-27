/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  debug.c:  gbonds debug module
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
 * This file is based on gedit-debug.c from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#include "debug.h"

gbDebugSection debug_flags = GBONDS_DEBUG_NONE;


/****************************************************************************/
/* Initialize debug flags, based on environmental variables.                */
/****************************************************************************/
void
gb_debug_init (void)
{
	if (g_getenv ("GBONDS_DEBUG") != NULL)
	{
		/* enable all debugging */
		debug_flags = ~GBONDS_DEBUG_NONE;
		return;
	}

	if (g_getenv ("GBONDS_DEBUG_VIEW") != NULL)
		debug_flags |= GBONDS_DEBUG_VIEW;
	if (g_getenv ("GBONDS_DEBUG_PRINT") != NULL)
		debug_flags |= GBONDS_DEBUG_PRINT;
	if (g_getenv ("GBONDS_DEBUG_PREFS") != NULL)
		debug_flags |= GBONDS_DEBUG_PREFS;
	if (g_getenv ("GBONDS_DEBUG_FILE") != NULL)
		debug_flags |= GBONDS_DEBUG_FILE;
	if (g_getenv ("GBONDS_DEBUG_DOC") != NULL)
		debug_flags |= GBONDS_DEBUG_DOC;
	if (g_getenv ("GBONDS_DEBUG_XML") != NULL)
		debug_flags |= GBONDS_DEBUG_XML;
	if (g_getenv ("GBONDS_DEBUG_TABLE") != NULL)
		debug_flags |= GBONDS_DEBUG_TABLE;
	if (g_getenv ("GBONDS_DEBUG_UPDATE") != NULL)
		debug_flags |= GBONDS_DEBUG_UPDATE;
	if (g_getenv ("GBONDS_DEBUG_UNDO") != NULL)
		debug_flags |= GBONDS_DEBUG_UNDO;
	if (g_getenv ("GBONDS_DEBUG_RECENT") != NULL)
		debug_flags |= GBONDS_DEBUG_RECENT;
	if (g_getenv ("GBONDS_DEBUG_COMMANDS") != NULL)
		debug_flags |= GBONDS_DEBUG_COMMANDS;
	if (g_getenv ("GBONDS_DEBUG_WINDOW") != NULL)
		debug_flags |= GBONDS_DEBUG_WINDOW;
	if (g_getenv ("GBONDS_DEBUG_UI") != NULL)
		debug_flags |= GBONDS_DEBUG_UI;
}


/****************************************************************************/
/* Print debugging information.                                             */
/****************************************************************************/
void
gb_debug (gbDebugSection  section,
	  const gchar    *file,
	  gint            line,
	  const gchar    *function,
	  const gchar    *format,
	  ...)
{
	if  (debug_flags & section)
	{
		va_list  args;
		gchar   *msg;

		g_return_if_fail (format != NULL);

		va_start (args, format);
		msg = g_strdup_vprintf (format, args);
		va_end (args);

		g_print ("%s:%d (%s) %s\n", file, line, function, msg);

		g_free (msg);
	}
	
}
