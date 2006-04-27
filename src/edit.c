/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  edit.c:  Savings Bond Edit module
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
#include <gnome.h>

#include "edit.h"
#include "hig.h"

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*============================================*/
/* Private function prototypes.               */
/*============================================*/
static void create_add_dialog_widgets    (gbHigDialog *add_dlg,
					  gbView      *view);

static void add_response                 (GtkDialog   *dlg,
					  gint         response,
					  gbView      *view);

static void create_title_dialog_widgets  (gbHigDialog *add_dlg,
					  gbView      *view);

static void title_response               (GtkDialog   *dlg,
					  gint         response,
					  gbView      *view);



/*****************************************************************************/
/* create "Add bond" dialog.                                                 */
/*****************************************************************************/
void
gb_edit_add_bond (gbView       *view,
		  BonoboWindow *win)
{
	GtkWidget *add_dlg;

	g_return_if_fail (view && GB_IS_VIEW(view));
	g_return_if_fail (win && BONOBO_IS_WINDOW(win));

	add_dlg = gb_hig_dialog_new_with_buttons ( _("Add new bond"),
						   GTK_WINDOW(win),
						   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_STOCK_OK, GTK_RESPONSE_OK,
						   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						   NULL );

	create_add_dialog_widgets (GB_HIG_DIALOG(add_dlg), view);

	g_signal_connect( G_OBJECT(add_dlg), "response",
			  G_CALLBACK(add_response), view );

	gtk_widget_show_all( GTK_WIDGET(add_dlg) );
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create widgets to enter a bond by sn and idate.                 */
/*---------------------------------------------------------------------------*/
static void
create_add_dialog_widgets (gbHigDialog *add_dlg,
			   gbView      *view)
{
	GtkWidget    *wframe, *whbox, *wlabel, *sn_entry, *idate_entry;
	GtkSizeGroup *size_group;
	gchar        *name, *label;

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	name = gb_doc_get_title (view->doc);
	if ( name == NULL ) {
		name = gb_doc_get_short_name (view->doc);
		g_return_if_fail (name != NULL);
	}
	label = g_strdup_printf (_("Add bond to \"%s\" "), name);
	g_free (name);
	wframe = gb_hig_category_new (label);
	g_free (label);
	gb_hig_dialog_add_widget (add_dlg, wframe);

	whbox = gb_hig_hbox_new();
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe), whbox);

	wlabel = gtk_label_new(_("Serial Number: "));
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0, 0.5);
	gtk_size_group_add_widget (size_group, wlabel);
	gb_hig_hbox_add_widget (GB_HIG_HBOX(whbox), wlabel);
	sn_entry = gtk_entry_new();
	gb_hig_hbox_add_widget (GB_HIG_HBOX(whbox), sn_entry);

	whbox = gb_hig_hbox_new();
	gb_hig_category_add_widget (GB_HIG_CATEGORY(wframe), whbox);

	wlabel = gtk_label_new(_("Issue Date: "));
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0, 0.5);
	gtk_size_group_add_widget (size_group, wlabel);
	gb_hig_hbox_add_widget (GB_HIG_HBOX(whbox), wlabel);
	idate_entry = gtk_entry_new();
	gb_hig_hbox_add_widget (GB_HIG_HBOX(whbox), idate_entry);

	g_object_set_data( G_OBJECT(add_dlg), "sn_entry", sn_entry );
	g_object_set_data( G_OBJECT(add_dlg), "idate_entry", idate_entry );
}

