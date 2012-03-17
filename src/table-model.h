/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  table-model.h:  gbonds redemption table model header file
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
#ifndef __TABLE_MODEL_H__
#define __TABLE_MODEL_H__

#include <glib-object.h>

#include "types.h"

G_BEGIN_DECLS

#define GB_TYPE_TABLE_MODEL            (gb_table_model_get_type ())
#define GB_TABLE_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TABLE_MODEL, gbTableModel))
#define GB_TABLE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GB_TYPE_TABLE_MODEL, gbTableModelClass))
#define GB_IS_TABLE_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_TABLE_MODEL))
#define GB_IS_TABLE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GB_TYPE_TABLE_MODEL))

typedef struct _gbTableModel          gbTableModel;
typedef struct _gbTableModelClass     gbTableModelClass;

typedef struct _gbTableModelPrivate   gbTableModelPrivate;

struct _gbTableModel {
	GObject         object;

	gbTableModelPrivate *private;
};

struct _gbTableModelClass {
	GObjectClass         parent_class;

	void (*changed)          (gbTableModel *table_model, gpointer user_data);
};


GType         gb_table_model_get_type                (void);

GObject      *gb_table_model_new                     (void);

void          gb_table_model_update                  (gbTableModel       *table_model);

GList        *gb_table_model_get_rdate_list          (gbTableModel       *table_model,
						      gint                max_dates );

void          gb_table_model_free_rdate_list         (GList              *dates);

gbDate        gb_table_model_get_rdate_today         (void);

gbDate        gb_table_model_get_best_rdate_today    (gbTableModel       *table_model);

gbDate        gb_table_model_get_rdate_max           (gbTableModel       *table_model);

gbDate        gb_table_model_get_rdate_min           (gbTableModel       *table_model);

gdouble       gb_table_model_get_value               (gbTableModel       *table_model,
						      gbSeries            series,
						      gdouble             denom,
						      gdouble             issue_price,
						      gbDate              idate,
						      gbDate              rdate,
						      gboolean           *nopay_flag);

G_END_DECLS


#endif /* __TABLE_MODEL_H__ */
