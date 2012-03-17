/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  edit.h:  Savings Bond Edit module header file
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
#ifndef __EDIT_H__
#define __EDIT_H__

#include <bonobo/bonobo-window.h>
#include "view.h"

G_BEGIN_DECLS

void    gb_edit_add_bond        (gbView       *view,
				 BonoboWindow *win);

void    gb_edit_delete_bonds    (gbView       *view,
				 BonoboWindow *win);

void    gb_edit_title           (gbView       *view,
				 BonoboWindow *win);

G_END_DECLS

#endif /* __EDIT_H__ */
