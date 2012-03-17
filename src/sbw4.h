/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  sbw4.h:  SBW version 4 file format definitions
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
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

#ifndef __SBW4_H__
#define __SBW4_H__

#include "types.h"

#define SBW4_EPOCH GB_DATE(APR,1941)

typedef struct {
	short rdate;
	short dummy1;
	short n_bonds;
	short dummy3;
	short dummy4;
	short dummy5;
} SBW4_Head;

#define SBW4_CBOND_SIZE 5

typedef struct {
	long dummy0;
	long dummy1;
	long dummy2;
	long dummy3;
	long dummy4;
	long dummy5;
	long denom;
	long mdate;
	long dummy8;
	long dummy9;
	long idate;
	long dummy11;
	long dummy12;
	long adate;
	long dummy14;
	long dummy15;
	long dummy16;
	long dummy17;
	long dummy18;
	long dummy19;
	long dummy20;
} SBW4_BondInfoFixed;

#endif /* __SBW4_H__ */
