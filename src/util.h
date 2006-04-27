/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  util.h:  various small utility functions
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

#ifndef __UTIL_H__
#define __UTIL_H__

G_BEGIN_DECLS

gchar *gb_util_add_extension     (const gchar *orig_filename);

gchar *gb_util_remove_extension  (const gchar *orig_filename);

gchar *gb_util_make_absolute     (const gchar *orig_filename);

gchar *gb_util_get_home_data_dir (void);

G_END_DECLS

#endif /* __UTIL_H__ */
