/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  prefs-dialog.c:  Preferences dialog module
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

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include "hig.h"
#include "prefs-dialog.h"
#include "prefs.h"
#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _gbPrefsDialogPrivate
{
	GtkWidget *notebook;

	GtkWidget *startup1_radio;
	GtkWidget *startup2_radio;
	GtkWidget *startup3_radio;
	GtkWidget *startup_file_entry;

	GtkWidget *range1_radio;
	GtkWidget *range2_radio;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static gbHigDialogClass* parent_class = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gb_prefs_dialog_class_init         (gbPrefsDialogClass *klass);
static void       gb_prefs_dialog_init               (gbPrefsDialog      *dlg);
static void       gb_prefs_dialog_finalize           (GObject            *object);
static void       gb_prefs_dialog_construct          (gbPrefsDialog      *dlg);

static void       response_cb                        (gbPrefsDialog      *dialog,
						      gint                response,
						      gpointer            user_data);

static GtkWidget *startup_page                       (gbPrefsDialog      *dlg);
static GtkWidget *rdate_range_page                   (gbPrefsDialog      *dlg);

static void       update_startup_page_from_prefs     (gbPrefsDialog      *dlg);
static void       update_rdate_range_page_from_prefs (gbPrefsDialog      *dlg);

static void       update_prefs_from_startup_page     (gbPrefsDialog      *dlg);
static void       update_prefs_from_rdate_range_page (gbPrefsDialog      *dlg);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gb_prefs_dialog_get_type (void)
{
	static GType dialog_type = 0;

	if (!dialog_type)
    	{
      		static GTypeInfo dialog_info =
      		{
			sizeof (gbPrefsDialogClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gb_prefs_dialog_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (gbPrefsDialog),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gb_prefs_dialog_init
      		};

     		dialog_type = g_type_register_static (GB_TYPE_HIG_DIALOG,
						      "gbPrefsDialog",
						      &dialog_info, 
						      0);
    	}

	return dialog_type;
}

static void
gb_prefs_dialog_class_init (gbPrefsDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gb_debug (DEBUG_PREFS, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gb_prefs_dialog_finalize;  	
}

static void
gb_prefs_dialog_init (gbPrefsDialog *dlg)
{
	gb_debug (DEBUG_PREFS, "");

	dlg->private = g_new0 (gbPrefsDialogPrivate, 1);
}

static void 
gb_prefs_dialog_finalize (GObject *object)
{
	gbPrefsDialog* dlg;
	
	gb_debug (DEBUG_PREFS, "");

	g_return_if_fail (object != NULL);
	
   	dlg = GB_PREFS_DIALOG (object);

	g_return_if_fail (GB_IS_PREFS_DIALOG (dlg));
	g_return_if_fail (dlg->private != NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	g_free (dlg->private);
}

/*****************************************************************************/
/* NEW preferences dialog.                                                   */
/*****************************************************************************/
GtkWidget*
gb_prefs_dialog_new (GtkWindow *parent)
{
	GtkWidget *dlg;

	gb_debug (DEBUG_PREFS, "");

	dlg = GTK_WIDGET (g_object_new (GB_TYPE_PREFS_DIALOG, NULL));

	if (parent)
		gtk_window_set_transient_for (GTK_WINDOW (dlg), parent);
	
	gb_prefs_dialog_construct (GB_PREFS_DIALOG(dlg));

	return dlg;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gb_prefs_dialog_construct (gbPrefsDialog *dlg)
{
	GtkWidget *notebook;

	g_return_if_fail (GB_IS_PREFS_DIALOG (dlg));
	g_return_if_fail (dlg->private != NULL);

	gtk_dialog_add_button (GTK_DIALOG(dlg),
			       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);

	gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_CLOSE);

	g_signal_connect(G_OBJECT (dlg), "response",
			 G_CALLBACK (response_cb), NULL);

	notebook = gtk_notebook_new ();
	gb_hig_dialog_add_widget (GB_HIG_DIALOG(dlg), notebook);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  startup_page (dlg),
				  gtk_label_new (_("Startup")));

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  rdate_range_page (dlg),
				  gtk_label_new (_("Redemption dates")));

	update_startup_page_from_prefs (dlg);
	update_rdate_range_page_from_prefs (dlg);

        gtk_widget_show_all (GTK_DIALOG (dlg)->vbox);   

        gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
        gtk_window_set_title (GTK_WINDOW (dlg), _("gbabels Preferences"));
        gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (gbPrefsDialog *dlg,
	     gint          response,
	     gpointer      user_data)
{
	gb_debug (DEBUG_PREFS, "START");

	g_return_if_fail(dlg != NULL);
	g_return_if_fail(GTK_IS_DIALOG(dlg));

	switch(response) {
	case GTK_RESPONSE_CLOSE:
		gtk_widget_hide (GTK_WIDGET(dlg));
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached ();
	}

	gb_debug (DEBUG_PREFS, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Build Startup action Properties Notebook Tab                   */
/*--------------------------------------------------------------------------*/
static GtkWidget *
startup_page (gbPrefsDialog *dlg)
{
	GtkWidget *wvbox, *wframe, *wentry;
	GSList *radio_group = NULL;

	wvbox = gb_hig_vbox_new (GB_HIG_VBOX_OUTER);

	wframe = gb_hig_category_new (_("GBonds always opens with"));
	gb_hig_vbox_add_widget (GB_HIG_VBOX(wvbox), wframe);

	radio_group = NULL;

	dlg->private->startup1_radio =
	    gtk_radio_button_new_with_label (radio_group, _("a blank inventory"));
	radio_group =
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (dlg->private->startup1_radio));
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe),
				    dlg->private->startup1_radio);

	dlg->private->startup2_radio =
	    gtk_radio_button_new_with_label (radio_group, _("the most recently accessed inventory"));
	radio_group =
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (dlg->private->startup2_radio));
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe),
				    dlg->private->startup2_radio);

	dlg->private->startup3_radio =
	    gtk_radio_button_new_with_label (radio_group, _("a default inventory:"));
	radio_group =
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (dlg->private->startup3_radio));
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe),
				    dlg->private->startup3_radio);

	dlg->private->startup_file_entry =
		gnome_file_entry_new (NULL, _("select file:"));
	gnome_file_entry_set_modal (GNOME_FILE_ENTRY(dlg->private->startup_file_entry),
				    TRUE);
	wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(dlg->private->startup_file_entry));
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe),
				    dlg->private->startup_file_entry);

	g_signal_connect_swapped (
		G_OBJECT(dlg->private->startup1_radio),
		"toggled", G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_connect_swapped (
		G_OBJECT(dlg->private->startup2_radio),
		"toggled", G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_connect_swapped (
		G_OBJECT(dlg->private->startup3_radio),
		"toggled", G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_connect_swapped (
		G_OBJECT(wentry),
		"changed", G_CALLBACK(update_prefs_from_startup_page), dlg);

	return wvbox;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Build Redemption date range page.                              */
/*--------------------------------------------------------------------------*/
static GtkWidget *
rdate_range_page (gbPrefsDialog *dlg)
{
	GtkWidget *wvbox, *wframe;
	GtkSizeGroup *label_size_group;
	GSList *radio_group = NULL;

	wvbox = gb_hig_vbox_new (GB_HIG_VBOX_OUTER);
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	wframe = gb_hig_category_new (_("Redemption date choices"));
	gb_hig_vbox_add_widget (GB_HIG_VBOX(wvbox), wframe);

	radio_group = NULL;

	dlg->private->range1_radio =
	    gtk_radio_button_new_with_label (radio_group, _("include all dates for which data exists"));
	radio_group =
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (dlg->private->range1_radio));
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe),
				    dlg->private->range1_radio);

	dlg->private->range2_radio =
	    gtk_radio_button_new_with_label (radio_group, _("limited to 1 year (newest dates for which data exists)"));
	radio_group =
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (dlg->private->range2_radio));
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe),
				    dlg->private->range2_radio);

	g_signal_connect_swapped (
		G_OBJECT(dlg->private->range1_radio),
		"toggled", G_CALLBACK(update_prefs_from_rdate_range_page), dlg);
	g_signal_connect_swapped (
		G_OBJECT(dlg->private->range2_radio),
		"toggled", G_CALLBACK(update_prefs_from_rdate_range_page), dlg);

	return wvbox;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update startup page widgets from current prefs.                */
