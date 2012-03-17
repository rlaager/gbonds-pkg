/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  commands.c:  gbonds commands module
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

#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include "commands.h"
#include "view.h"
#include "file.h"
#include "edit.h"
#include "update.h"
#include "print-dialog.h"
#include "prefs-dialog.h"
#include "debug.h"

#define LOGO_PIXMAP gnome_program_locate_file (NULL,\
					 GNOME_FILE_DOMAIN_APP_PIXMAP,\
					 "gbonds/gbonds_logo.png",\
					 FALSE, NULL)


/****************************************************************************/
/* File->New command.                                                       */
/****************************************************************************/
void 
gb_cmd_file_new (BonoboUIComponent *uic,
		 gpointer           user_data,
		 const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");
	
	gb_file_new (GTK_WINDOW(window));
}

/****************************************************************************/
/* File->Open command.                                                      */
/****************************************************************************/
void 
gb_cmd_file_open (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_file_open (GTK_WINDOW(window));
}

/****************************************************************************/
/* File->Import command.                                                    */
/****************************************************************************/
void 
gb_cmd_file_import (BonoboUIComponent *uic,
		    gpointer           user_data,
		    const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_file_import (GTK_WINDOW(window));
}

/****************************************************************************/
/* File->Save command.                                                      */
/****************************************************************************/
void 
gb_cmd_file_save (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_file_save (GB_VIEW(window->view)->doc, GTK_WINDOW(window));
}

/****************************************************************************/
/* File->Save_as command.                                                   */
/****************************************************************************/
void 
gb_cmd_file_save_as (BonoboUIComponent *uic,
		     gpointer           user_data,
		     const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_file_save_as (GB_VIEW(window->view)->doc, GTK_WINDOW(window));
}

/****************************************************************************/
/* File->Print command.                                                     */
/****************************************************************************/
void
gb_cmd_file_print (BonoboUIComponent *uic,
		   gpointer           user_data,
		   const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_print_dialog (GB_VIEW(window->view), BONOBO_WINDOW(window));

}

/****************************************************************************/
/* File->Close command.                                                     */
/****************************************************************************/
void 
gb_cmd_file_close (BonoboUIComponent *uic,
		   gpointer           user_data,
		   const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_file_close (window);
}

/****************************************************************************/
/* File->Exit command.                                                      */
/****************************************************************************/
void 
gb_cmd_file_exit (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_file_exit ();
}


/****************************************************************************/
/* Edit->Cut command.                                                       */
/****************************************************************************/
void 
gb_cmd_edit_cut (BonoboUIComponent *uic,
		 gpointer           user_data,
		 const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_view_cut (GB_VIEW(window->view)); 
}

/****************************************************************************/
/* Edit->Copy command.                                                      */
/****************************************************************************/
void 
gb_cmd_edit_copy (BonoboUIComponent *uic,
		  gpointer           user_data,
		  const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_view_copy (GB_VIEW(window->view)); 
}

/****************************************************************************/
/* Edit->Paste command.                                                     */
/****************************************************************************/
void 
gb_cmd_edit_paste (BonoboUIComponent *uic,
		   gpointer           user_data,
		   const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_view_paste (GB_VIEW(window->view)); 
}


/****************************************************************************/
/* Edit->Add command.                                                       */
/****************************************************************************/
void 
gb_cmd_edit_add (BonoboUIComponent *uic,
		 gpointer           user_data,
		 const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_edit_add_bond (GB_VIEW(window->view), BONOBO_WINDOW(window)); 
}

/****************************************************************************/
/* Edit->Delete command.                                                    */
/****************************************************************************/
void 
gb_cmd_edit_delete (BonoboUIComponent *uic,
		    gpointer           user_data,
		    const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_edit_delete_bonds (GB_VIEW(window->view), BONOBO_WINDOW(window)); 
}

/****************************************************************************/
/* Edit->Title command.                                                     */
/****************************************************************************/
void 
gb_cmd_edit_title (BonoboUIComponent *uic,
		   gpointer           user_data,
		   const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_edit_title (GB_VIEW(window->view), BONOBO_WINDOW(window)); 
}

