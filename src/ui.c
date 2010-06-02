/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  ui.c:  gbonds ui module
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
#include <config.h>

#include "recent-files/egg-recent-view.h"
#include "recent-files/egg-recent-view-bonobo.h"
#include <gconf/gconf-client.h>

#include "ui.h"
#include "commands.h"
#include "recent.h" 
#include "file.h"
#include "prefs.h"

#include "debug.h"

/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/
#define GBONDS_UI_XML GBONDS_UI_DIR "gbonds-ui.xml"

/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/


/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/

static BonoboUIVerb gb_ui_verbs [] = {
	BONOBO_UI_VERB ("FileNew",               gb_cmd_file_new),
	BONOBO_UI_VERB ("FileOpen",              gb_cmd_file_open),
	BONOBO_UI_VERB ("FileImport",            gb_cmd_file_import),
	BONOBO_UI_VERB ("FileSave",              gb_cmd_file_save),
	BONOBO_UI_VERB ("FileSaveAs",            gb_cmd_file_save_as),
	BONOBO_UI_VERB ("FilePrint",             gb_cmd_file_print),
	BONOBO_UI_VERB ("FileClose",             gb_cmd_file_close),
	BONOBO_UI_VERB ("FileExit",              gb_cmd_file_exit),
	BONOBO_UI_VERB ("EditCut",               gb_cmd_edit_cut),
	BONOBO_UI_VERB ("EditCopy",              gb_cmd_edit_copy),
	BONOBO_UI_VERB ("EditPaste",             gb_cmd_edit_paste),
	BONOBO_UI_VERB ("EditAdd",               gb_cmd_edit_add),
	BONOBO_UI_VERB ("EditDelete",            gb_cmd_edit_delete),
	BONOBO_UI_VERB ("EditTitle",             gb_cmd_edit_title),
	BONOBO_UI_VERB ("EditSelectAll",         gb_cmd_edit_select_all),
	BONOBO_UI_VERB ("EditUnSelectAll",       gb_cmd_edit_unselect_all),
	BONOBO_UI_VERB ("SettingsPreferences",   gb_cmd_settings_preferences),
	BONOBO_UI_VERB ("SettingsUpdate",        gb_cmd_settings_update),
	BONOBO_UI_VERB ("HelpContents",          gb_cmd_help_contents),
	BONOBO_UI_VERB ("About",                 gb_cmd_help_about),

	BONOBO_UI_VERB_END
};

static gchar* doc_verbs [] = {
	"/commands/FileSave",
	"/commands/FileSaveAs",
	"/commands/FilePrint",
	"/commands/FilePrintPreview",
	"/commands/FileClose",
	"/commands/FileCloseAll",
	"/commands/EditUndo",
	"/commands/EditRedo",
	"/commands/EditCut",
	"/commands/EditCopy",
	"/commands/EditPaste",
	"/commands/EditAdd",
	"/commands/EditTitle",
	"/commands/EditDelete",
	"/commands/EditSelectAll",
	"/commands/EditUnSelectAll",

	NULL
};

static gchar* doc_modified_verbs [] = {
	"/commands/FileSave",

	NULL
};

static gchar* selection_verbs [] = {
	"/commands/EditCut",
	"/commands/EditCopy",
	"/commands/EditDelete",
	"/commands/EditUnSelectAll",

	NULL
};


/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/

static void view_menu_item_toggled_cb     (BonoboUIComponent           *ui_component,
					   const char                  *path,
					   Bonobo_UIComponent_EventType type,
					   const char                  *state,
					   BonoboWindow                *win);

static void set_app_main_toolbar_style 	  (BonoboUIComponent           *ui_component);

static void set_verb_sensitive            (BonoboUIComponent           *ui_component,
					   gchar                       *cname,
					   gboolean                     sensitive);

static void set_verb_list_sensitive       (BonoboUIComponent           *ui_component,
					   gchar                      **vlist,
					   gboolean                     sensitive);

static void set_verb_state                (BonoboUIComponent           *ui_component,
					   gchar                       *cname,
					   gboolean                     state);



/*****************************************************************************/
/* Initialize UI component for given window.                                 */
/*****************************************************************************/
void
gb_ui_init (BonoboUIComponent *ui_component,
	    BonoboWindow      *win)
{
        EggRecentView    *recent_view;
	EggRecentModel   *recent_model;

	gb_debug (DEBUG_UI, "START");

	gb_debug (DEBUG_UI, "window = %p", win);

	g_return_if_fail (ui_component != NULL);

	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (win),
					  "/gbonds/UIConfig/kvps");
	gb_debug (DEBUG_UI, "Path set");

	bonobo_ui_util_set_ui (ui_component,
			       "", GBONDS_UI_XML, "gbonds", NULL);
	gb_debug (DEBUG_UI, "UI set");

	bonobo_ui_component_add_verb_list_with_data(ui_component,
						    gb_ui_verbs, win);
	gb_debug (DEBUG_UI, "verb list added");

	/* Set the toolbar style according to prefs */
	set_app_main_toolbar_style (ui_component);
		
	/* Add listener for the view menu */
	bonobo_ui_component_add_listener (ui_component, "ViewMainToolbar", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);

	bonobo_ui_component_add_listener (ui_component, "MainToolbarSystem", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "MainToolbarIcon", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "MainToolbarIconText", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "MainToolbarTooltips", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);

	set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	/* add a GeditRecentView object */
        recent_model = gb_recent_get_model ();
        recent_view  =
		EGG_RECENT_VIEW (egg_recent_view_bonobo_new (ui_component,
							     "/menu/File/Recents"));
        egg_recent_view_set_model (recent_view, recent_model);
        
        g_signal_connect (G_OBJECT (recent_view), "activate",
                          G_CALLBACK (gb_file_open_recent), win);

	/* Squirrel away a copy to be unreferenced in gl_ui_unref() */
	g_object_set_data (G_OBJECT (ui_component), "recent-view", recent_view);
	gb_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Unref wrapper.                                                            */
