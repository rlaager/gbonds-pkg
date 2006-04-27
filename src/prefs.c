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

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include <gconf/gconf-client.h>

#include "prefs.h"

#include "debug.h"

gbPrefs      *gb_prefs     = NULL;

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/* GConf keys */
#define BASE_KEY                            "/apps/gbonds"

#define PREF_STARTUP_ACTION                 "/startup-action"
#define PREF_STARTUP_FILE                   "/startup-file"

#define PREF_RDATE_RANGE                    "/rdate-range"

#define PREF_MAIN_TOOLBAR_VISIBLE           "/main-toolbar-visible"
#define PREF_MAIN_TOOLBAR_BUTTONS_STYLE     "/main-toolbar-buttons-style"
#define PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS     "/main-toolbar-view-tooltips"

#define PREF_MAX_RECENTS                    "/max-recents"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/
static GConfClient *gconf_client = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void notify_cb (GConfClient *client,
		       guint cnxn_id,
		       GConfEntry *entry,
		       gpointer user_data);

static gchar *get_string (GConfClient* client, const gchar* key, const gchar* def);
static gboolean get_bool (GConfClient* client, const gchar* key, gboolean def);
static gint get_int (GConfClient* client, const gchar* key, gint def);
static gdouble get_float (GConfClient* client, const gchar* key, gdouble def);


/*****************************************************************************/
/* Initialize preferences module.                                            */
/*****************************************************************************/
void 
gb_prefs_init (void)
{
	gb_debug (DEBUG_PREFS, "");

	gconf_client = gconf_client_get_default ();
	
	g_return_if_fail (gconf_client != NULL);

	gconf_client_add_dir (gconf_client,
			      BASE_KEY,
			      GCONF_CLIENT_PRELOAD_ONELEVEL,
			      NULL);
	
	gconf_client_notify_add (gconf_client,
				 BASE_KEY,
				 notify_cb,
				 NULL, NULL, NULL);
}


/*****************************************************************************/
/* Save all settings.                                                        */
/*****************************************************************************/
void 
gb_prefs_save_settings (void)
{
	gb_debug (DEBUG_PREFS, "START");
	
	g_return_if_fail (gconf_client != NULL);
	g_return_if_fail (gb_prefs != NULL);


	/* Startup action */
	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_STARTUP_ACTION,
			      gb_prefs->startup_action,
			      NULL);
	if (gb_prefs->startup_file) {
		gconf_client_set_string (gconf_client,
					 BASE_KEY PREF_STARTUP_FILE,
					 gb_prefs->startup_file,
					 NULL);
	}

	/* Rdate range */
	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_RDATE_RANGE,
			      gb_prefs->rdate_range,
			      NULL);

	/* Main Toolbar */
	gconf_client_set_bool (gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			       gb_prefs->main_toolbar_visible,
			       NULL);

	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_MAIN_TOOLBAR_BUTTONS_STYLE,
			      gb_prefs->main_toolbar_buttons_style,
			      NULL);

	gconf_client_set_bool (gconf_client,
			       BASE_KEY PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS,
			       gb_prefs->main_toolbar_view_tooltips,
			       NULL);

	/* Recent files */
	gconf_client_set_int (gconf_client,
			      BASE_KEY PREF_MAX_RECENTS,
			      gb_prefs->max_recents,
			      NULL);


	gconf_client_suggest_sync (gconf_client, NULL);
	
	gb_debug (DEBUG_PREFS, "END");
}

/*****************************************************************************/
/* Load all settings.                                                        */
/*****************************************************************************/
void
gb_prefs_load_settings (void)
{
	gchar *string;

	gb_debug (DEBUG_PREFS, "START");
	
	if (gb_prefs == NULL)
		gb_prefs = g_new0 (gbPrefs, 1);

	if (gconf_client == NULL)
	{
		/* TODO: in any case set default values */
		g_warning ("Cannot load settings.");
		return;
	}


	/* Startup action */
	gb_prefs->startup_action =
		get_int (gconf_client,
			 BASE_KEY PREF_STARTUP_ACTION,
			 GB_PREFS_STARTUP_BLANK);

	gb_prefs->startup_file =
		get_string (gconf_client,
			    BASE_KEY PREF_STARTUP_FILE,
			    NULL);

	/* Rdate range */
	gb_prefs->rdate_range =
		get_int (gconf_client,
			 BASE_KEY PREF_RDATE_RANGE,
			 GB_PREFS_RDATE_RANGE_YEAR);

	/* User Inferface/Main Toolbar */
	gb_prefs->main_toolbar_visible =
		get_bool (gconf_client,
			  BASE_KEY PREF_MAIN_TOOLBAR_VISIBLE,
			  TRUE);

	gb_prefs->main_toolbar_buttons_style =
		get_int (gconf_client,
			 BASE_KEY PREF_MAIN_TOOLBAR_BUTTONS_STYLE,
			 GB_TOOLBAR_SYSTEM);

	gb_prefs->main_toolbar_view_tooltips =
		get_bool (gconf_client,
			  BASE_KEY PREF_MAIN_TOOLBAR_VIEW_TOOLTIPS,
			  TRUE);

	/* Recent files */
	gb_prefs->max_recents =
		get_int (gconf_client,
			 BASE_KEY PREF_MAX_RECENTS,
			 4);


	gb_debug (DEBUG_PREFS, "max_recents = %d", gb_prefs->max_recents);

	gb_debug (DEBUG_PREFS, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Key changed callback.                                           */
/*---------------------------------------------------------------------------*/
static void 
notify_cb (GConfClient *client,
	   guint cnxn_id,
	   GConfEntry *entry,
	   gpointer user_data)
{
	gb_debug (DEBUG_PREFS, "Key was changed: %s", entry->key);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Utilities to get values with defaults.                          */
/*---------------------------------------------------------------------------*/
static gchar*
get_string (GConfClient* client,
	    const gchar* key,
	    const gchar* def)
{
  gchar* val;

  val = gconf_client_get_string (client, key, NULL);

  if (val != NULL) {

      return val;

  } else {

      return def ? g_strdup (def) : NULL;

  }
}

static gboolean
get_bool (GConfClient* client,
	  const gchar* key,
	  gboolean def)
{
  GConfValue* val;
  gboolean retval;

  val = gconf_client_get (client, key, NULL);

  if (val != NULL) {

	  if ( val->type == GCONF_VALUE_BOOL ) {
		  retval = gconf_value_get_bool (val);
	  } else {
		  retval = def;
	  }

	  gconf_value_free (val);

	  return retval;

  } else {

      return def;

  }
}

static gint
get_int (GConfClient* client,
	 const gchar* key,
	 gint def)
{
  GConfValue* val;
  gint retval;

  val = gconf_client_get (client, key, NULL);

  if (val != NULL) {

	  if ( val->type == GCONF_VALUE_INT) {
		  retval = gconf_value_get_int(val);
	  } else {
		  retval = def;
	  }

	  gconf_value_free (val);

	  return retval;

  } else {

	  return def;

  }
}

static gdouble
get_float (GConfClient* client,
	   const gchar* key,
	   gdouble def)
{
  GConfValue* val;
  gdouble retval;

  val = gconf_client_get (client, key, NULL);

  if (val != NULL) {

	  if ( val->type == GCONF_VALUE_FLOAT ) {
		  retval = gconf_value_get_float(val);
	  } else {
		  retval = def;
	  }

	  gconf_value_free (val);

	  return retval;

  } else {

	  return def;

  }
}

