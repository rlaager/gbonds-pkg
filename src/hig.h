/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  hig.h:  HIG inspired dialogs and layout tools
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

#ifndef __HIG_H__
#define __HIG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_HIG_PAD1   6
#define GB_HIG_PAD2  12


/*===========================================================================*/
/* HIG inspired alert.                                                       */
/*===========================================================================*/

#define GB_TYPE_HIG_ALERT (gb_hig_alert_get_type ())
#define GB_HIG_ALERT(obj) \
        (GTK_CHECK_CAST((obj), GB_TYPE_HIG_ALERT, gbHigAlert ))
#define GB_HIG_ALERT_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_HIG_ALERT, gbHigAlertClass))
#define GB_IS_HIG_ALERT(obj) \
        (GTK_CHECK_TYPE ((obj), GB_TYPE_HIG_ALERT))
#define GB_IS_HIG_ALERT_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_HIG_ALERT))

typedef struct _gbHigAlert      gbHigAlert;
typedef struct _gbHigAlertClass gbHigAlertClass;

struct _gbHigAlert {
	GtkDialog         parent_widget;
};

struct _gbHigAlertClass {
	GtkDialogClass    parent_class;
};

GType      gb_hig_alert_get_type    (void);

GtkWidget *gb_hig_alert_new         (GtkWindow      *parent,
				     GtkDialogFlags  flags,
				     GtkMessageType  type,
				     GtkButtonsType  buttons,
				     const gchar    *primary_text,
				     const gchar    *secondary_text);


/*===========================================================================*/
/* HIG Dialog wrapper.                                                       */
/*===========================================================================*/

#define GB_TYPE_HIG_DIALOG (gb_hig_dialog_get_type ())
#define GB_HIG_DIALOG(obj) \
        (GTK_CHECK_CAST((obj), GB_TYPE_HIG_DIALOG, gbHigDialog ))
#define GB_HIG_DIALOG_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_HIG_DIALOG, gbHigDialogClass))
#define GB_IS_HIG_DIALOG(obj) \
        (GTK_CHECK_TYPE ((obj), GB_TYPE_HIG_DIALOG))
#define GB_IS_HIG_DIALOG_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_HIG_DIALOG))

typedef struct _gbHigDialog      gbHigDialog;
typedef struct _gbHigDialogClass gbHigDialogClass;

struct _gbHigDialog {
	GtkDialog         parent_widget;

	GtkWidget        *vbox;
};

struct _gbHigDialogClass {
	GtkDialogClass    parent_class;
};

GType      gb_hig_dialog_get_type         (void);

GtkWidget *gb_hig_dialog_new              (void);

GtkWidget *gb_hig_dialog_new_with_buttons (const gchar    *title,
					   GtkWindow      *parent,
					   GtkDialogFlags  flags,
					   const gchar    *first_button_text,
					   ...);

void       gb_hig_dialog_add_widget       (gbHigDialog   *dialog,
					   GtkWidget     *widget);


/*===========================================================================*/
/* HIG Category (analogous to a gtk_frame).                                  */
/*===========================================================================*/

#define GB_TYPE_HIG_CATEGORY (gb_hig_category_get_type ())
#define GB_HIG_CATEGORY(obj) \
        (GTK_CHECK_CAST((obj), GB_TYPE_HIG_CATEGORY, gbHigCategory ))
#define GB_HIG_CATEGORY_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_HIG_CATEGORY, gbHigCategoryClass))
#define GB_IS_HIG_CATEGORY(obj) \
        (GTK_CHECK_TYPE ((obj), GB_TYPE_HIG_CATEGORY))
#define GB_IS_HIG_CATEGORY_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_HIG_CATEGORY))

typedef struct _gbHigCategory      gbHigCategory;
typedef struct _gbHigCategoryClass gbHigCategoryClass;

struct _gbHigCategory {
	GtkVBox           parent_widget;

	GtkWidget        *label;
	GtkWidget        *vbox;
};

struct _gbHigCategoryClass {
	GtkVBoxClass      parent_class;
};

GType      gb_hig_category_get_type         (void);

GtkWidget *gb_hig_category_new              (const gchar *header);

void       gb_hig_category_add_widget       (gbHigCategory *cat,
					     GtkWidget     *widget);


/*===========================================================================*/
/* HIG VBOX wrapper.                                                         */
/*===========================================================================*/

typedef enum {
	GB_HIG_VBOX_OUTER,
	GB_HIG_VBOX_INNER,
} gbHigVBoxType;

#define GB_TYPE_HIG_VBOX (gb_hig_vbox_get_type ())
#define GB_HIG_VBOX(obj) \
        (GTK_CHECK_CAST((obj), GB_TYPE_HIG_VBOX, gbHigVBox ))
#define GB_HIG_VBOX_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_HIG_VBOX, gbHigVBoxClass))
#define GB_IS_HIG_VBOX(obj) \
        (GTK_CHECK_TYPE ((obj), GB_TYPE_HIG_VBOX))
#define GB_IS_HIG_VBOX_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_HIG_VBOX))

typedef struct _gbHigVBox      gbHigVBox;
typedef struct _gbHigVBoxClass gbHigVBoxClass;

struct _gbHigVBox {
	GtkVBox           parent_widget;
};

struct _gbHigVBoxClass {
	GtkVBoxClass      parent_class;
};

GType      gb_hig_vbox_get_type         (void);

GtkWidget *gb_hig_vbox_new              (gbHigVBoxType  type);

void       gb_hig_vbox_add_widget       (gbHigVBox     *hig_vbox,
					 GtkWidget     *widget);


/*===========================================================================*/
/* HIG HBOX wrapper.                                                         */
/*===========================================================================*/

#define GB_TYPE_HIG_HBOX (gb_hig_hbox_get_type ())
#define GB_HIG_HBOX(obj) \
        (GTK_CHECK_CAST((obj), GB_TYPE_HIG_HBOX, gbHigHBox ))
#define GB_HIG_HBOX_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_HIG_HBOX, gbHigHBoxClass))
#define GB_IS_HIG_HBOX(obj) \
        (GTK_CHECK_TYPE ((obj), GB_TYPE_HIG_HBOX))
#define GB_IS_HIG_HBOX_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_HIG_HBOX))

typedef struct _gbHigHBox      gbHigHBox;
typedef struct _gbHigHBoxClass gbHigHBoxClass;

struct _gbHigHBox {
	GtkHBox           parent_widget;
};

struct _gbHigHBoxClass {
	GtkHBoxClass      parent_class;
};

GType      gb_hig_hbox_get_type           (void);

GtkWidget *gb_hig_hbox_new                (void);

void       gb_hig_hbox_add_widget         (gbHigHBox     *hig_hbox,
					   GtkWidget     *widget);

void       gb_hig_hbox_add_widget_justify (gbHigHBox     *hig_hbox,
					   GtkWidget     *widget);


G_END_DECLS

#endif /* __HIG_H__ */
