/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  view.c:  gbonds View module
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
#include <gtk/gtk.h>
#include <gtk/gtkinvisible.h>
#include <string.h>

#include "view.h"
#include "doc.h"
#include "table.h"
#include "prefs.h"
#include "marshal.h"

#include "debug.h"

/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/

/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/

enum {
	SELECTION_CHANGED,
	LAST_SIGNAL
};

enum {
	/* Visible columns. */
	BOND_COLUMN_SN,
	BOND_COLUMN_ISSUE,
	BOND_COLUMN_DENOM,
	BOND_COLUMN_SERIES,
	BOND_COLUMN_VALUE,
	BOND_COLUMN_INTEREST,
	BOND_COLUMN_YIELD,
	BOND_COLUMN_NEXT,
	BOND_COLUMN_FINAL,
	BOND_COLUMN_FLAGS,

	/* Hidden columns. */
	BOND_COLUMN_RECORD,
	BOND_COLUMN_COLOR,

	N_BOND_COLUMNS
};

/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/

static GtkContainerClass *parent_class;

static guint signals[LAST_SIGNAL] = {0};

/* Public "CLIPBOARD" selection, for sharing data with other apps */
static GdkAtom clipboard_atom = GDK_NONE;

/* Private "GBONDS" selection, for sharing data between gbonds windows/apps */
static GdkAtom gbonds_selection_atom = GDK_NONE;


/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/

static void       gb_view_class_init               (gbViewClass       *class);
static void       gb_view_init                     (gbView            *view);
static void       gb_view_finalize                 (GObject           *object);

static void       gb_view_construct                (gbView            *view);
static GtkWidget *gb_view_construct_rdate_selector (gbView            *view);
static GtkWidget *gb_view_construct_totals_list    (gbView            *view);
static GtkWidget *gb_view_construct_bond_list      (gbView            *view);
static GtkWidget *gb_view_construct_legend         (gbView            *view);
static void       gb_view_construct_selection      (gbView            *view);

static void       table_model_changed_cb           (gbTableModel      *table_model,
						    gpointer           data);

static void       rdate_changed_cb                 (GtkWidget         *widget,
						    gbView            *view);

static gint       compare_strings_cb               (GtkTreeModel      *model,
						    GtkTreeIter       *a,
						    GtkTreeIter       *b,
						    gpointer           user_data);

static gint       compare_dates_cb                 (GtkTreeModel      *model,
						    GtkTreeIter       *a,
						    GtkTreeIter       *b,
						    gpointer           user_data);

static gint       compare_values_cb                (GtkTreeModel      *model,
						    GtkTreeIter       *a,
						    GtkTreeIter       *b,
						    gpointer           user_data);

static gint       compare_percents_cb              (GtkTreeModel      *model,
						    GtkTreeIter       *a,
						    GtkTreeIter       *b,
						    gpointer           user_data);

static void       update_view                      (gbView            *view);

static void       foreach_selected_cb              (GtkTreeModel      *model,
						    GtkTreePath       *path,
						    GtkTreeIter       *iter,
						    gpointer           data);

static void       selection_changed_cb             (GtkTreeSelection  *treeselection,
						    gbView            *view);

static void       selection_clear_cb               (GtkWidget         *widget,
						    GdkEventSelection *event,
						    gpointer           data);

static void       selection_get_cb                 (GtkWidget         *widget,
						    GtkSelectionData  *selection_data,
						    guint              info,
						    guint              time,
						    gpointer           data);

