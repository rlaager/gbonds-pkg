/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  rules.h:  Savings Bond Rules module header file.
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

#ifndef __RULES_H__
#define __RULES_H__

#include "types.h"

G_BEGIN_DECLS

gbStatus  gb_rules_test_series_denom           (gbSeries  series,
						gdouble   denom );

gbStatus  gb_rules_test_issue                  (gbSeries  series,
						gbDate    idate );

gbStatus  gb_rules_determine_issue             (gbSeries  series,
						gdouble   denom,
						gdouble  *issue);

gbStatus  gb_rules_determine_maturity          (gbSeries  series,
						gbDate    idate,
						gbDate   *mdate);

gbStatus  gb_rules_get_last_accrual            (gbSeries  series,
						gbDate    idate,
						gbDate    rdate,
						gbDate    mdate,
						gbDate   *a1date);

gbStatus  gb_rules_get_next_accrual            (gbSeries  series,
						gbDate    idate,
						gbDate    rdate,
						gbDate    mdate,
						gbDate   *a2date);

gbStatus  gb_rules_determine_exchangeability   (gbSeries  series,
						gbDate    idate,
						gbDate    mdate,
						gbDate    rdate,
						gboolean *exchangeable_flag);

gbStatus  gb_rules_determine_nopay             (gbSeries  series,
						gbDate    idate,
						gbDate    rdate,
						gboolean *nopay_flag);

G_END_DECLS

#endif /* __RULES_H__ */


