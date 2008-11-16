/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  file.c:  FILE menu dialog module
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

#include <gnome.h>
#include <string.h>

#include "doc-xml.h"
#include "doc-sbw.h"
#include "file.h"
#include "recent.h"
#include "hig.h"
#include "util.h"
#include "libbonobo.h"

#include "debug.h"

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/* Saved state of file selectors */
static gchar *open_path   = NULL;
static gchar *import_path = NULL;
static gchar *save_path   = NULL;

/*===========================================*/
/* Local function prototypes.                */
/*===========================================*/
#ifdef HAVE_FILE_CHOOSER
static void open_response             (GtkDialog         *chooser,
				       gint               response,
				       GtkWindow         *window);
static void import_response           (GtkDialog         *chooser,
				       gint               response,
				       GtkWindow         *window);
static void save_as_response          (GtkDialog         *chooser,
				       gint               response,
				       gbDoc             *doc);
#else
static void open_ok                   (GtkWidget         *widget,
				       GtkFileSelection  *fsel);

static void import_ok                 (GtkWidget         *widget,
				       GtkFileSelection  *fsel);
static void save_as_ok_cb             (GtkWidget         *widget,
				       GtkFileSelection  *fsel);
static void save_as_cancel_cb         (GtkWidget         *widget,
				       GtkFileSelection  *fsel);
static void save_as_destroy_cb        (GtkWidget         *widget,
				       gboolean          *destroy_flag);
#endif

gboolean    import_real               (const gchar       *filename,
				       GtkWindow         *window);



/*****************************************************************************/
/* "New" menu callback.                                                      */
/*****************************************************************************/
void
gb_file_new (GtkWindow *window)
{
	gbDoc     *doc;
	GtkWidget *new_window;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (window && GB_IS_WINDOW(window));

	doc = GB_DOC(gb_doc_new ());

	if ( gb_window_is_empty (GB_WINDOW(window)) ) {
		gb_window_set_doc (GB_WINDOW(window), doc);
	} else {
		new_window = gb_window_new_from_doc (doc);
		gtk_widget_show_all (new_window);
	}

	g_object_unref (doc);

	gb_debug (DEBUG_FILE, "END");
}

#ifdef HAVE_FILE_CHOOSER

/*****************************************************************************/
/* "Open" menu callback.                                                     */
/*****************************************************************************/
void
gb_file_open (GtkWindow *window)
{
	GtkWidget     *chooser;
	GtkFileFilter *filter;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	chooser = gtk_file_chooser_dialog_new ("Open inventory",
					       window,
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       NULL);

	/* Recover state of open dialog */
	if (open_path != NULL) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
						     open_path);
	}

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.gbonds");
	gtk_file_filter_set_name (filter, _("GBonds documents"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

	g_signal_connect (G_OBJECT (chooser), "response",
			  G_CALLBACK (open_response), window);

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (chooser));

	gb_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open "response" callback.                                       */
/*---------------------------------------------------------------------------*/
static void
open_response (GtkDialog     *chooser,
	       gint           response,
	       GtkWindow     *window)
{
	gchar            *raw_filename;
	gchar 		 *filename;
	GtkWidget        *dlg;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (chooser && GTK_IS_FILE_CHOOSER (chooser));
	g_return_if_fail (window && GTK_IS_WINDOW (window));

	switch (response) {

	case GTK_RESPONSE_ACCEPT:
		/* get the filename */
		raw_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));
		filename = g_filename_to_utf8 (raw_filename, -1, NULL, NULL, NULL);

		if (!raw_filename || 
		    !filename || 
		    g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

			dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);

		} else {

			if (!g_file_test (raw_filename, G_FILE_TEST_IS_REGULAR)) {

				dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_CLOSE,
						_("File does not exist"),
						_("Please select a file or supply a valid file name"));

				gtk_dialog_run (GTK_DIALOG (dlg));
				gtk_widget_destroy (dlg);


			} else {
		
				if ( gb_file_open_real (filename, window) ) {
					gtk_widget_destroy (GTK_WIDGET (chooser));
				}

			}

		}

		g_free (filename);
		g_free (raw_filename);
		break;

	default:
		gtk_widget_destroy (GTK_WIDGET (chooser));
		break;

	}

	gb_debug (DEBUG_FILE, "END");
}