static void       selection_received_cb            (GtkWidget         *widget,
						    GtkSelectionData  *selection_data,
						    guint              time,
						    gpointer           data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
GType
gb_view_get_type (void)
{
	static GType view_type = 0;

	if (!view_type) {
		static GTypeInfo view_info = {
			sizeof (gbViewClass),
			NULL,
			NULL,
			(GClassInitFunc) gb_view_class_init,
			NULL,
			NULL,
			sizeof (gbView),
			0,
			(GInstanceInitFunc) gb_view_init,
		};

		view_type =
		    g_type_register_static (gtk_vbox_get_type (),
					    "gbView", &view_info, 0);
	}

	return view_type;
}

static void
gb_view_class_init (gbViewClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	gb_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gb_view_finalize;

	signals[SELECTION_CHANGED] =
		g_signal_new ("selection_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (gbViewClass, selection_changed),
			      NULL, NULL,
			      gb_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gb_debug (DEBUG_VIEW, "END");
}

static void
gb_view_init (gbView *view)
{
	gb_debug (DEBUG_VIEW, "START");

	view->doc = NULL;

	gb_debug (DEBUG_VIEW, "END");
}

static void
gb_view_finalize (GObject *object)
{
	gbView *view;

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GB_IS_VIEW (object));

	view = GB_VIEW (object);

	g_object_unref (view->bond_store);

	g_object_unref (view->doc);

	g_free (view->rdate);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gb_debug (DEBUG_VIEW, "END");
}

/****************************************************************************/
/* NEW view object.                                                         */
/****************************************************************************/
GtkWidget *
gb_view_new (gbDoc *doc)
{
	gbView       *view;
	gbTableModel *model;

	gb_debug (DEBUG_VIEW, "START");

	view  = g_object_new (gb_view_get_type (), NULL);
	view->doc = g_object_ref (doc);

	model = gb_table_get_model ();
	view->rdate = gb_date_fmt (gb_table_model_get_best_rdate_today (model));

	gb_view_construct (view);

	gb_debug (DEBUG_VIEW, "END");

	return GTK_WIDGET (view);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gb_view_construct (gbView *view)
{
	GtkWidget *wvbox, *whbox, *w;

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	wvbox = GTK_WIDGET (view);

	w = gb_view_construct_rdate_selector( view );
	gtk_box_pack_start( GTK_BOX(wvbox), w, FALSE, FALSE, 5 );
  
	w = gb_view_construct_bond_list( view );
	gtk_box_pack_start( GTK_BOX(wvbox), w, TRUE, TRUE, 5 );

	whbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, FALSE, FALSE, 5 );

	w = gb_view_construct_totals_list( view );
	gtk_box_pack_start( GTK_BOX(whbox), w, FALSE, FALSE, 0 );

	w = gb_view_construct_legend( view );
	gtk_box_pack_end( GTK_BOX(whbox), w, FALSE, FALSE, 0 );

	gb_view_construct_selection (view);

	update_view (view);

	g_signal_connect_swapped (G_OBJECT(view->doc), "changed",
				  G_CALLBACK(update_view), view);

	gb_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create rdate selection widgets.                                 */
/*---------------------------------------------------------------------------*/
static GtkWidget *
gb_view_construct_rdate_selector (gbView *view)
{
	GtkWidget     *whbox;
	GList         *rdate_list;
	int            n_dates;
	gbTableModel  *table_model;

	/* Find out desired rdate range behavior. */
	switch ( gb_prefs->rdate_range ) {
	case GB_PREFS_RDATE_RANGE_ALL:  n_dates = 0;  break;
	case GB_PREFS_RDATE_RANGE_YEAR: n_dates = 12; break;
	default: n_dates = 12; break;
	}

	whbox = gtk_hbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX(whbox),
			    gtk_label_new (_("Redemption Date:")),
			    FALSE, FALSE, 5);

	/* Combo */
	view->rdate_combo = gtk_combo_new();
	gtk_container_set_border_width( GTK_CONTAINER(view->rdate_combo), 5 );
	table_model = gb_table_get_model ();
	rdate_list = gb_table_model_get_rdate_list( table_model, n_dates );
	gtk_combo_set_popdown_strings( GTK_COMBO(view->rdate_combo), rdate_list );
	gb_table_model_free_rdate_list( rdate_list );
	gtk_box_pack_start (GTK_BOX(whbox),
			    view->rdate_combo,
			    FALSE, FALSE, 10);

	g_signal_connect( G_OBJECT(table_model), "changed",
			  G_CALLBACK(table_model_changed_cb), view );

	view->rdate_entry = GTK_COMBO(view->rdate_combo)->entry;
	gtk_entry_set_max_length( GTK_ENTRY(view->rdate_entry), 8 );
	gtk_entry_set_editable( GTK_ENTRY(view->rdate_entry), FALSE );
	gtk_entry_set_text( GTK_ENTRY(view->rdate_entry), view->rdate );

	g_signal_connect( G_OBJECT(GTK_COMBO(view->rdate_combo)->entry),
			  "changed", G_CALLBACK(rdate_changed_cb), view );

	return whbox;

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create bond list.                                               */
/*---------------------------------------------------------------------------*/
static GtkWidget *
gb_view_construct_bond_list (gbView *view)
{
	GtkWidget         *w;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection  *bond_selection;

	w = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(w),
					GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );

	view->bond_store = gtk_list_store_new (N_BOND_COLUMNS,
					       G_TYPE_STRING  /* SN */,
					       G_TYPE_STRING  /* Issue date */,
					       G_TYPE_STRING  /* Denomination */,
					       G_TYPE_STRING  /* Series */,
					       G_TYPE_STRING  /* Value */,
					       G_TYPE_STRING  /* Interest */,
					       G_TYPE_STRING  /* Yield */,
					       G_TYPE_STRING  /* Next Accrual */,
					       G_TYPE_STRING  /* Final Maturity */,
					       G_TYPE_STRING  /* Flags */,
					       G_TYPE_POINTER /* Pointer to record */,
					       G_TYPE_STRING  /* Foreground color */);
	view->bond_list = gtk_tree_view_new_with_model (GTK_TREE_MODEL(view->bond_store));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(view->bond_list), TRUE);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Serial Number"), renderer,
							   "text", BOND_COLUMN_SN,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 110);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_SN);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_SN,
                                         compare_strings_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_SN),
                                         NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (view->bond_store),
					      BOND_COLUMN_SN,
					      GTK_SORT_ASCENDING);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Issue\nDate"), renderer,
							   "text", BOND_COLUMN_ISSUE,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_ISSUE);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_ISSUE,
                                         compare_dates_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_ISSUE),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Denomination"), renderer,
							   "text", BOND_COLUMN_DENOM,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 110);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_DENOM);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_DENOM,
                                         compare_values_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_DENOM),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Series"), renderer,
							   "text", BOND_COLUMN_SERIES,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 60);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_SERIES);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_SERIES,
                                         compare_strings_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_SERIES),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Value"), renderer,
							   "text", BOND_COLUMN_VALUE,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_VALUE);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_VALUE,
                                         compare_values_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_VALUE),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Interest"), renderer,
							   "text", BOND_COLUMN_INTEREST,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_INTEREST);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_INTEREST,
                                         compare_values_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_INTEREST),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Yield\nTo Date"), renderer,
							   "text", BOND_COLUMN_YIELD,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_YIELD);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_YIELD,
                                         compare_percents_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_YIELD),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Next\nAccrual"), renderer,
							   "text", BOND_COLUMN_NEXT,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_NEXT);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_NEXT,
                                         compare_dates_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_NEXT),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Final\nMaturity"), renderer,
							   "text", BOND_COLUMN_FINAL,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_FINAL);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_FINAL,
                                         compare_dates_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_FINAL),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT(renderer), "xalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("", renderer,
							   "text", BOND_COLUMN_FLAGS,
							   "foreground", BOND_COLUMN_COLOR,
							   NULL);
	gtk_tree_view_column_set_min_width (column, 40);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BOND_COLUMN_FLAGS);
        gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->bond_store),
                                         BOND_COLUMN_FLAGS,
                                         compare_strings_cb,
                                         GINT_TO_POINTER (BOND_COLUMN_FLAGS),
                                         NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view->bond_list), column);


	bond_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view->bond_list));
	gtk_tree_selection_set_mode (bond_selection, GTK_SELECTION_MULTIPLE);
	g_signal_connect (G_OBJECT (bond_selection), "changed",
			  G_CALLBACK (selection_changed_cb), view);

	gtk_container_add( GTK_CONTAINER(w), view->bond_list );

	return w;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Create totals list widgets.                                      */
