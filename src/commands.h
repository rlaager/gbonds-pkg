/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  commands.h:  gbonds commands module header file
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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <bonobo/bonobo-ui-component.h>

G_BEGIN_DECLS

void gb_cmd_file_new             (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_open            (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_import          (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_save            (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_save_as         (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_print           (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_print_preview   (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_close           (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_file_exit            (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);


void gb_cmd_edit_undo            (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_redo            (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_cut             (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_copy            (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_paste           (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_add             (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_delete          (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_title           (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_select_all      (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_edit_unselect_all    (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);


void gb_cmd_settings_preferences (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_settings_update      (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);


void gb_cmd_help_contents        (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);

void gb_cmd_help_about 	         (BonoboUIComponent *uic,
				  gpointer           user_data,
				  const gchar       *verbname);
G_END_DECLS

#endif /* __COMMANDS_H__ */ 
