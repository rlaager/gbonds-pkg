/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  table.c:  gbonds redemption table module
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
#include <config.h>


#include "table.h"

#include "debug.h"

static gbTableModel *model;


gbTableModel *
gb_table_get_model (void)
{
	return model;
}

void
gb_table_init (void)
{
	gb_debug (DEBUG_TABLE, "BEGIN");

	model = GB_TABLE_MODEL (gb_table_model_new ());

	gb_debug (DEBUG_TABLE, "END");
}
