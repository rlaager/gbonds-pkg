/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  prefs.h:  Application preferences module header file
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
#ifndef __PREFS_H__
#define __PREFS_H__

G_BEGIN_DECLS

typedef struct _gbPrefs gbPrefs;

typedef enum {
  GB_PREFS_STARTUP_BLANK,
  GB_PREFS_STARTUP_RECENT,
  GB_PREFS_STARTUP_DEFAULT,
} gbPrefsStartupAction;


typedef enum {
  GB_PREFS_RDATE_RANGE_ALL,
  GB_PREFS_RDATE_RANGE_YEAR,
} gbPrefsRdateRange;;

typedef enum {
	GB_TOOLBAR_SYSTEM = 0,
	GB_TOOLBAR_ICONS,
	GB_TOOLBAR_ICONS_AND_TEXT
} gbToolbarSetting;

struct _gbPrefs
{
	gbPrefsStartupAction  startup_action;
	gchar                *startup_file;

	gbPrefsRdateRange     rdate_range;

	/* User Interface/Main Toolbar */
	gboolean	      main_toolbar_visible;
	gbToolbarSetting      main_toolbar_buttons_style; 
	gboolean	      main_toolbar_view_tooltips;

	/* Recent files */
	gint                  max_recents;
};


gbPrefs *gb_prefs;


void          gb_prefs_save_settings       (void);

void          gb_prefs_load_settings       (void);

void          gb_prefs_init                (void);

G_END_DECLS

#endif /* __PREFS_H__ */