/*---------------------------------------------------------------------------*/
static GtkWidget *
gb_view_construct_totals_list(gbView *view)
{
	GtkWidget         *wouter_hbox, *wframe, *wvbox, *whbox, *wlabel;

	wouter_hbox = gtk_hbox_new (FALSE, 0);

	wframe = gtk_frame_new( _("Totals") );
	gtk_box_pack_start( GTK_BOX(wouter_hbox), wframe, FALSE, FALSE, 5 );

	wvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add( GTK_CONTAINER(wframe), wvbox );

	whbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );
	wlabel = gtk_label_new (_("Inventory value:"));
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	view->inventory_value_label = gtk_label_new ("$0.00");
	gtk_misc_set_alignment (GTK_MISC(view->inventory_value_label), 1.0, 0.5);
	gtk_box_pack_end( GTK_BOX(whbox), view->inventory_value_label, FALSE, FALSE, 5 );
	
	whbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );
	wlabel = gtk_label_new (_("Redemption value:"));
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	view->redemption_value_label = gtk_label_new ("$0.00");
	gtk_misc_set_alignment (GTK_MISC(view->redemption_value_label), 1.0, 0.5);
	gtk_box_pack_end( GTK_BOX(whbox), view->redemption_value_label, FALSE, FALSE, 5 );
	
	whbox = gtk_hbox_new (FALSE, 10);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );
	wlabel = gtk_label_new (_("Interest:"));
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	view->total_interest_label = gtk_label_new ("$0.00");
	gtk_misc_set_alignment (GTK_MISC(view->total_interest_label), 1.0, 0.5);
	gtk_box_pack_end( GTK_BOX(whbox), view->total_interest_label, FALSE, FALSE, 5 );
	

	return wouter_hbox;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Create legend widgets.                                           */