#else

/*****************************************************************************/
/* "Open" menu callback.                                                     */
/*****************************************************************************/
void
gb_file_open (GtkWindow *window)
{
	GtkFileSelection *fsel;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Open")));
	gtk_window_set_transient_for (GTK_WINDOW (fsel), window);
	gtk_window_set_title (GTK_WINDOW (fsel), _("Open inventory"));

	g_object_set_data (G_OBJECT (fsel), "parent_window", window);

	g_signal_connect (G_OBJECT (fsel->ok_button), "clicked",
			  G_CALLBACK (open_ok), fsel);

	g_signal_connect_swapped (G_OBJECT (fsel->cancel_button), "clicked",
				  G_CALLBACK (gtk_widget_destroy),
				  G_OBJECT (fsel));

	/* Recover state of open dialog */
	if (open_path != NULL) {
		gtk_file_selection_set_filename (fsel, open_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

	gb_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open "O.K." button callback.                                    */
/*---------------------------------------------------------------------------*/
static void
open_ok (GtkWidget        *widget,
	 GtkFileSelection *fsel)
{
	gchar            *filename;
	GtkWidget        *dlg;
	gint              ret;
	EggRecentModel   *recent;
	GtkWindow        *window;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	/* get the filename */
	filename = g_strdup (gtk_file_selection_get_filename (fsel));

	if (!filename || g_file_test (filename, G_FILE_TEST_IS_DIR)) {

		dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please select a file or supply a valid file name"));

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

	} else {

		if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {

			dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_CLOSE,
						_("File does not exist"),
						_("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);


		} else {
		
			window = g_object_get_data (G_OBJECT(fsel),
						    "parent_window");

			if ( gb_file_open_real (filename, window) ) {
				gtk_widget_destroy (GTK_WIDGET (fsel));
			}

		}

	}

	g_free (filename);

	gb_debug (DEBUG_FILE, "END");
}

#endif

/*****************************************************************************/
/* "Open recent" menu callback.                                              */
/*****************************************************************************/
gboolean
gb_file_open_recent (EggRecentView   *view,
		     EggRecentItem   *item,
		     GtkWindow       *window)
{
	gboolean result = FALSE;
	gchar *filename;
	
	gb_debug (DEBUG_FILE, "");

	filename = gb_recent_get_filename (item);

	if (filename) {
		gb_debug (DEBUG_FILE, "open recent: %s", filename);

		result = gb_file_open_real (filename, window);
		g_free (filename);
	}

	return result;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open a file.                                                    */
/*---------------------------------------------------------------------------*/
gboolean
gb_file_open_real (const gchar     *filename,
		   GtkWindow       *window)
{
	gchar            *abs_filename;
	gbDoc            *doc;
	gbDocXMLStatus    status;
	GtkWidget        *new_window;

	gb_debug (DEBUG_FILE, "START");

	abs_filename = gb_util_make_absolute (filename);
	doc = gb_doc_xml_open (abs_filename, &status);
	if (!doc) {
		GtkWidget *dlg;
		gchar *primary_msg;

		gb_debug (DEBUG_FILE, "couldn't open file");

		primary_msg = g_strdup_printf (_("Could not open file \"%s\""),
					       filename);

		dlg = gb_hig_alert_new (window,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					primary_msg,
					_("Not a supported file format"));

		g_free (primary_msg);

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

		g_free (abs_filename);

		gb_debug (DEBUG_FILE, "END false");

		return FALSE;

	} else {

		if ( gb_window_is_empty (GB_WINDOW(window)) ) {
			gb_window_set_doc (GB_WINDOW(window), doc);
		} else {
			new_window = gb_window_new_from_doc (doc);
			gtk_widget_show_all (new_window);
		}

		gb_recent_add_uri (abs_filename);

		if (open_path != NULL)
			g_free (open_path);
		open_path = g_path_get_dirname (abs_filename);
#ifndef HAVE_FILE_CHOOSER
		if (open_path != NULL)
			open_path = g_strconcat (open_path, "/", NULL);
#endif

		g_free (abs_filename);

		gb_debug (DEBUG_FILE, "END true");

		return TRUE;

	}
}

#ifdef HAVE_FILE_CHOOSER

/*****************************************************************************/
/* "Import" menu callback.                                                   */
/*****************************************************************************/
void
gb_file_import (GtkWindow *window)
{
	GtkWidget     *chooser;
	GtkFileFilter *filter;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	chooser = gtk_file_chooser_dialog_new ("Import inventory",
					       window,
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       NULL);

	/* Recover state of import dialog */
	if (import_path != NULL) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
						     import_path);
	}

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.sbw");
	gtk_file_filter_set_name (filter, _("SBW documents"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

	g_signal_connect (G_OBJECT (chooser), "response",
			  G_CALLBACK (import_response), window);

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (chooser));

	gb_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Import "response" callback.                                     */
/*---------------------------------------------------------------------------*/
static void
import_response (GtkDialog     *chooser,
		 gint           response,
		 GtkWindow     *window)
{
	gchar            *raw_filename;
	gchar 		 *filename;
	GtkWidget        *dlg;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (chooser && GTK_IS_FILE_CHOOSER (chooser));
	g_return_if_fail (window && GTK_IS_WINDOW (window));

	switch (response) {

	case GTK_RESPONSE_ACCEPT:
		/* get the filename */
		raw_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));
		filename = g_filename_to_utf8 (raw_filename, -1, NULL, NULL, NULL);

		if (!raw_filename || 
		    !filename || 
		    g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

			dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);

		} else {

			if (!g_file_test (raw_filename, G_FILE_TEST_IS_REGULAR)) {

				dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_CLOSE,
						_("File does not exist"),
						_("Please select a file or supply a valid file name"));

				gtk_dialog_run (GTK_DIALOG (dlg));
				gtk_widget_destroy (dlg);


			} else {
		
				if ( import_real (filename, window) ) {
					gtk_widget_destroy (GTK_WIDGET (chooser));
				}

			}

		}

		g_free (filename);
		g_free (raw_filename);
		break;

	default:
		gtk_widget_destroy (GTK_WIDGET (chooser));
		break;

	}

	gb_debug (DEBUG_FILE, "END");
}

#else

/*****************************************************************************/
/* "Import" menu callback.                                                   */
/*****************************************************************************/
void
gb_file_import (GtkWindow *window)
{
	GtkFileSelection *fsel;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Import")));
	gtk_window_set_transient_for (GTK_WINDOW (fsel), window);
	gtk_window_set_title (GTK_WINDOW (fsel), _("Import inventory"));

	g_object_set_data (G_OBJECT (fsel), "parent_window", window);

	g_signal_connect (G_OBJECT (fsel->ok_button), "clicked",
			  G_CALLBACK (import_ok), fsel);

	g_signal_connect_swapped (G_OBJECT (fsel->cancel_button), "clicked",
				  G_CALLBACK (gtk_widget_destroy),
				  G_OBJECT (fsel));

	/* Recover state of import dialog */
	if (import_path != NULL) {
		gtk_file_selection_set_filename (fsel, import_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

	gb_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Import "O.K." button callback.                                  */
/*---------------------------------------------------------------------------*/
static void
import_ok (GtkWidget        *widget,
	   GtkFileSelection *fsel)
{
	gchar            *filename;
	GtkWidget        *dlg;
	gint              ret;
	EggRecentModel   *recent;
	GtkWindow        *window;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	/* get the filename */
	filename = g_strdup (gtk_file_selection_get_filename (fsel));

	if (!filename || g_file_test (filename, G_FILE_TEST_IS_DIR)) {

		dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please select a file or supply a valid file name"));

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

	} else {

		if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {

			dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_CLOSE,
						_("File does not exist"),
						_("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);


		} else {
		
			window = g_object_get_data (G_OBJECT(fsel),
						    "parent_window");

			if ( import_real (filename, window) ) {
				gtk_widget_destroy (GTK_WIDGET (fsel));
			}

		}

	}

	g_free (filename);

	gb_debug (DEBUG_FILE, "END");
}

#endif

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Import a file.                                                  */
/*---------------------------------------------------------------------------*/
gboolean
import_real (const gchar     *filename,
	     GtkWindow       *window)
{
	gchar            *abs_filename;
	gbDoc            *doc;
	gbStatus          status;
	GtkWidget        *new_window;

	gb_debug (DEBUG_FILE, "START");

	abs_filename = gb_util_make_absolute (filename);
	doc = gb_doc_sbw_open (abs_filename, &status);
	if (!doc) {
		GtkWidget *dlg;
		gchar *primary_msg;

		gb_debug (DEBUG_FILE, "couldn't open file");

		primary_msg = g_strdup_printf (_("Could not open file \"%s\""),
					       filename);

		dlg = gb_hig_alert_new (window,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					primary_msg,
					_("Not a supported file format"));

		g_free (primary_msg);

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

		g_free (abs_filename);

		gb_debug (DEBUG_FILE, "END false");

		return FALSE;

	} else {

		if ( gb_window_is_empty (GB_WINDOW(window)) ) {
			gb_window_set_doc (GB_WINDOW(window), doc);
		} else {
			new_window = gb_window_new_from_doc (doc);
			gtk_widget_show_all (new_window);
		}

		if (import_path != NULL)
			g_free (import_path);
		import_path = g_path_get_dirname (abs_filename);
#ifndef HAVE_FILE_CHOOSER
		if (import_path != NULL)
			import_path = g_strconcat (import_path, "/", NULL);
#endif
		g_free (abs_filename);

		gb_debug (DEBUG_FILE, "END true");

		return TRUE;

	}
}

/*****************************************************************************/
/* "Save" menu callback.                                                     */
/*****************************************************************************/
gboolean
gb_file_save (gbDoc     *doc,
	      GtkWindow *window)
{
	gbDocXMLStatus    status;
	gchar            *filename = NULL;

	gb_debug (DEBUG_FILE, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	
	if (gb_doc_is_untitled (doc))
	{
		gb_debug (DEBUG_FILE, "Untitled");

		return gb_file_save_as (doc, window);
	}

	if (!gb_doc_is_modified (doc))	
	{
		gb_debug (DEBUG_FILE, "Not modified");

		return TRUE;
	}
	
	filename = gb_doc_get_filename (doc);
	g_return_val_if_fail (filename != NULL, FALSE);
	
	gb_doc_xml_save (doc, filename, &status);

	if (status != GB_DOC_XML_OK)
	{
		GtkWidget *dialog;
		gchar *primary_msg;

		gb_debug (DEBUG_FILE, "FAILED");

		primary_msg = g_strdup_printf (_("Could not save file \"%s\""),
					       filename);

		dialog = gb_hig_alert_new (window,
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_MESSAGE_ERROR,
					   GTK_BUTTONS_CLOSE,
					   primary_msg,
					   _("Error encountered during save.  The file is still not saved."));

		g_free (primary_msg);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		g_free (filename);

		return FALSE;
	}	
	else
	{
		gb_debug (DEBUG_FILE, "OK");

		gb_recent_add_uri (filename);

		g_free (filename);

		return TRUE;
	}
}

#ifdef HAVE_FILE_CHOOSER

/*****************************************************************************/
/* "Save As" menu callback.                                                  */
/*****************************************************************************/
gboolean
gb_file_save_as (gbDoc     *doc,
		 GtkWindow *window)
{
	GtkWidget        *chooser;
	GtkFileFilter    *filter;
	gboolean          saved_flag = FALSE;
	gchar            *name, *title;

	gb_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (doc && GB_IS_DOC(doc), FALSE);
	g_return_val_if_fail (window && GTK_IS_WINDOW(window), FALSE);

	name = gb_doc_get_short_name (doc);
	title = g_strdup_printf (_("Save \"%s\" as"), name);
	g_free (name);

	chooser = gtk_file_chooser_dialog_new (title,
					       window,
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       NULL);

	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);

	g_free (title);

	/* Recover proper state of save-as dialog */
	if (save_path != NULL) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
						     save_path);
	}

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.gbonds");
	gtk_file_filter_set_name (filter, _("GBonds documents"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

	g_signal_connect (G_OBJECT (chooser), "response",
			  G_CALLBACK (save_as_response), doc);

	g_object_set_data (G_OBJECT (chooser), "saved_flag", &saved_flag);

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (chooser));

	/* Hold here and process events until we are done with this dialog. */
	/* This is so we can return a boolean result of our save attempt.   */
	gtk_main ();

	gb_debug (DEBUG_FILE, "END");

	/* Return flag as set by one of the above callbacks, TRUE = saved */
	return saved_flag;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" ok button callback.                                   */
/*---------------------------------------------------------------------------*/
static void
save_as_response (GtkDialog     *chooser,
		  gint           response,
		  gbDoc         *doc)
{
	gchar            *raw_filename, *filename, *full_filename;
	GtkWidget        *dlg;
	gbDocXMLStatus    status;
	gboolean         *saved_flag;
	gchar            *primary_msg;
	gboolean          cancel_flag = FALSE;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_CHOOSER (chooser));

	saved_flag = g_object_get_data (G_OBJECT(chooser), "saved_flag");

	switch (response) {

	case GTK_RESPONSE_ACCEPT:
		/* get the filename */
		raw_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));

		gb_debug (DEBUG_FILE, "raw_filename = \"%s\"", raw_filename);

		if (!raw_filename || g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

			dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);

		} else {

			full_filename = gb_util_add_extension (raw_filename);

			filename = g_filename_to_utf8 (full_filename, -1,
						       NULL, NULL, NULL);

			gb_debug (DEBUG_FILE, "filename = \"%s\"", filename);

			if (g_file_test (full_filename, G_FILE_TEST_IS_REGULAR)) {
				gint ret;

				primary_msg = g_strdup_printf (_("Overwrite file \"%s\"?"),
							       filename);

				dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						primary_msg,
						_("File already exists."));
			
				g_free (primary_msg);

				ret = gtk_dialog_run (GTK_DIALOG (dlg));
				if ( ret == GTK_RESPONSE_NO ) {
					cancel_flag = TRUE;
				}
				gtk_widget_destroy (dlg);
			}

			if (!cancel_flag) {

				gb_doc_xml_save (doc, filename, &status);

				gb_debug (DEBUG_FILE, "status of save = %d", status);

				if ( status != GB_DOC_XML_OK ) {

					primary_msg = g_strdup_printf (_("Could not save file \"%s\""),
								       filename);

					dlg = gb_hig_alert_new (GTK_WINDOW(chooser),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_CLOSE,
							primary_msg,
							_("Error encountered during save.  The file is still not saved."));

					g_free (primary_msg);

					gtk_dialog_run (GTK_DIALOG (dlg));
					gtk_widget_destroy (dlg);

				} else {

					*saved_flag = TRUE;

					gb_recent_add_uri (filename);

					if (save_path != NULL)
						g_free (save_path);
					save_path = g_path_get_dirname (filename);

					gtk_widget_destroy (GTK_WIDGET (chooser));
					gtk_main_quit ();
				}

			}

			g_free (filename);
			g_free (full_filename);
		}

		g_free (raw_filename);
		break;

	default:
		*saved_flag = FALSE;
		gtk_widget_destroy (GTK_WIDGET (chooser));
		gtk_main_quit ();
		break;

	}

	gb_debug (DEBUG_FILE, "END");
}

