/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  window.h:  a gbonds app window
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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtk.h>
#include <bonobo/bonobo-window.h>
#include <bonobo/bonobo-ui-component.h>

#include "view.h"

G_BEGIN_DECLS

#define GB_TYPE_WINDOW (gb_window_get_type ())
#define GB_WINDOW(obj) \
        (GTK_CHECK_CAST((obj), GB_TYPE_WINDOW, gbWindow ))
#define GB_WINDOW_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_WINDOW, gbWindowClass))
#define GB_IS_WINDOW(obj) \
        (GTK_CHECK_TYPE ((obj), GB_TYPE_WINDOW))
#define GB_IS_WINDOW_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_WINDOW))

typedef struct _gbWindow      gbWindow;
typedef struct _gbWindowClass gbWindowClass;

struct _gbWindow {
	BonoboWindow         parent_widget;

	BonoboUIComponent   *uic;

	GtkWidget           *view;
};

struct _gbWindowClass {
	BonoboWindowClass    parent_class;
};

GType        gb_window_get_type          (void);

GtkWidget   *gb_window_new               (void);

GtkWidget   *gb_window_new_from_file     (const gchar *filename);

GtkWidget   *gb_window_new_from_doc      (gbDoc       *doc);

gboolean     gb_window_is_empty          (gbWindow    *window);

void         gb_window_set_doc           (gbWindow    *window,
					  gbDoc       *doc);

const GList *gb_window_get_window_list   (void);

G_END_DECLS

#endif /* __WINDOW_H__ */
