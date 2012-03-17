/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  window.c:  a gbonds app window
 *
 *  Copyright (C) 2002-2003  Jim Evins <evins@snaught.com>.
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

#include "ui.h"
#include "window.h"
#include "util.h"
#include "doc.h"
#include "file.h"
#include "doc-xml.h"

#include "debug.h"

/*============================================================================*/
/* Private macros and constants.                                              */
/*============================================================================*/

#define DEFAULT_WINDOW_WIDTH  620
#define DEFAULT_WINDOW_HEIGHT 500

/*============================================================================*/
/* Private globals                                                            */
/*============================================================================*/
static BonoboWindowClass *parent_class;

static GList *window_list = NULL;


/*============================================================================*/
/* Local function prototypes                                                  */
/*============================================================================*/

static void     gb_window_class_init   (gbWindowClass *class);
static void     gb_window_init         (gbWindow      *window);
static void     gb_window_finalize     (GObject       *object);
static void     gb_window_destroy      (GtkObject     *gtk_object);

static void     set_window_title       (gbWindow      *window,
					gbDoc         *doc);

static gboolean window_delete_event_cb (gbWindow      *window,
					GdkEvent      *event,
					gpointer       user_data);

static void     selection_changed_cb   (gbView        *view,
					gbWindow      *window);

static void     name_changed_cb        (gbDoc         *doc,
					gbWindow      *window);

static void     modified_changed_cb    (gbDoc         *doc,
					gbWindow      *window);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
GType
gb_window_get_type (void)
{
	static GType window_type = 0;

	if (!window_type) {
		static GTypeInfo window_info = {
			sizeof (gbWindowClass),
			NULL,
			NULL,
			(GClassInitFunc) gb_window_class_init,
			NULL,
			NULL,
			sizeof (gbWindow),
			0,
			(GInstanceInitFunc) gb_window_init,
		};

		window_type =
		    g_type_register_static (bonobo_window_get_type (),
					    "gbWindow",
					    &window_info, 0);
	}

	return window_type;
}

static void
gb_window_class_init (gbWindowClass *class)
{
	GObjectClass   *object_class     = (GObjectClass *) class;
	GtkObjectClass *gtk_object_class = (GtkObjectClass *) class;

	gb_debug (DEBUG_WINDOW, "START");

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gb_window_finalize;

	gtk_object_class->destroy = gb_window_destroy;

	gb_debug (DEBUG_WINDOW, "END");
}

static void
gb_window_init (gbWindow *window)
{
	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;

	gb_debug (DEBUG_WINDOW, "START");

	ui_container = bonobo_window_get_ui_container(BONOBO_WINDOW(window));
	ui_component = bonobo_ui_component_new_default ();
	bonobo_ui_component_set_container (ui_component,
					   BONOBO_OBJREF (ui_container),
					   NULL);

	gb_ui_init (ui_component, BONOBO_WINDOW (window));

	gtk_window_set_default_size (GTK_WINDOW (window),
				     DEFAULT_WINDOW_WIDTH,
				     DEFAULT_WINDOW_HEIGHT);

	g_signal_connect (G_OBJECT(window), "delete-event",
			  G_CALLBACK(window_delete_event_cb), NULL);
	
	window->uic  = ui_component;
	window->view = NULL;

	window_list = g_list_append (window_list, window);

	gb_debug (DEBUG_WINDOW, "END");
}

static void
gb_window_finalize (GObject *object)
{
	gbWindow *window;

	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GB_IS_WINDOW (object));

	window = GB_WINDOW (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gb_debug (DEBUG_WINDOW, "END");
}

static void
gb_window_destroy (GtkObject *gtk_object)
{
	gbWindow *window;

	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (gtk_object != NULL);
	g_return_if_fail (GB_IS_WINDOW (gtk_object));

	window = GB_WINDOW (gtk_object);
	window_list = g_list_remove (window_list, window);

	if (window->uic) {
		gb_ui_unref(window->uic);
		window->uic = NULL;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy) {
		GTK_OBJECT_CLASS (parent_class)->destroy (gtk_object);
	}

	gb_debug (DEBUG_WINDOW, "END");
}


/****************************************************************************/
/* Create an app window.                                                    */
/****************************************************************************/
GtkWidget *
gb_window_new (void)
{
	gbWindow *window;

	gb_debug (DEBUG_WINDOW, "START");

	window = g_object_new (gb_window_get_type (),
			       "win_name", "gbonds",
			       "title",    _("(none) - gbonds"),
			       NULL);

	gb_debug (DEBUG_WINDOW, "window=%p", window);
	gb_debug (DEBUG_WINDOW, "view=%p", window->view);

	gb_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}

/****************************************************************************/
/* Create an app window from a doc.                                       */
/****************************************************************************/
GtkWidget*
gb_window_new_from_doc (gbDoc *doc)
{
	gbWindow *window;

	gb_debug (DEBUG_WINDOW, "START");

	window = GB_WINDOW (gb_window_new ());

	gb_window_set_doc (window, doc);

	gb_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}