/*---------------------------------------------------------------------------*/
/* Add "response" callback.                                                  */
/*---------------------------------------------------------------------------*/
static void
add_response (GtkDialog *add_dlg,
	      gint       response,
	      gbView    *view)
{
	GtkWidget *sn_entry, *idate_entry;
	gchar     *sn, *idate, *msg;
	gbDocBond *p_bond;
	gbStatus   status;
	GtkWidget *dlg;

	switch (response) {

	case GTK_RESPONSE_OK:
		sn_entry =
			GTK_WIDGET( g_object_get_data( G_OBJECT(add_dlg), "sn_entry" ) );
		idate_entry =
			GTK_WIDGET( g_object_get_data( G_OBJECT(add_dlg), "idate_entry" ) );

		sn    = gtk_editable_get_chars( GTK_EDITABLE(sn_entry), 0, -1 );
		idate = gtk_editable_get_chars( GTK_EDITABLE(idate_entry), 0, -1 );

		p_bond = gb_doc_bond_new_from_sn_idate( sn, idate, &status );

		g_free( sn );
		g_free( idate );

		switch (status) {

		case GB_ERROR_PARSE_SN_EMPTY:
			msg=g_strdup_printf( _("Must enter serial number.") );
			break;

		case GB_ERROR_PARSE_SN_BAD_DENOM:
			msg=g_strdup_printf( _("Bad serial number, unknown denomination.") );
			break;

		case GB_ERROR_PARSE_SN_DIGITS:
			msg=g_strdup_printf( _("Bad serial number, must contain 1-10 digits.") );
			break;

		case GB_ERROR_PARSE_SN_BAD_SERIES:
			msg=g_strdup_printf( _("Bad serial number, bad series.") );
			break;

		case GB_ERROR_PARSE_DATE_EMPTY:
			msg=g_strdup_printf( _("Must enter issue date.") );
			break;

		case GB_ERROR_PARSE_DATE_FORMAT:
			msg=g_strdup_printf( _("Bad issue date, format should be mm/yyyy.") );
			break;

		case GB_ERROR_PARSE_DATE_MONTH:
			msg=g_strdup_printf( _("Bad issue date, month must be 1-12.") );
			break;

		case GB_ERROR_PARSE_DATE_YEAR:
			msg=g_strdup_printf(
				_("Bad issue date, year must be in 4 digit format 1940 or later.") );
			break;

		case GB_ERROR_BAD_SERIES_DENOM:
			msg=g_strdup_printf( _("Denomination not valid for series") );
			break;

		case GB_ERROR_BAD_ISSUE_DATE:
			msg=g_strdup_printf( _("Issue date not valid for series") );
			break;

		default:
			msg=g_strdup_printf( _("Unknown error(%d) while adding bond"), status );
			break;

		case GB_OK:
			status = gb_doc_add_bond( view->doc, p_bond );
			if ( status == GB_ERROR_DUPLICATE_SN ) {
				msg = g_strdup_printf( _("Duplicate Serial Number, %s"),
						       p_bond->sn );
			}
			else {
				msg = NULL;
			}
		}
		if ( msg == NULL ) {
			gtk_widget_destroy( GTK_WIDGET(add_dlg) );
		} else {
			g_print ("ALERT: %s\n", msg);
			dlg = gb_hig_alert_new (GTK_WINDOW(add_dlg),
						GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_CLOSE,
						_("Cannot create or add bond"), msg);
			gtk_dialog_run (GTK_DIALOG (dlg));
                        gtk_widget_destroy (dlg);

		}
		g_free( msg );
		break;


	default:
		gtk_widget_destroy( GTK_WIDGET(add_dlg) );
		break;
	}
}

