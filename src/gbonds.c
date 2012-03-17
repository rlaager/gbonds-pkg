
/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  gbonds.c: main program module
 *
 *  Copyright (C) 2000-2003  Jim Evins <evins@snaught.com>.
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

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomeui/gnome-window-icon.h>

#include "splash.h"
#include "prefs.h"
#include "recent.h"
#include "table.h"
#include "hig.h"
#include "window.h"
#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define ICON_PIXMAP gnome_program_locate_file (NULL,\
					       GNOME_FILE_DOMAIN_APP_PIXMAP,\
					       "gbonds.png",\
					       FALSE, NULL)

/*========================================================*/
/* Private globals                                        */
/*========================================================*/

/*========================================================*/
/* Local function prototypes                              */
/*========================================================*/
gint save_session_cb (GnomeClient        *client,
                      gint                phase,
                      GnomeRestartStyle   save_style,
                      gint                shutdown,
                      GnomeInteractStyle  interact_style,
                      gint                fast,
                      gpointer            client_data);

void client_die_cb   (GnomeClient        *client,
                      gpointer            client_data);


/****************************************************************************/
/* main program                                                             */
/****************************************************************************/
int
main (int argc, char **argv)
{
        GnomeProgram  *program;
	GnomeClient   *client;
        GValue         value = { 0, };
	poptContext    ctx;
        char         **args;
        GList         *file_list = NULL, *p;
	gint           i;
	GtkWidget     *win;
	gchar	      *utf8_filename;
	GList         *dates;
	gbTableModel  *table_model;

	bindtextdomain (GETTEXT_PACKAGE, GBONDS_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        /* Initialize gnome program */
        program = gnome_program_init ("gbonds", VERSION,
                                      LIBGNOMEUI_MODULE, argc, argv,
                                      GNOME_PROGRAM_STANDARD_PROPERTIES,
                                      NULL);

	/* Splash screen */
        gb_splash ();

	/* Set default icon */
	if (!g_file_test (ICON_PIXMAP, G_FILE_TEST_EXISTS))
	{
		g_warning ("Could not find %s", ICON_PIXMAP);
	}
	else
	{
		gnome_window_icon_set_default_from_file (ICON_PIXMAP);
	}
	
        if (bonobo_ui_init ("gbonds", VERSION, &argc, argv) == FALSE) {
                g_error (_("Could not initialize Bonobo!\n"));
        }

        /* Load user preferences */
	gb_debug_init ();
        gb_prefs_init ();
        gb_prefs_load_settings ();
	gb_recent_init ();
	gb_table_init ();

	/* Handle session management */
	client = gnome_master_client();
        g_signal_connect (G_OBJECT (client), "save_yourself",
                          G_CALLBACK (save_session_cb),
                          (gpointer)argv[0]);
        g_signal_connect (G_OBJECT (client), "die",
                          G_CALLBACK (client_die_cb), NULL);

	/* Make sure we are configured with some redemption data */
	table_model = gb_table_get_model ();
	dates = gb_table_model_get_rdate_list (table_model, 1);
	if (!dates) {
		GtkWidget *alert;

		alert = gb_hig_alert_new (NULL,
					  GTK_DIALOG_MODAL,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_OK,
					  _("Missing Redemption Data!"),
					  _("Could not find any valid redemption data.  "
					    "Is gbonds installed?"));
		gtk_dialog_run (GTK_DIALOG(alert));
		return -1;
	}
	gb_table_model_free_rdate_list (dates);

	/* Parse args and build the list of files to be loaded at startup */
        g_value_init (&value, G_TYPE_POINTER);
        g_object_get_property (G_OBJECT (program),
                               GNOME_PARAM_POPT_CONTEXT, &value);
        ctx = g_value_get_pointer (&value);
        g_value_unset (&value);
        args = (char**) poptGetArgs(ctx);
        for (i = 0; args && args[i]; i++)
        {
		utf8_filename = g_filename_to_utf8 (args[i], -1, NULL, NULL, NULL);
		if (utf8_filename)
			file_list = g_list_append (file_list, utf8_filename);
        }

	/* Any files on command line? */
	for (p = file_list; p; p = p->next) {
		win = gb_window_new_from_file (p->data);
		if ( win != NULL ) {
			gtk_widget_show_all (win);
		}
	}

	/* If not, consult preferences for how to start up */
	if ( gb_window_get_window_list() == NULL ) {

		win = gb_window_new ();
		switch (gb_prefs->startup_action) {

		case GB_PREFS_STARTUP_BLANK:
			gb_file_new (win);
			break;

		case GB_PREFS_STARTUP_RECENT:
		{
			EggRecentModel *recent_model;
			GList          *recent_list;

			recent_model = gb_recent_get_model ();
			recent_list = egg_recent_model_get_list (recent_model);
			if ( recent_list != NULL ) {
				if ( !gb_file_open_real (recent_list->data, win) ) {
					gb_file_new (win);
				}
			}
		}
		break;

		case GB_PREFS_STARTUP_DEFAULT:
			if ( !gb_file_open_real (gb_prefs->startup_file, win) ) {
				gb_file_new (win);
			}
			break;

		default:
			g_warning ("Unknown startup action\n");
			break;

		}

		gtk_widget_show_all (win);

	}

	g_list_free (file_list);

	/* If our redemption data is about to expire, or has expired, help the user
	   download new data. */
	if (gb_table_model_get_rdate_today()>=gb_table_model_get_rdate_max(table_model)) {
		gb_update_druid();
	}


        /* Begin main loop */
        bonobo_main ();

        return 0;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save session" callback.                                        */
/*---------------------------------------------------------------------------*/
gint save_session_cb (GnomeClient        *client,
                      gint                phase,
                      GnomeRestartStyle   save_style,
                      gint                shutdown,
                      GnomeInteractStyle  interact_style,
                      gint                fast,
                      gpointer            client_data)
{
        gchar       *argv[128];
        gint         argc;
        const GList *window_list;
        GList       *p;
        gbWindow    *window;
        gbDoc       *doc;

        argv[0] = (gchar *)client_data;
        argc = 1;

        window_list = gb_window_get_window_list();
        for ( p=(GList *)window_list; p != NULL; p=p->next ) {
                window = GB_WINDOW(p->data);
                if ( !gb_window_is_empty (window) ) {
                        doc = GB_VIEW(window->view)->doc;
                        argv[argc++] = gb_doc_get_filename (doc);
                }
        }
        gnome_client_set_clone_command(client, argc, argv);
        gnome_client_set_restart_command(client, argc, argv);

        return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Die" callback.                                                 */
/*---------------------------------------------------------------------------*/
void client_die_cb (GnomeClient *client,
                    gpointer     client_data)
{
        gb_file_exit ();
}

