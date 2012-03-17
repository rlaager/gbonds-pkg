/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  view.h:  gbonds View module header file
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

#ifndef __VIEW_H__
#define __VIEW_H__

#include <gtk/gtk.h>

#include "doc.h"

G_BEGIN_DECLS

#define GB_VIEW_NORMAL_COLOR           "black"
#define GB_VIEW_NOPAY_COLOR            "green4"
#define GB_VIEW_MATURED_EXCH_COLOR     "blue"
#define GB_VIEW_MATURED_NOT_EXCH_COLOR "firebrick3"


#define GB_TYPE_VIEW            (gb_view_get_type ())
#define GB_VIEW(obj)            (GTK_CHECK_CAST((obj), GB_TYPE_VIEW, gbView ))
#define GB_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_VIEW, gbViewClass))
#define GB_IS_VIEW(obj)         (GTK_CHECK_TYPE ((obj), GB_TYPE_VIEW))
#define GB_IS_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_VIEW))

typedef struct _gbView      gbView;
typedef struct _gbViewClass gbViewClass;

struct _gbView {
	GtkVBox            parent_widget;

	gbDoc             *doc;

	GtkListStore      *bond_store;

	gchar             *rdate;

	GtkTreeViewColumn *sort_column;

	GtkWidget         *rdate_combo;
	GtkWidget         *rdate_entry;
	GtkWidget         *bond_list;
	GtkWidget         *inventory_value_label;
	GtkWidget         *redemption_value_label;
	GtkWidget         *total_interest_label;

	/* Clipboard selection stuff */
	gint               have_selection;
	GByteArray        *selection_data;
	GtkWidget         *invisible;
};

struct _gbViewClass {
	GtkVBoxClass      parent_class;

	/* Selection changed signal */
	void (*selection_changed) (gbView   *view,
				   gpointer  user_data);
};

GType      gb_view_get_type                (void);

GtkWidget *gb_view_new                     (gbDoc             *doc);

void       gb_view_select_all              (gbView            *view);

void       gb_view_unselect_all            (gbView            *view);

gboolean   gb_view_is_selection_empty      (gbView            *view);

GList     *gb_view_get_selected_bond_list  (gbView            *view);

void       gb_view_delete_selection        (gbView            *view);

void       gb_view_cut                     (gbView            *view);

void       gb_view_copy                    (gbView            *view);

void       gb_view_paste                   (gbView            *view);

G_END_DECLS

#endif