/****************************************************************************/
/* Create an app window from a file.                                        */
/****************************************************************************/
GtkWidget*
gb_window_new_from_file (const gchar *filename)
{
	gbWindow         *window;
	gchar            *abs_filename;
	gbDoc            *doc;
	gbDocXMLStatus    status;

	gb_debug (DEBUG_WINDOW, "START");

	window = GB_WINDOW (gb_window_new ());

	abs_filename = gb_util_make_absolute (filename);
	doc = gb_doc_xml_open (abs_filename, &status);
	g_free (abs_filename);

	gb_window_set_doc (window, doc);

	gb_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}

/****************************************************************************/
/* Is window empty?                                                         */
/****************************************************************************/
gboolean
gb_window_is_empty (gbWindow    *window)
{
	gbDoc *doc;

	g_return_val_if_fail (window && GB_IS_WINDOW (window), FALSE);

	if (window->view == NULL) return TRUE;

	g_return_val_if_fail (GB_IS_VIEW (window->view), FALSE);

	doc = GB_VIEW(window->view)->doc;

	g_return_val_if_fail (doc && GB_IS_DOC (doc), FALSE);

	if (gb_doc_is_untitled(doc) && !gb_doc_is_modified(doc)) return TRUE;

	return FALSE;
}

/****************************************************************************/
/* Create view from doc and place in window.                              */
/****************************************************************************/
void
gb_window_set_doc (gbWindow    *window,
		   gbDoc       *doc)
{
	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (GB_IS_WINDOW (window));
	g_return_if_fail (GB_IS_DOC (doc));

	gb_doc_clear_modified (doc);

	set_window_title (window, doc);

	if ( window->view != NULL ) {
		gtk_widget_destroy (window->view);
		window->view = NULL;
	}

	window->view = gb_view_new (doc);
	bonobo_window_set_contents (BONOBO_WINDOW(window), window->view);

	gtk_widget_show_all (window->view);

	gb_ui_update_all (window->uic, GB_VIEW(window->view));

	g_signal_connect (G_OBJECT(window->view), "selection_changed",
			  G_CALLBACK(selection_changed_cb), window);

	g_signal_connect (G_OBJECT(doc), "name_changed",
			  G_CALLBACK(name_changed_cb), window);

	g_signal_connect (G_OBJECT(doc), "modified_changed",
			  G_CALLBACK(modified_changed_cb), window);

	gb_debug (DEBUG_WINDOW, "END");
}

/****************************************************************************/
/* Return list of app windows.                                              */
/****************************************************************************/
const GList *
gb_window_get_window_list (void)
{
	gb_debug (DEBUG_WINDOW, "");
	return window_list;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set window title based on name and state of doc.              */
/*---------------------------------------------------------------------------*/
static void 
set_window_title (gbWindow *window,
		  gbDoc  *doc)
{
	gchar *name, *title;

	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (window && GB_IS_WINDOW (window));
	g_return_if_fail (doc && GB_IS_DOC (doc));

	name = gb_doc_get_title (doc);
	if ( name == NULL ) {
		name = gb_doc_get_short_name (doc);
		g_return_if_fail (name != NULL);
	}

	if (gb_doc_is_modified (doc)) {
		title = g_strdup_printf ("%s %s - gbonds",
					 name, _("(modified)"));
	} else {
		title = g_strdup_printf ("%s - gbonds", name);
	}

	gtk_window_set_title (GTK_WINDOW(window), title);

	g_free (name);
	g_free (title);

	gb_debug (DEBUG_WINDOW, "END");
}

/*-------------------------------------------------------------------------*/
/* PRIVATE.  Window "delete-event" callback.                               */
/*-------------------------------------------------------------------------*/
static gboolean
window_delete_event_cb (gbWindow      *window,
			GdkEvent      *event,
			gpointer       user_data)
{
	gb_debug (DEBUG_WINDOW, "START");

	g_return_val_if_fail (window && GB_IS_WINDOW (window), TRUE);

	gb_file_close (window);

	gb_debug (DEBUG_WINDOW, "END");

	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "selection state changed" callback.                        */
/*---------------------------------------------------------------------------*/
static void 
selection_changed_cb (gbView   *view,
		      gbWindow *window)
{
	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (view && GB_IS_VIEW (view));
	g_return_if_fail (window && GB_IS_WINDOW (window));

	gb_ui_update_selection_verbs (window->uic, view);

	gb_debug (DEBUG_WINDOW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Doc "name changed" callback.                                  */
/*---------------------------------------------------------------------------*/
static void 
name_changed_cb (gbDoc  *doc,
		 gbWindow *window)
{
	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (doc && GB_IS_DOC (doc));
	g_return_if_fail (window && GB_IS_WINDOW (window));

	set_window_title (window, doc);

	gb_debug (DEBUG_WINDOW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Doc "modified state changed" callback.                        */
/*---------------------------------------------------------------------------*/
static void 
modified_changed_cb (gbDoc  *doc,
		     gbWindow *window)
{
	gb_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (doc && GB_IS_DOC (doc));
	g_return_if_fail (window && GB_IS_WINDOW (window));

	set_window_title (window, doc);

	gb_ui_update_modified_verbs (window->uic, doc);

	gb_debug (DEBUG_WINDOW, "END");
}

