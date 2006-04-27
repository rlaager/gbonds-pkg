/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  recent.h:  gbonds recent files module header file
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
#ifndef __RECENT_H__
#define __RECENT_H__

#include "recent-files/egg-recent-model.h"

gchar            *gb_recent_get_filename (EggRecentItem *item);

void		  gb_recent_add_uri      (gchar         *uri);

EggRecentModel   *gb_recent_get_model    (void);

void              gb_recent_init         (void);

#endif /*__RECENT_H__*/