/*--------------------------------------------------------------------------*/
static void
update_startup_page_from_prefs (gbPrefsDialog *dlg)
{
	GtkWidget *wentry;

	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->private->startup1_radio),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->private->startup2_radio),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->private->startup3_radio),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->private->startup_file_entry),
		G_CALLBACK(update_prefs_from_startup_page), dlg);

	switch (gb_prefs->startup_action) {
	case GB_PREFS_STARTUP_BLANK:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->private->startup1_radio),
			TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(dlg->private->startup_file_entry),
					  FALSE);
		break;
	case GB_PREFS_STARTUP_RECENT:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->private->startup2_radio),
			TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(dlg->private->startup_file_entry),
					  FALSE);
		break;
	case GB_PREFS_STARTUP_DEFAULT:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->private->startup3_radio),
			TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(dlg->private->startup_file_entry),
					  TRUE);
		wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(dlg->private->startup_file_entry));
		gtk_entry_set_text (GTK_ENTRY(wentry), gb_prefs->startup_file);
		break;
	default:
		g_warning ("Illegal startup action");	/* Should not happen */
		break;
	}

	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->private->startup1_radio),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->private->startup2_radio),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->private->startup3_radio),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->private->startup_file_entry),
		G_CALLBACK(update_prefs_from_startup_page), dlg);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update redemption date range page widgets from current prefs.  */