/****************************************************************************/
/* Edit->Select_all command.                                                */
/****************************************************************************/
void
gb_cmd_edit_select_all (BonoboUIComponent *uic,
			gpointer           user_data,
			const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_view_select_all (GB_VIEW(window->view)); 
}

/****************************************************************************/
/* Edit->Select_all command.                                                */
/****************************************************************************/
void
gb_cmd_edit_unselect_all (BonoboUIComponent *uic,
			  gpointer           user_data,
			  const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_view_unselect_all (GB_VIEW(window->view)); 
}

/****************************************************************************/
/* Settings->Preferences command.                                           */
/****************************************************************************/
void
gb_cmd_settings_preferences (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);
	static GtkWidget *dlg = NULL;

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	if (dlg != NULL)
	{
		gtk_window_present (GTK_WINDOW (dlg));
		gtk_window_set_transient_for (GTK_WINDOW (dlg),	
					      GTK_WINDOW(window));

		return;
	}
		
	dlg = gb_prefs_dialog_new (GTK_WINDOW(window));

	g_signal_connect (G_OBJECT (dlg), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &dlg);
	
	gtk_widget_show (dlg);
}

/****************************************************************************/
/* Settings->Update command.                                                */
/****************************************************************************/
void
gb_cmd_settings_update (BonoboUIComponent *uic,
			gpointer           user_data,
			const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gb_update_druid ();

}

/****************************************************************************/
/* Help->Contents command.                                                  */
/****************************************************************************/
void 
gb_cmd_help_contents (BonoboUIComponent *uic,
		      gpointer           user_data,
		      const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);
	GError *error = NULL;

	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	gnome_help_display_with_doc_id (NULL, NULL, "gbonds.xml", NULL, &error);
	
	if (error != NULL)
	{
		g_warning (error->message);

		g_error_free (error);
	}
}

/****************************************************************************/
/* Help->About command.                                                     */
/****************************************************************************/
void 
gb_cmd_help_about (BonoboUIComponent *uic,
		   gpointer           user_data,
		   const gchar       *verbname)
{
	gbWindow *window = GB_WINDOW (user_data);
	static GtkWidget *about = NULL;
	GdkPixbuf        *pixbuf = NULL;
	
	gchar *copy_text = "Copyright 2001-2003 Jim Evins";
	gchar *about_text =
		_("A savings bond inventory program for GNOME.\n"
		  " \n"
		  "Gbonds is free software; you can redistribute it and/or modify it "
		  "under the terms of the GNU General Public License as published by "
		  "the Free Software Foundation; either version 2 of the License, or "
		  "(at your option) any later version.\n" " \n"
		  "This program is distributed in the hope that it will be useful, but "
		  "WITHOUT ANY WARRANTY; without even the implied warranty of "
		  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
		  "General Public License for more details.\n"
		  " \n"
		  "DISCLAIMER:  GBonds is not affiliated with or endorsed by the "
		  "U.S. Treasury or the U.S. Government.");

	gchar *authors[] = {
		"Jim Evins <evins@snaught.com>",
		" ",
		_("See the file AUTHORS for additional credits,"),
		_("or visit http://snaught.com/gbonds"),
		NULL
	};
	
	gb_debug (DEBUG_COMMANDS, "");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	if (about != NULL)
	{
		gdk_window_show (about->window);
		gdk_window_raise (about->window);
		return;
	}
	
	pixbuf = gdk_pixbuf_new_from_file ( LOGO_PIXMAP, NULL);

	about = gnome_about_new (_("gbonds"), VERSION,
				 copy_text,
				 about_text,
				(const char **)authors,
				(const char **)NULL,
				(const char *)NULL,
				pixbuf);

	gtk_window_set_transient_for (GTK_WINDOW (about),
				      GTK_WINDOW (window));

	gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);

	if (pixbuf != NULL)
		g_object_unref (pixbuf);
	
	g_signal_connect (G_OBJECT (about), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &about);
	
	gtk_widget_show (about);
}