#else

/*****************************************************************************/
/* "Save As" menu callback.                                                  */
/*****************************************************************************/
gboolean
gb_file_save_as (gbDoc     *doc,
		 GtkWindow *window)
{
	GtkFileSelection *fsel;
	gboolean          saved_flag = FALSE;
	gboolean          destroy_flag = FALSE;
	gchar            *name, *title;

	gb_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (doc && GB_IS_DOC(doc), FALSE);
	g_return_val_if_fail (window && GTK_IS_WINDOW(window), FALSE);

	name = gb_doc_get_short_name (doc);
	title = g_strdup_printf (_("Save \"%s\" as"), name);
	g_free (name);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (title));
	gtk_window_set_modal (GTK_WINDOW (fsel), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (fsel), window);

	g_object_set_data (G_OBJECT (fsel), "doc", doc);
	g_object_set_data (G_OBJECT (fsel), "saved_flag", &saved_flag);

	g_signal_connect (G_OBJECT (fsel->ok_button), "clicked",
			  G_CALLBACK (save_as_ok_cb), fsel);

	g_signal_connect (G_OBJECT (fsel->cancel_button), "clicked",
			  G_CALLBACK (save_as_cancel_cb), fsel);

	g_signal_connect (G_OBJECT (fsel), "destroy",
			  G_CALLBACK (save_as_destroy_cb), &destroy_flag);

	/* Recover proper state of save-as dialog */
	if (save_path != NULL) {
		gtk_file_selection_set_filename (fsel, save_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

	/* Hold here and process events until we are done with this dialog. */
	gtk_main ();

	/* Destroy dialog if not already destroyed. */
	if (!destroy_flag) {
		/* Disconnect our destroy callback first, so that we don't
		 * kill the current gtk_main() loop. */
		g_signal_handlers_disconnect_by_func (GTK_OBJECT (fsel),
						      G_CALLBACK (save_as_destroy_cb),
						      &destroy_flag);
		gtk_widget_destroy (GTK_WIDGET (fsel));
	}

	g_free (title);

	gb_debug (DEBUG_FILE, "END");

	/* Return flag as set by one of the above callbacks, TRUE = saved */
	return saved_flag;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" ok button callback.                                   */
/*---------------------------------------------------------------------------*/
static void
save_as_ok_cb (GtkWidget        *widget,
	       GtkFileSelection *fsel)
{
	gchar            *raw_filename, *filename;
	GtkWidget        *dlg;
	gbDoc            *doc;
	gbDocXMLStatus    status;
	EggRecentModel   *recent;
	gboolean         *saved_flag;
	gchar            *primary_msg;
	gboolean          cancel_flag = FALSE;

	gb_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	doc = g_object_get_data (G_OBJECT(fsel), "doc");
	saved_flag = g_object_get_data (G_OBJECT(fsel), "saved_flag");

	/* get the filename */
	raw_filename = g_strdup (gtk_file_selection_get_filename (fsel));

	gb_debug (DEBUG_FILE, "raw_filename = \"%s\"", raw_filename);

	if (!raw_filename || g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

		dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please supply a valid file name"));

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

	} else {

		filename = gb_util_add_extension (raw_filename);

		gb_debug (DEBUG_FILE, "filename = \"%s\"", filename);

		if (g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
			gint ret;

			primary_msg = g_strdup_printf (_("Overwrite file \"%s\"?"),
						       filename);

			dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						primary_msg,
						_("File already exists."));
			
			g_free (primary_msg);

			ret = gtk_dialog_run (GTK_DIALOG (dlg));
			if ( ret == GTK_RESPONSE_NO ) {
				cancel_flag = TRUE;
			}
			gtk_widget_destroy (dlg);
		}

		if (!cancel_flag) {

			gb_doc_xml_save (doc, filename, &status);

			gb_debug (DEBUG_FILE, "status of save = %d", status);

			if ( status != GB_DOC_XML_OK ) {

				primary_msg = g_strdup_printf (_("Could not save file \"%s\""),
							       filename);

				dlg = gb_hig_alert_new (GTK_WINDOW(fsel),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_CLOSE,
							primary_msg,
							_("Error encountered during save.  The file is still not saved."));

				g_free (primary_msg);

				gtk_dialog_run (GTK_DIALOG (dlg));
				gtk_widget_destroy (dlg);

			} else {

				*saved_flag = TRUE;

				gb_recent_add_uri (filename);

				if (save_path != NULL)
					g_free (save_path);
				save_path = g_path_get_dirname (filename);
				if (save_path != NULL)
					save_path = g_strconcat (save_path, "/", NULL);

				gtk_widget_destroy (GTK_WIDGET (fsel));
			}

		}

		g_free (filename);
	}

	g_free (raw_filename);

	gb_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" cancel button callback.                               */
/*---------------------------------------------------------------------------*/
static void
save_as_cancel_cb (GtkWidget        *widget,
		   GtkFileSelection *fsel)
{
	gboolean *saved_flag = g_object_get_data (G_OBJECT (fsel), "saved_flag");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	*saved_flag = FALSE;
	gtk_widget_hide (GTK_WIDGET (fsel));
	gtk_main_quit ();
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" destroy callback.                                     */
/*---------------------------------------------------------------------------*/
static void
save_as_destroy_cb (GtkWidget *widget,
		    gboolean  *destroy_flag)
{
	*destroy_flag = TRUE;
	gtk_main_quit ();
}

#endif

/*****************************************************************************/
/* "Close" menu callback.                                                    */
/*****************************************************************************/
gboolean
gb_file_close (gbWindow *window)
{
	gbView  *view;
	gbDoc   *doc;
	gboolean close = TRUE;

	gb_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (window && GB_IS_WINDOW(window), TRUE);

	if ( !gb_window_is_empty (window) ) {

		view = GB_VIEW(window->view);
		doc = view->doc;

		if (gb_doc_is_modified (doc))	{
			GtkWidget *msgbox;
			gchar *fname = NULL, *msg = NULL;
			gint ret;

			fname = gb_doc_get_short_name (doc);
			
			msg = g_strdup_printf (_("Save changes to document \"%s\" before closing?"),
					       fname);
			
			msgbox = gb_hig_alert_new (GTK_WINDOW(window),
						   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_WARNING,
						   GTK_BUTTONS_NONE,
						   msg,
						   _("Your changes will be lost if you don't save them."));

			gtk_dialog_add_button (GTK_DIALOG (msgbox),
					       _("Close without saving"),
					       GTK_RESPONSE_NO);

			gtk_dialog_add_button (GTK_DIALOG (msgbox),
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

			gtk_dialog_add_button (GTK_DIALOG (msgbox),
					       GTK_STOCK_SAVE, GTK_RESPONSE_YES);

			gtk_dialog_set_default_response	(GTK_DIALOG (msgbox), GTK_RESPONSE_YES);

			gtk_window_set_resizable (GTK_WINDOW (msgbox), FALSE);

			ret = gtk_dialog_run (GTK_DIALOG (msgbox));
		
			gtk_widget_destroy (msgbox);

			g_free (fname);
			g_free (msg);
		
			switch (ret)
			{
			case GTK_RESPONSE_YES:
				close = gb_file_save (doc,
						      GTK_WINDOW(window));
				break;
			case GTK_RESPONSE_NO:
				close = TRUE;
				break;
			default:
				close = FALSE;
			}

			gb_debug (DEBUG_FILE, "CLOSE: %s", close ? "TRUE" : "FALSE");
		}

	}

	if (close) {
		gtk_widget_destroy (GTK_WIDGET(window));

		if ( gb_window_get_window_list () == NULL ) {
			
			gb_debug (DEBUG_FILE, "All windows closed.");
	
			bonobo_main_quit ();
		}

	}

	gb_debug (DEBUG_FILE, "END");

	return close;
}

/*****************************************************************************/
/* "Exit" menu callback.                                                     */
/*****************************************************************************/
void
gb_file_exit (void)
{
	const GList *window_list;
	GList       *p, *p_next;

	gb_debug (DEBUG_FILE, "START");

	window_list = gb_window_get_window_list ();

	for (p=(GList *)window_list; p != NULL; p=p_next) {
		p_next = p->next;

		gb_file_close (GB_WINDOW(p->data));
	}

	gb_debug (DEBUG_FILE, "END");
}