/*--------------------------------------------------------------------------*/
static void
update_rdate_range_page_from_prefs (gbPrefsDialog *dlg)
{
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->private->range1_radio),
		G_CALLBACK(update_prefs_from_rdate_range_page), dlg);
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->private->range2_radio),
		G_CALLBACK(update_prefs_from_rdate_range_page), dlg);

	switch (gb_prefs->rdate_range) {
	case GB_PREFS_RDATE_RANGE_ALL:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->private->range1_radio),
			TRUE);
		break;
	case GB_PREFS_RDATE_RANGE_YEAR:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->private->range2_radio),
			TRUE);
		break;
	default:
		g_warning ("Illegal rdate range");	/* Should not happen */
		break;
	}

	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->private->range1_radio),
		G_CALLBACK(update_prefs_from_rdate_range_page), dlg);
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->private->range2_radio),
		G_CALLBACK(update_prefs_from_rdate_range_page), dlg);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of startup page widgets.       */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_startup_page (gbPrefsDialog *dlg)
{
	GtkWidget *wentry;

	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->private->startup1_radio))) {
		gb_prefs->startup_action = GB_PREFS_STARTUP_BLANK;
		gtk_widget_set_sensitive (GTK_WIDGET(dlg->private->startup_file_entry),
					  FALSE);
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->private->startup2_radio))) {
		gb_prefs->startup_action = GB_PREFS_STARTUP_RECENT;
		gtk_widget_set_sensitive (GTK_WIDGET(dlg->private->startup_file_entry),
					  FALSE);
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->private->startup3_radio))) {
		gb_prefs->startup_action = GB_PREFS_STARTUP_DEFAULT;
		gtk_widget_set_sensitive (GTK_WIDGET(dlg->private->startup_file_entry),
					  TRUE);
		wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(dlg->private->startup_file_entry));
		gb_prefs->startup_file = gtk_editable_get_chars (GTK_EDITABLE(wentry),
								 0, -1);
	}

	gb_prefs_save_settings ();
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of rdate range page widgets.   */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_rdate_range_page (gbPrefsDialog *dlg)
{
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->private->range1_radio))) {
		gb_prefs->rdate_range = GB_PREFS_RDATE_RANGE_ALL;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->private->range2_radio))) {
		gb_prefs->rdate_range = GB_PREFS_RDATE_RANGE_YEAR;
	}

	gb_prefs_save_settings ();
}


	

	
