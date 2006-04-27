/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  prefs-dialog.h:  Preferences dialog module header file
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

#ifndef __PREFS_DIALOG_H__
#define __PREFS_DIALOG_H__

#include <gtk/gtk.h>
#include "hig.h"

G_BEGIN_DECLS

#define GB_TYPE_PREFS_DIALOG            (gb_prefs_dialog_get_type ())
#define GB_PREFS_DIALOG(obj)            (GTK_CHECK_CAST ((obj), GB_TYPE_PREFS_DIALOG, gbPrefsDialog))
#define GB_PREFS_DIALOG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GB_TYPE_PREFS_DIALOG, gbPrefsDialogClass))
#define GB_IS_PREFS_DIALOG(obj)         (GTK_CHECK_TYPE ((obj), GB_TYPE_PREFS_DIALOG))
#define GB_IS_PREFS_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GB_TYPE_PREFS_DIALOG))
#define GB_PREFS_DIALOG_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GB_TYPE_PREFS_DIALOG, gbPrefsDialogClass))


typedef struct _gbPrefsDialog 		gbPrefsDialog;
typedef struct _gbPrefsDialogClass	gbPrefsDialogClass;

typedef struct _gbPrefsDialogPrivate	gbPrefsDialogPrivate;

struct _gbPrefsDialog
{
	gbHigDialog           parent_instance;

	gbPrefsDialogPrivate *private;

};

struct  _gbPrefsDialogClass
{
	gbHigDialogClass      parent_class;
};

GtkType    	gb_prefs_dialog_get_type 	(void) G_GNUC_CONST;

GtkWidget      *gb_prefs_dialog_new		(GtkWindow *parent);

G_END_DECLS

#endif /* __PREFS_DIALOG_H__ */
