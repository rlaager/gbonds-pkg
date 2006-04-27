/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  util.c:  various small utility functions
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <config.h>

#include <string.h>
#include <gnome.h>

#include "util.h"


/****************************************************************************/
/* Append ".gbonds" extension to filename if needed.                        */
/****************************************************************************/
gchar *
gb_util_add_extension (const gchar * orig_filename)
{
	gchar *new_filename, *extension;

	extension = strrchr (orig_filename, '.');
	if (extension == NULL) {
		new_filename = g_strconcat (orig_filename, ".gbonds", NULL);
	} else {
		if (g_strcasecmp (extension, ".gbonds") != 0) {
			new_filename =
			    g_strconcat (orig_filename, ".gbonds", NULL);
		} else {
			new_filename = g_strdup (orig_filename);
		}
	}

	return new_filename;
}

/****************************************************************************/
/* Remove ".gbonds" extension from filename if needed.                      */
/****************************************************************************/
gchar *
gb_util_remove_extension (const gchar * orig_filename)
{
	gchar *new_filename, *extension;

	new_filename = g_strdup (orig_filename);

	extension = strrchr (new_filename, '.');
	if (extension != NULL) {
		if (g_strcasecmp (extension, ".gbonds") == 0) {
			*extension = 0; /* truncate string, rm extension */
		}
	}

	return new_filename;
}

/****************************************************************************/
/* Make sure we have an absolute path to filename.                          */
/****************************************************************************/
gchar *
gb_util_make_absolute (const gchar * filename)
{
	gchar *pwd, *absolute_filename;

	if (g_path_is_absolute (filename)) {
		absolute_filename = g_strdup (filename);
	} else {
		pwd = g_get_current_dir ();
		absolute_filename =
		    g_strjoin (G_DIR_SEPARATOR_S, pwd, filename, NULL);
		g_free (pwd);
	}

	return absolute_filename;
}

/****************************************************************************/
/* Get '~/.gbonds' directory path.                                          */
/****************************************************************************/
gchar *
gb_util_get_home_data_dir (void)
{
	gchar *dir = gnome_util_prepend_user_home( ".gbonds" );

	/* Try to create ~/.gbonds directory.  If it already exists, no problem. */
	mkdir( dir, 0775 );

	return dir;
}