/*---------------------------------------------------------------------------*/
static GtkWidget *
gb_view_construct_legend(gbView *view)
{
	GtkWidget    *wouter_hbox, *wframe, *wvbox, *whbox, *wlabel;
	GdkColor      gdk_color = {0,0,0,0};
	GtkSizeGroup *size_group;

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	wouter_hbox = gtk_hbox_new (FALSE, 0);

	wframe = gtk_frame_new( _("Legend") );
	gtk_box_pack_start( GTK_BOX(wouter_hbox), wframe, FALSE, FALSE, 5 );

	wvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add( GTK_CONTAINER(wframe), wvbox );

	whbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );
	wlabel = gtk_label_new (_("*"));
	gtk_size_group_add_widget (size_group, wlabel);
	gdk_color_parse (GB_VIEW_NOPAY_COLOR, &gdk_color);
	gtk_widget_modify_fg (wlabel, GTK_STATE_NORMAL, &gdk_color);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.5);
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	wlabel = gtk_label_new (_("- Not yet eligible for payment"));
	gdk_color_parse (GB_VIEW_NOPAY_COLOR, &gdk_color);
	gtk_widget_modify_fg (wlabel, GTK_STATE_NORMAL, &gdk_color);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.5);
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	
	whbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );
	wlabel = gtk_label_new (_("M"));
	gtk_size_group_add_widget (size_group, wlabel);
	gdk_color_parse (GB_VIEW_MATURED_EXCH_COLOR, &gdk_color);
	gtk_widget_modify_fg (wlabel, GTK_STATE_NORMAL, &gdk_color);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.5);
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	wlabel = gtk_label_new (_("- Matured (exchangeable)"));
	gdk_color_parse (GB_VIEW_MATURED_EXCH_COLOR, &gdk_color);
	gtk_widget_modify_fg (wlabel, GTK_STATE_NORMAL, &gdk_color);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.5);
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	
	whbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );
	wlabel = gtk_label_new (_("M"));
	gtk_size_group_add_widget (size_group, wlabel);
	gdk_color_parse (GB_VIEW_MATURED_NOT_EXCH_COLOR, &gdk_color);
	gtk_widget_modify_fg (wlabel, GTK_STATE_NORMAL, &gdk_color);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.5);
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );
	wlabel = gtk_label_new (_("- Matured (not exchangeable)"));
	gdk_color_parse (GB_VIEW_MATURED_NOT_EXCH_COLOR, &gdk_color);
	gtk_widget_modify_fg (wlabel, GTK_STATE_NORMAL, &gdk_color);
	gtk_misc_set_alignment (GTK_MISC(wlabel), 0.0, 0.5);
	gtk_box_pack_start( GTK_BOX(whbox), wlabel, FALSE, FALSE, 5 );

	return wouter_hbox;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create clipboard selection targets.                             */