/*****************************************************************************/
void
gb_ui_unref (BonoboUIComponent *ui_component)
{
	EggRecentView *recent_view;

	/* Pull out recent view to unreference. */
	recent_view = g_object_get_data (G_OBJECT(ui_component), "recent-view");
	if (recent_view) {
		g_object_unref (recent_view);
	}

	bonobo_object_unref(ui_component);
}

/*****************************************************************************/
/* Update all verbs of given UI component.                                   */
/*****************************************************************************/
void
gb_ui_update_all (BonoboUIComponent *ui_component,
		  gbView            *view)
{
	gbDoc *doc;

	gb_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_list_sensitive (ui_component, doc_verbs, TRUE);

	doc = view->doc;
	g_return_if_fail (doc != NULL);

	set_verb_sensitive (ui_component, "/commands/EditUndo",
			    gb_doc_can_undo (doc));
	set_verb_sensitive (ui_component, "/commands/EditRedo",
			    gb_doc_can_redo (doc));

	set_verb_list_sensitive (ui_component, 
				 doc_modified_verbs,
				 gb_doc_is_modified (doc));

	set_verb_list_sensitive (ui_component,
				 selection_verbs,
				 !gb_view_is_selection_empty (view));

	bonobo_ui_component_thaw (ui_component, NULL);

	gb_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update all verbs of given UI component to "no document" state.            */
/*****************************************************************************/
void
gb_ui_update_nodoc (BonoboUIComponent *ui_component)
{
	gb_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);
	
	set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	bonobo_ui_component_thaw (ui_component, NULL);

	gb_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update doc modified verbs of given UI component.                        */
/*****************************************************************************/
void
gb_ui_update_modified_verbs (BonoboUIComponent *ui_component,
			     gbDoc             *doc)
{
	gb_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_list_sensitive (ui_component, 
				 doc_modified_verbs,
				 gb_doc_is_modified (doc));

	bonobo_ui_component_thaw (ui_component, NULL);

	gb_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update verbs associated with selection state of given UI component.       */
/*****************************************************************************/
void
gb_ui_update_selection_verbs (BonoboUIComponent *ui_component,
			      gbView            *view)
{
	gb_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_list_sensitive (ui_component,
				 selection_verbs,
				 !gb_view_is_selection_empty (view));

	bonobo_ui_component_thaw (ui_component, NULL);

	gb_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update undo/redo verbs of given UI component.                             */
/*****************************************************************************/
void
gb_ui_update_undo_redo_verbs (BonoboUIComponent *ui_component,
			      gbDoc             *doc)
{
	gb_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_sensitive (ui_component,
			    "/commands/EditUndo", gb_doc_can_undo (doc));

	set_verb_sensitive (ui_component,
			    "/commands/EditRedo", gb_doc_can_redo (doc));

	bonobo_ui_component_thaw (ui_component, NULL);

	gb_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View menu item toggled callback.                                */
/*---------------------------------------------------------------------------*/
static void
view_menu_item_toggled_cb (BonoboUIComponent           *ui_component,
			   const char                  *path,
			   Bonobo_UIComponent_EventType type,
			   const char                  *state,
			   BonoboWindow                *win)
{
	gboolean s;

	gb_debug (DEBUG_UI, "");

	s = (strcmp (state, "1") == 0);

	if (strcmp (path, "ViewMainToolbar") == 0)
	{
		gb_prefs->main_toolbar_visible = s;
		set_app_main_toolbar_style (ui_component);
		gb_prefs_save_settings ();

		return;
	}

	if (s && (strcmp (path, "MainToolbarSystem") == 0))
	{		
		gb_prefs->main_toolbar_buttons_style = GB_TOOLBAR_SYSTEM;
		set_app_main_toolbar_style (ui_component);
		gb_prefs_save_settings ();

		return;
	}

	if (s && (strcmp (path, "MainToolbarIcon") == 0))
	{		
		gb_prefs->main_toolbar_buttons_style = GB_TOOLBAR_ICONS;
		set_app_main_toolbar_style (ui_component);
		gb_prefs_save_settings ();

		return;
	}

	if (s && (strcmp (path, "MainToolbarIconText") == 0))
	{		
		gb_prefs->main_toolbar_buttons_style = GB_TOOLBAR_ICONS_AND_TEXT;
		set_app_main_toolbar_style (ui_component);
		gb_prefs_save_settings ();

		return;
	}

	if (strcmp (path, "MainToolbarTooltips") == 0)
	{
		gb_prefs->main_toolbar_view_tooltips = s;
		set_app_main_toolbar_style (ui_component);
		gb_prefs_save_settings ();

		return;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set main toolbar style.                                         */
/*---------------------------------------------------------------------------*/
static void
set_app_main_toolbar_style (BonoboUIComponent *ui_component)
{
	GConfClient *client;
	gboolean     labels;

	gb_debug (DEBUG_UI, "START");

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));
			
	bonobo_ui_component_freeze (ui_component, NULL);

	/* Updated view menu */
	set_verb_state (ui_component, 
			"/commands/ViewMainToolbar",
			gb_prefs->main_toolbar_visible);

	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarSystem",
			    gb_prefs->main_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarIcon",
			    gb_prefs->main_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarIconText",
			    gb_prefs->main_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarTooltips",
			    gb_prefs->main_toolbar_visible);

	set_verb_state (ui_component, 
			"/commands/MainToolbarSystem",
			gb_prefs->main_toolbar_buttons_style == GB_TOOLBAR_SYSTEM);

	set_verb_state (ui_component, 
			"/commands/MainToolbarIcon",
			gb_prefs->main_toolbar_buttons_style == GB_TOOLBAR_ICONS);

	set_verb_state (ui_component, 
			"/commands/MainToolbarIconText",
			gb_prefs->main_toolbar_buttons_style == GB_TOOLBAR_ICONS_AND_TEXT);

	set_verb_state (ui_component, 
			"/commands/MainToolbarTooltips",
			gb_prefs->main_toolbar_view_tooltips);

	
	/* Actually update main_toolbar style */
	bonobo_ui_component_set_prop (
		ui_component, "/MainToolbar",
		"tips", gb_prefs->main_toolbar_view_tooltips ? "1" : "0",
		NULL);
	
	switch (gb_prefs->main_toolbar_buttons_style)
	{
		case GB_TOOLBAR_SYSTEM:
						
			client = gconf_client_get_default ();
			if (client == NULL) 
				goto error;

			labels = gconf_client_get_bool (client, 
					"/desktop/gnome/interface/toolbar-labels", NULL);

			g_object_unref (G_OBJECT (client));
			
			if (labels)
			{			
				bonobo_ui_component_set_prop (
					ui_component, "/MainToolbar", "look", "both", NULL);
			
			}
			else
			{
				bonobo_ui_component_set_prop (
					ui_component, "/MainToolbar", "look", "icons", NULL);
			}
	
			break;
			
		case GB_TOOLBAR_ICONS:
			bonobo_ui_component_set_prop (
				ui_component, "/MainToolbar", "look", "icon", NULL);
			
			break;
			
		case GB_TOOLBAR_ICONS_AND_TEXT:
			bonobo_ui_component_set_prop (
				ui_component, "/MainToolbar", "look", "both", NULL);
			
			break;
		default:
			goto error;
			break;
	}
	
	bonobo_ui_component_set_prop (
			ui_component, "/MainToolbar",
			"hidden", gb_prefs->main_toolbar_visible ? "0":"1", NULL);

 error:
	bonobo_ui_component_thaw (ui_component, NULL);

	gb_debug (DEBUG_UI, "END");
}


/*---------------------------------------------------------------------------*/
/* Set sensitivity of verb.                                                  */
/*---------------------------------------------------------------------------*/
static void
set_verb_sensitive (BonoboUIComponent  *ui_component,
		    gchar              *cname,
		    gboolean            sensitive)
{
	gb_debug (DEBUG_UI, "START");

	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (ui_component,
				      cname,
				      "sensitive",
				      sensitive ? "1" : "0",
				      NULL);

	gb_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* Set sensitivity of a list of verbs.                                       */
/*---------------------------------------------------------------------------*/
static void
set_verb_list_sensitive (BonoboUIComponent   *ui_component,
			 gchar              **vlist,
			 gboolean             sensitive)
{
	gb_debug (DEBUG_UI, "START");

	g_return_if_fail (vlist != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	for ( ; *vlist; ++vlist)
	{
		bonobo_ui_component_set_prop (ui_component,
					      *vlist,
					      "sensitive",
					      sensitive ? "1" : "0",
					      NULL);
	}

	gb_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* Set state of a verb.                                                      */
/*---------------------------------------------------------------------------*/
static void
set_verb_state (BonoboUIComponent   *ui_component,
		gchar               *cname,
		gboolean             state)
{
	gb_debug (DEBUG_UI, "START");

	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (ui_component,
				      cname,
				      "state",
				      state ? "1" : "0",
				      NULL);

	gb_debug (DEBUG_UI, "END");
}

