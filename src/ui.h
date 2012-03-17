/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  ui.h:  gbonds UI module header file
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
#ifndef __GB_UI_H__
#define __GB_UI_H__

#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-window.h>

#include "view.h"

G_BEGIN_DECLS

void gb_ui_init                   (BonoboUIComponent *ui_component,
				   BonoboWindow      *win);

void gb_ui_unref                  (BonoboUIComponent *ui_component);

void gb_ui_update_all             (BonoboUIComponent *ui_component,
				   gbView            *view);

void gb_ui_update_nodoc           (BonoboUIComponent *ui_component);

void gb_ui_update_modified_verbs  (BonoboUIComponent *ui_component,
				   gbDoc             *doc);

void gb_ui_update_selection_verbs (BonoboUIComponent *ui_component,
				   gbView            *view);

void gb_ui_update_undo_redo_verbs (BonoboUIComponent *ui_component,
				   gbDoc             *doc);

G_END_DECLS

#endif /* __GB_UI_H__ */