/*---------------------------------------------------------------------------*/
static void
gb_view_construct_selection (gbView *view)
{
	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	view->have_selection = FALSE;
	view->selection_data = NULL;
	view->invisible = gtk_invisible_new ();

	if ( !clipboard_atom ) {
		clipboard_atom = gdk_atom_intern( "CLIPBOARD", FALSE );
	}
	if ( !gbonds_selection_atom ) {
		gbonds_selection_atom = gdk_atom_intern( "GBONDS_CLIPBOARD", FALSE );
	}

	gtk_selection_add_target( view->invisible,
				  clipboard_atom,
				  GDK_SELECTION_TYPE_STRING, 1 );
	gtk_selection_add_target( view->invisible,
				  GDK_SELECTION_PRIMARY,
				  GDK_SELECTION_TYPE_STRING, 1 );
	gtk_selection_add_target( view->invisible,
				  gbonds_selection_atom,
				  GDK_SELECTION_TYPE_STRING, 1 );

	g_signal_connect( G_OBJECT(view->invisible), "selection_clear_event",
			  GTK_SIGNAL_FUNC(selection_clear_cb), view );

	g_signal_connect( G_OBJECT(view->invisible), "selection_get",
			  GTK_SIGNAL_FUNC(selection_get_cb), view );

	g_signal_connect( G_OBJECT(view->invisible), "selection_received",
			  GTK_SIGNAL_FUNC(selection_received_cb), view );

	gb_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle table model "changed" signal (redemption tables updated) */
/*---------------------------------------------------------------------------*/
static void
table_model_changed_cb (gbTableModel      *table_model,
			gpointer           data)
{
	gbView      *view = GB_VIEW (data);
	GList       *rdate_list;
	int          n_dates;

	/* Find out desired rdate range behavior. */
	switch ( gb_prefs->rdate_range ) {
	case GB_PREFS_RDATE_RANGE_ALL:  n_dates = 0;  break;
	case GB_PREFS_RDATE_RANGE_YEAR: n_dates = 12; break;
	default: n_dates = 12; break;
	}

	g_signal_handlers_block_by_func( G_OBJECT(view->rdate_entry),
					 G_CALLBACK(rdate_changed_cb), view );

	rdate_list = gb_table_model_get_rdate_list( table_model, n_dates );
	gtk_combo_set_popdown_strings( GTK_COMBO(view->rdate_combo), rdate_list );
	gb_table_model_free_rdate_list( rdate_list );

	gtk_entry_set_text( GTK_ENTRY(view->rdate_entry), view->rdate );

	g_signal_handlers_unblock_by_func( G_OBJECT(view->rdate_entry),
					   G_CALLBACK(rdate_changed_cb), view );

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Rdate selector "changed" callback.                              */
/*---------------------------------------------------------------------------*/
static void                     
rdate_changed_cb (GtkWidget *widget,
		  gbView    *view)
{
	gchar *rdate;

	rdate = gtk_editable_get_chars( GTK_EDITABLE(view->rdate_entry),
					0, -1 );
	if (strlen(rdate)) {

		view->rdate = rdate;
		update_view (view);

	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Compare strings "sort" callback.                                */
/*---------------------------------------------------------------------------*/
static gint
compare_strings_cb (GtkTreeModel      *model,
		    GtkTreeIter       *itera,
		    GtkTreeIter       *iterb,
		    gpointer           user_data)
{
	gint   column_id = GPOINTER_TO_INT (user_data);
	gchar *a_string = NULL, *b_string = NULL;
	gint   ret_val;

        gtk_tree_model_get (model, itera, column_id, &a_string, -1);
        gtk_tree_model_get (model, iterb, column_id, &b_string, -1);
        g_return_val_if_fail (a_string, 0);
        g_return_val_if_fail (b_string, 0);

	ret_val = strcmp (a_string, b_string);

	g_free (a_string);
	g_free (b_string);

	return ret_val;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Compare dates "sort" callback.                                  */
/*---------------------------------------------------------------------------*/
static gint
compare_dates_cb (GtkTreeModel      *model,
		  GtkTreeIter       *itera,
		  GtkTreeIter       *iterb,
		  gpointer           user_data)
{
	gint    column_id = GPOINTER_TO_INT (user_data);
	gchar  *a_string = NULL, *b_string = NULL;
	gint    a_month, a_year;
	gint    b_month, b_year;
	gbDate  a_date, b_date;

        gtk_tree_model_get (model, itera, column_id, &a_string, -1);
        gtk_tree_model_get (model, iterb, column_id, &b_string, -1);
        g_return_val_if_fail (a_string, 0);
        g_return_val_if_fail (b_string, 0);

	sscanf (a_string, "%d/%d", &a_month, &a_year);
	sscanf (b_string, "%d/%d", &b_month, &b_year);

	g_free (a_string);
	g_free (b_string);

	a_date = GB_DATE (a_month, a_year);
	b_date = GB_DATE (b_month, b_year);

	if ( a_date < b_date ) {
		return -1;
	}
	else if ( a_date > b_date ) {
		return +1;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Compare values "sort" callback.                                 */
/*---------------------------------------------------------------------------*/
static gint
compare_values_cb (GtkTreeModel      *model,
		   GtkTreeIter       *itera,
		   GtkTreeIter       *iterb,
		   gpointer           user_data)
{
	gint     column_id = GPOINTER_TO_INT (user_data);
	gchar   *a_string = NULL, *b_string = NULL;
	gdouble  a_val, b_val;

        gtk_tree_model_get (model, itera, column_id, &a_string, -1);
        gtk_tree_model_get (model, iterb, column_id, &b_string, -1);
        g_return_val_if_fail (a_string, 0);
        g_return_val_if_fail (b_string, 0);

	sscanf (a_string, "$%lf", &a_val);
	sscanf (b_string, "$%lf", &b_val);

	g_free (a_string);
	g_free (b_string);

	if ( a_val < b_val ) {
		return -1;
	}
	else if ( a_val > b_val ) {
		return +1;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Compare percents "sort" callback.                               */
/*---------------------------------------------------------------------------*/
static gint
compare_percents_cb (GtkTreeModel      *model,
		     GtkTreeIter       *itera,
		     GtkTreeIter       *iterb,
		     gpointer           user_data)
{
	gint     column_id = GPOINTER_TO_INT (user_data);
	gchar   *a_string = NULL, *b_string = NULL;
	gdouble  a_val, b_val;

        gtk_tree_model_get (model, itera, column_id, &a_string, -1);
        gtk_tree_model_get (model, iterb, column_id, &b_string, -1);
        g_return_val_if_fail (a_string, 0);
        g_return_val_if_fail (b_string, 0);

	sscanf (a_string, "%lf", &a_val);
	sscanf (b_string, "%lf", &b_val);

	g_free (a_string);
	g_free (b_string);

	if ( a_val < b_val ) {
		return -1;
	}
	else if ( a_val > b_val ) {
		return +1;
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update view.                                                    */
/*---------------------------------------------------------------------------*/
static void
update_view (gbView *view)
{
	GList            *p, *p1, *selected_bond_list;
	gbDocBond        *p_bond;
	gbDocBondInfo    *info;
	gbStatus          status;
	GtkTreeIter       iter;
	GtkTreeSelection *selection;
	double            inventory_value=0.0, redemption_value=0.0, total_interest=0.0;
	gchar            *color;
	gchar            *string;

	/*----------------------------------------------*/
	/* Get list of currently selected bonds.        */
	/*----------------------------------------------*/
	selected_bond_list = gb_view_get_selected_bond_list (view);

	/*----------------------------------------------*/
	/* Update Bond LIST                             */
	/*----------------------------------------------*/
	gtk_list_store_clear (view->bond_store);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view->bond_list));
	for ( p = view->doc->list; p != NULL; p=p->next ) {
		p_bond = (gbDocBond *)p->data;

		gb_debug (DEBUG_VIEW, "Updating bond %s @ %p", p_bond->sn, p);

		info = gb_doc_bond_get_info( p_bond, view->rdate, &status );
		if ( info == NULL ) break;

		gb_debug (DEBUG_VIEW, "info->sn:              %s", info->sn);
		gb_debug (DEBUG_VIEW, "info->issue_date:      %s", info->issue_date);
		gb_debug (DEBUG_VIEW, "info->denom_str:       %s", info->denom_str);
		gb_debug (DEBUG_VIEW, "info->series:          %s", info->series);
		gb_debug (DEBUG_VIEW, "info->value_str:       %s", info->value_str);
		gb_debug (DEBUG_VIEW, "info->interest_str:    %s", info->interest_str);
		gb_debug (DEBUG_VIEW, "info->yield_str:       %s", info->yield_str);
		gb_debug (DEBUG_VIEW, "info->next_accrual:    %s", info->next_accrual);
		gb_debug (DEBUG_VIEW, "info->final_maturity:  %s", info->final_maturity);
		gb_debug (DEBUG_VIEW, "info->flag_str:        %s", info->flag_str);

		if ( info->nopay_flag ) {
			color = GB_VIEW_NOPAY_COLOR;
		}
		else if ( info->matured_flag ) {
			if ( info->exchangeable_flag ) {
				color = GB_VIEW_MATURED_EXCH_COLOR;
			}
			else {
				color = GB_VIEW_MATURED_NOT_EXCH_COLOR;
			}
		}
		else {
			color = GB_VIEW_NORMAL_COLOR;
		}

		gtk_list_store_append (view->bond_store, &iter);

		gtk_list_store_set (view->bond_store, &iter,
				    BOND_COLUMN_SN,       info->sn,
				    BOND_COLUMN_ISSUE,    info->issue_date,
				    BOND_COLUMN_DENOM,    info->denom_str,
				    BOND_COLUMN_SERIES,   info->series,
				    BOND_COLUMN_VALUE,    info->value_str,
				    BOND_COLUMN_INTEREST, info->interest_str,
				    BOND_COLUMN_YIELD,    info->yield_str,
				    BOND_COLUMN_NEXT,     info->next_accrual,
				    BOND_COLUMN_FINAL,    info->final_maturity,
				    BOND_COLUMN_FLAGS,    info->flag_str,
				    BOND_COLUMN_RECORD,   p_bond,
				    BOND_COLUMN_COLOR,    color,
				    -1);

		inventory_value += info->value;
		total_interest  += info->interest;
		if ( !info->nopay_flag ) redemption_value += info->value;

		gb_doc_bond_free_info( info );

		/* preserve old selections that are still valid, if any */
		for ( p1 = selected_bond_list; p1 != NULL; p1=p1->next ) {
			if ( p1->data == p_bond ) {
				gtk_tree_selection_select_iter (selection, &iter);
			}
		}
	}

	/*----------------------------------------------*/
	/* Update TOTALS                                */
	/*----------------------------------------------*/
	string = gb_value_fmt( inventory_value, TRUE );
	gtk_label_set_text (GTK_LABEL(view->inventory_value_label), string);
	g_free( string );
	string = gb_value_fmt( redemption_value, TRUE );
	gtk_label_set_text (GTK_LABEL(view->redemption_value_label), string);
	g_free( string );
	string = gb_value_fmt( total_interest, TRUE );
	gtk_label_set_text (GTK_LABEL(view->total_interest_label), string);
	g_free( string );

	g_list_free (selected_bond_list);
}

/*****************************************************************************/
/* Get selected bond list.                                                   */
/*****************************************************************************/
GList *
gb_view_get_selected_bond_list (gbView *view)
{
	GList            *selected_bond_list;
	GtkTreeSelection *selection;

	gb_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (GB_IS_VIEW (view), NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view->bond_list));
	selected_bond_list = NULL;
	gtk_tree_selection_selected_foreach (selection,
					     foreach_selected_cb,
					     &selected_bond_list);
	return selected_bond_list;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Build selected bond list callback.                              */
/*---------------------------------------------------------------------------*/
static void
foreach_selected_cb (GtkTreeModel      *model,
		     GtkTreePath       *path,
		     GtkTreeIter       *iter,
		     gpointer           data)
{
	GList     **selected_bond_list = (GList **)data;
	gbDocBond  *p_bond;

        gtk_tree_model_get (model, iter, BOND_COLUMN_RECORD, &p_bond, -1);

	*selected_bond_list = g_list_append (*selected_bond_list, p_bond);
}

/*---------------------------------------------------------------------------*/
/* Tree view selection "changed" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
selection_changed_cb (GtkTreeSelection  *treeselection,
		      gbView            *view)
{
	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

}

/*****************************************************************************/
/* Select all items.                                                         */
/*****************************************************************************/
void
gb_view_select_all (gbView *view)
{
	GtkTreeSelection *selection;

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view->bond_list));
	gtk_tree_selection_select_all (selection);

	gb_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Remove all selections                                                     */
/*****************************************************************************/
void
gb_view_unselect_all (gbView *view)
{
	GtkTreeSelection *selection;

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view->bond_list));
	gtk_tree_selection_unselect_all (selection);

	gb_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Is our current selection empty?                                           */
/*****************************************************************************/
gboolean
gb_view_is_selection_empty (gbView *view)
{
	GList            *selected_bond_list;

	gb_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GB_IS_VIEW (view), FALSE);

	selected_bond_list = gb_view_get_selected_bond_list (view);

	if ( selected_bond_list == NULL ) {
		return TRUE;
	} else {
		g_list_free (selected_bond_list);
		return FALSE;
	}
}

/*****************************************************************************/
/* Delete selected objects. (Bypass clipboard)                               */
/*****************************************************************************/
void
gb_view_delete_selection (gbView *view)
{
	GList            *selected_bond_list, *p;
	gbDocBond        *p_bond;

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	selected_bond_list = gb_view_get_selected_bond_list (view);

	for ( p=selected_bond_list; p != NULL; p=p->next ) {
		p_bond = (gbDocBond *)p->data;

		gb_doc_delete_bond (view->doc, p_bond);
	}

	g_list_free (selected_bond_list);

	gb_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* "Cut" selected items and place in clipboard selections.                   */
/*****************************************************************************/
void
gb_view_cut (gbView *view)
{
	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	gb_view_copy (view);
	gb_view_delete_selection (view);

	gb_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* "Copy" selected items to clipboard selections.                            */
/*****************************************************************************/
void
gb_view_copy (gbView *view)
{
	GList         *selected_bond_list, *p;
	gbDocBond     *p_bond;
	gbDocBondInfo *p_info;
	gbStatus       status;
	gchar         *string;
	GByteArray    *array;

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	if ( view->selection_data != NULL ) {
		g_byte_array_free( view->selection_data, TRUE );
		view->selection_data = NULL;
	}

	array = g_byte_array_new();

	selected_bond_list = gb_view_get_selected_bond_list( view );
	for ( p=selected_bond_list; p != NULL; p=p->next ) {

		p_bond = (gbDocBond *)p->data;
		p_info = gb_doc_bond_get_info( p_bond, view->rdate, &status );
		string = g_strdup_printf( "%s %s %s %s %s %s %s %s %s %s\n",
					  p_info->sn,
					  p_info->denom_str,
					  p_info->series,
					  p_info->issue_date,
					  p_info->value_str,
					  p_info->interest_str,
					  p_info->yield_str,
					  p_info->next_accrual,
					  p_info->final_maturity,
					  p_info->flag_str );
		g_byte_array_append( array, (guint8 *)string, strlen(string) );
		g_free( string );

		gb_doc_bond_free_info( p_info );
		p_info = NULL;
	}

	g_byte_array_append( array, (guint8 *)"", 1 ); /* NULL terminate array */
	view->selection_data = array;

	/* Export data not only to our private selection, but to the public */
	/* clipboard and primary selections for other programs to pick up.  */
	gtk_selection_owner_set( view->invisible,
				 clipboard_atom, GDK_CURRENT_TIME );
	gtk_selection_owner_set( view->invisible,
				 GDK_SELECTION_PRIMARY, GDK_CURRENT_TIME );
	gtk_selection_owner_set( view->invisible,
				 gbonds_selection_atom, GDK_CURRENT_TIME );
	view->have_selection = TRUE;

	gb_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* "Paste" from private clipboard selection.                                 */
/*****************************************************************************/
void
gb_view_paste (gbView *view)
{
	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	/* We only expect to import selections from other GBonds windows and  */
	/* instances since we have very specific data requirements.           */
	gtk_selection_convert (GTK_WIDGET (view->invisible),
			       gbonds_selection_atom, GDK_SELECTION_TYPE_STRING,
			       GDK_CURRENT_TIME);

	gb_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-clear" signal.                                */
/*---------------------------------------------------------------------------*/
static void
selection_clear_cb (GtkWidget         *widget,
		    GdkEventSelection *event,
		    gpointer          data)
{
	gbView *view = GB_VIEW (data);

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	/* Ignore "PRIMARY" selection, */
	/* but not the clipboard or our private selection */
	if ( event->selection != GDK_SELECTION_PRIMARY ) {

		view->have_selection = FALSE;
		if ( view->selection_data != NULL ) {
			g_byte_array_free( view->selection_data, TRUE );
			view->selection_data = NULL;
			
		}
	}

	gb_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-get" signal.                                  */
/*---------------------------------------------------------------------------*/
static void
selection_get_cb (GtkWidget        *widget,
		  GtkSelectionData *selection_data,
		  guint            info,
		  guint            time,
		  gpointer         data)
{
	gbView *view = GB_VIEW (data);

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	if (view->have_selection) {
		gtk_selection_data_set( selection_data, GDK_SELECTION_TYPE_STRING, 8,
					view->selection_data->data,
					view->selection_data->len );
	}

	gb_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-received" signal.  (Result of Paste)          */
/*---------------------------------------------------------------------------*/
static void
selection_received_cb (GtkWidget        *widget,
		       GtkSelectionData *selection_data,
		       guint            time,
		       gpointer         data)
{
	gchar      **text, *p_sn, *p_denom, *p_series, *p_idate;
	int          i;
	gbDocBond   *p_bond;
	gbStatus     errno;
	gbView      *view = GB_VIEW (data);

	gb_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GB_IS_VIEW (view));

	if (selection_data->length < 0) {
		return;
	}
	if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
		return;
	}

	text = g_strsplit( (gchar *)selection_data->data, "\n", 0 );
	for ( i=0; (text[i] != NULL) && (*text[i] != 0); i++ ) {
		p_sn     = strtok( text[i], " " );
		p_denom  = strtok( NULL, " $" );
		p_series = strtok( NULL, " " );
		p_idate  = strtok( NULL, " " );

		p_bond = gb_doc_bond_new( p_series,
					  p_idate,
					  g_strtod(p_denom,NULL),
					  p_sn,
					  &errno );
		if ( errno == GB_OK ) {
			errno = gb_doc_add_bond( view->doc, p_bond );
		}
	}
	g_strfreev( text );

	gb_view_unselect_all (view);

	gb_debug (DEBUG_VIEW, "END");
}