/*****************************************************************************/
/* create "Delete" dialog(s).                                                */
/*****************************************************************************/
void
gb_edit_delete_bonds (gbView       *view,
		      BonoboWindow *win)
{
	GtkWidget     *del_dlg;
	gbDocBond     *p_bond;
	gbDocBondInfo *info;
	gbStatus       status;
	gchar         *msg;
	GList         *selected_bond_list, *p;

	enum { ASK, DONT_ASK, CANCEL } state = ASK;

	g_return_if_fail (view && GB_IS_VIEW(view));
	g_return_if_fail (win && BONOBO_IS_WINDOW(win));

	selected_bond_list = gb_view_get_selected_bond_list( view );

	for ( p=selected_bond_list; (p != NULL) && (state != CANCEL); p=p->next ) {
		p_bond = (gbDocBond *)p->data;
		info = gb_doc_bond_get_info( p_bond, view->rdate, &status );
		if ( !info ) {
			gb_hig_alert_new (GTK_WINDOW(win),
					  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_CLOSE,
					  _("Error"),
					  _("Problem getting bond information!"));
			return;
		}
		if ( state == ASK ) {
			msg=g_strdup_printf( _("        SN: %s\n"
					       "        Issue date: %s\n"),
					     info->sn, info->issue_date );

			if ( p->next == NULL ) /* Last or only selected bond */ {

				del_dlg =
					gb_hig_alert_new (GTK_WINDOW(win),
							  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							  GTK_MESSAGE_QUESTION,
							  GTK_BUTTONS_YES_NO,
							  _("Delete selected bond?"),
							  msg);
				gb_doc_bond_free_info( info );
				g_free( msg );

				switch ( gtk_dialog_run( GTK_DIALOG(del_dlg) ) ) {

				case GTK_RESPONSE_YES:
					gb_doc_delete_bond( view->doc, p_bond );
					break;

				default: /* No */
					break;
					
				}
				gtk_widget_destroy( GTK_WIDGET(del_dlg) );

			}
			else /* Multiple selected bonds, more complex question */ {

				del_dlg =
					gb_hig_alert_new (GTK_WINDOW(win),
							  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							  GTK_MESSAGE_QUESTION,
							  GTK_BUTTONS_NONE,
							  _("Delete bond?"),
							  msg);
				gb_doc_bond_free_info( info );
				g_free( msg );

				gtk_dialog_add_buttons (GTK_DIALOG(del_dlg),
							GTK_STOCK_YES, GTK_RESPONSE_YES,
							_("Yes to All"), 1,
							GTK_STOCK_NO, GTK_RESPONSE_NO,
							GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							NULL);

				switch ( gtk_dialog_run( GTK_DIALOG(del_dlg) ) ) {

				case GTK_RESPONSE_YES:
					gb_doc_delete_bond( view->doc, p_bond );
					state = ASK;
					break;

				case 1: /* Yes to All */
					gb_doc_delete_bond( view->doc, p_bond );
					state = DONT_ASK;
					break;

				case GTK_RESPONSE_NO:
					state = ASK;
					break;

				default: /* Cancel */
					state = CANCEL;
					break;
				}
				gtk_widget_destroy( GTK_WIDGET(del_dlg) );

			}

		}
		else /* DONT_ASK */ {

			/* Just go ahead and delete bond */
			gb_doc_delete_bond( view->doc, p_bond );

		}

	}
	g_list_free( selected_bond_list );
}

/*****************************************************************************/
/* create "Edit title" dialog.                                               */
/*****************************************************************************/
void
gb_edit_title (gbView       *view,
	       BonoboWindow *win)
{
	GtkWidget *title_dlg;

	g_return_if_fail (view && GB_IS_VIEW(view));
	g_return_if_fail (win && BONOBO_IS_WINDOW(win));

	title_dlg = gb_hig_dialog_new_with_buttons (_("Edit inventory title"),
						    GTK_WINDOW(win),
						    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						    GTK_STOCK_OK, GTK_RESPONSE_OK,
						    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						    NULL);

	create_title_dialog_widgets (GB_HIG_DIALOG(title_dlg), view);

	g_signal_connect( G_OBJECT(title_dlg), "response",
			  G_CALLBACK(title_response), view );

	gtk_widget_show_all( GTK_WIDGET(title_dlg) );
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create widgets to edit title of inventory.    .                 */
/*---------------------------------------------------------------------------*/
static void
create_title_dialog_widgets (gbHigDialog *title_dlg,
			     gbView      *view)
{
	GtkWidget *wframe, *title_entry;
	gchar     *name, *label, *text;

	name = gb_doc_get_short_name (view->doc);
	label = g_strdup_printf (_("Title for \"%s\" "), name);
	g_free(name);
	wframe = gb_hig_category_new (label);
	g_free(label);
	gb_hig_dialog_add_widget (title_dlg, wframe);

	title_entry = gtk_entry_new ();
	text = gb_doc_get_title (view->doc);
	if ( text != NULL ) {
		gtk_entry_set_text( GTK_ENTRY(title_entry), text );
		g_free( text );
	}
	gb_hig_category_add_widget( GB_HIG_CATEGORY(wframe), title_entry );

	g_object_set_data( G_OBJECT(title_dlg), "title_entry", title_entry );
}

/*---------------------------------------------------------------------------*/
/* Title "response" callback.                                                */
/*---------------------------------------------------------------------------*/
static void
title_response (GtkDialog *title_dlg,
		gint       response,
		gbView    *view)
{
	GtkWidget *title_entry;
	gchar     *text;

	switch (response) {

	case GTK_RESPONSE_OK:
		title_entry =
			GTK_WIDGET(g_object_get_data( G_OBJECT(title_dlg), "title_entry"));

		text = gtk_editable_get_chars( GTK_EDITABLE(title_entry), 0, -1 );
		gb_doc_set_title (view->doc, text );
		g_free( text );
		break;

	default:
		break;

	}

	gtk_widget_destroy( GTK_WIDGET(title_dlg) );
}






