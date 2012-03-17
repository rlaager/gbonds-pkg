/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  rules.c:  Savings Bond Rules module.  Various functions for testing
 *            validity and determining characteristics of bonds.  These
 *            have been localized to this small module to facilitate
 *            future modifications as rules and regulations change for
 *            bonds issued in the future.
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

#include <math.h>
#include <stdio.h>

#include "rules.h"


/***************************************************************************/
/* Test a series and denomination for validity.                            */
/***************************************************************************/
gbStatus
gb_rules_test_series_denom (gbSeries  series,
			    gdouble   denom )
{
	switch (series) {

	case GB_SERIES_E:
		if ( denom > 1000 ) {
			return GB_ERROR_BAD_SERIES_DENOM;
		}
		break;

	case GB_SERIES_S:
		if ( denom > 100 ) {
			return GB_ERROR_BAD_SERIES_DENOM;
		}
		break;

	case GB_SERIES_EE:
	case GB_SERIES_I:
		if ( denom < 50 ) {
			return GB_ERROR_BAD_SERIES_DENOM;
		}
		break;

	default:
		return GB_ERROR_BAD_SERIES;
		break;
	}

	return GB_OK;
}

/***************************************************************************/
/* Test a series and issue date pair for validity.                         */
/***************************************************************************/
gbStatus
gb_rules_test_issue (gbSeries  series,
		     gbDate    idate )
{
	switch (series) {

	case GB_SERIES_EE:
		if ( idate < GB_DATE(JAN,1980) ) {
			return GB_ERROR_BAD_ISSUE_DATE;
		}
		break;

	case GB_SERIES_S:
		if ( idate < GB_DATE(MAY,1967) ) {
			return GB_ERROR_BAD_ISSUE_DATE;
		}
		if ( idate > GB_DATE(OCT,1970) ) {
			return GB_ERROR_BAD_ISSUE_DATE;
		}
		break;

	case GB_SERIES_I:
		if ( idate < GB_DATE(SEP,1998) ) {
			return GB_ERROR_BAD_ISSUE_DATE;
		}
		break;

	case GB_SERIES_E:
		if ( idate < GB_DATE(MAY,1941) ) {
			return GB_ERROR_BAD_ISSUE_DATE;
		}
		if ( idate > GB_DATE(JUN,1980) ) {
			return GB_ERROR_BAD_ISSUE_DATE;
		}
		break;

	default:
		return GB_ERROR_BAD_SERIES;
		break;
	}

	return GB_OK;
}

/***************************************************************************/
/* Determine issue price based on series and denomination.                 */
/***************************************************************************/
gbStatus
gb_rules_determine_issue (gbSeries  series,
			  gdouble   denom,
			  gdouble  *issue )
{
	switch (series) {

	case GB_SERIES_EE:
		*issue = denom / 2.0;
		break;
  
	case GB_SERIES_E:
		*issue = denom * 0.75;
		break;
  
	case GB_SERIES_I:
	case GB_SERIES_S:
		*issue = denom;
		break;

	default:
		return GB_ERROR_BAD_SERIES;
		break;
	}

	return GB_OK;
}

/***************************************************************************/
/* Determine final maturity date based on series and issue date.           */
/***************************************************************************/
gbStatus
gb_rules_determine_maturity (gbSeries  series,
			     gbDate    idate,
			     gbDate   *mdate )
{
	gint active_years;

	switch (series) {

	case GB_SERIES_EE:
	case GB_SERIES_S:
	case GB_SERIES_I:
		active_years = 30;
		break;

	case GB_SERIES_E:
		if ( idate <= GB_DATE(NOV,1965) ) {
			active_years = 40;
		}
		else {
			active_years = 30;
		}
		break;

	default:
		return GB_ERROR_BAD_SERIES;
		break;
	}

	*mdate = idate + 12*active_years;

	return GB_OK;
}

/***************************************************************************/
/* For a given series, issue date, redemption date, and maturity date,     */
/* determine the last date that the bond increased in value.               */
/***************************************************************************/
gbStatus
gb_rules_get_last_accrual (gbSeries  series,
			   gbDate    idate,
			   gbDate    rdate,
			   gbDate    mdate,
			   gbDate   *a1date )
{
	gint total_months, accrued_months=0;

	total_months = rdate - idate;

	if ( total_months < 0 ) {
		*a1date = 0;
		return GB_OK;
	}

	switch (series) {

	case GB_SERIES_EE:
		if ( idate >= GB_DATE(MAY,1997) ) {
			accrued_months = total_months;
		}
		else {
			if ( (idate >= GB_DATE(MAR,1993)) && (idate <= GB_DATE(APR,1995))
			     && ( total_months < 60 ) ) {
				accrued_months = total_months;
			}
			else {
				accrued_months = ( total_months / 6 ) * 6;
			}
		}
		break;

	case GB_SERIES_S:
		accrued_months = ( total_months / 6 ) * 6;
		break;

	case GB_SERIES_I:
		accrued_months = total_months;
		break;

	case GB_SERIES_E:
		if ( (idate >= GB_DATE(MAY,1952)) && (idate <= GB_DATE(JAN,1957)) ) {
			accrued_months = ( (total_months-2)/ 6 ) * 6 + 2;
			break;
		}
		if ( (idate >= GB_DATE(FEB,1957)) && (idate <= GB_DATE(MAY,1959)) ) {
			accrued_months = ( (total_months-5)/ 6 ) * 6 + 5;
			break;
		}
		if ( (idate >= GB_DATE(JUN,1959)) && (idate <= GB_DATE(NOV,1965)) ) {
			accrued_months = ( (total_months-3)/ 6 ) * 6 + 3;
			break;
		}
		if ( (idate >= GB_DATE(JUN,1969)) && (idate <= GB_DATE(NOV,1973)) ) {
			accrued_months = ( (total_months-4)/ 6 ) * 6 + 4;
			break;
		}
		accrued_months = ( total_months / 6 ) * 6;
		break;

	default:
		return GB_ERROR_BAD_SERIES;
	}

	if ( rdate >= mdate ) {
		*a1date = mdate;
	}
	else {
		*a1date = idate + accrued_months;
	}

	return GB_OK;
}

/***************************************************************************/
/* For a given series, issue date, redemption date, and maturity date,     */
/* determine the next date that the bond will increase in value.           */
/***************************************************************************/
gbStatus
gb_rules_get_next_accrual (gbSeries  series,
			   gbDate    idate,
			   gbDate    rdate,
			   gbDate    mdate,
			   gbDate   *a2date )
{
	gbDate   a1date;
	gint     total_months, accrual_step_months=6;
	gbStatus rval;

	total_months =   rdate - idate;

	if ( total_months < 0 ) {
		*a2date = 0;
		return GB_OK;
	}

	rval= gb_rules_get_last_accrual( series, idate, rdate, mdate, &a1date );
	if ( rval != GB_OK ) {
		return rval;
	}

	switch (series) {

	case GB_SERIES_EE:
		if ( idate >= GB_DATE(MAY,1997) ) {
			accrual_step_months = 1;
		}
		else {
			if ( (idate >= GB_DATE(MAR,1993)) && (idate <= GB_DATE(APR,1995))
			     && ( total_months < 60 ) ) {
				accrual_step_months = 1;
			}
			else {
				accrual_step_months = 6;
			}
		}
		break;

	case GB_SERIES_S:
		accrual_step_months = 6;
		break;

	case GB_SERIES_I:
		accrual_step_months = 6;
		break;

	case GB_SERIES_E:
		accrual_step_months = 6;
		break;

	default:
		return GB_ERROR_BAD_SERIES;
	}

	if ( (a1date + accrual_step_months) >= mdate ) {
		*a2date = mdate;
	}
	else {
		*a2date = a1date + accrual_step_months;
	}

	return GB_OK;
}

/***************************************************************************/
/* For given series, issue date, and maturity date, determine and set the  */
/* exchangeability flag.                                                   */
/***************************************************************************/
gbStatus
gb_rules_determine_exchangeability (gbSeries  series,
				    gbDate    idate,
				    gbDate    mdate,
				    gbDate    rdate,
				    gboolean *exchangeable_flag )
{
	gbDate sdate, mdate1;

	switch( series ) {
	case GB_SERIES_S:
	case GB_SERIES_I:
		*exchangeable_flag = FALSE;
		break;

	case GB_SERIES_E:
	case GB_SERIES_EE:
		sdate = idate + 6;
		mdate1 = mdate + 12;
		if ( ( rdate >= sdate ) && ( rdate <= mdate1 ) ) {
			*exchangeable_flag = TRUE;
		}
		else {
			*exchangeable_flag = FALSE;
		}
		break;
		
	default:
		return GB_ERROR_BAD_SERIES;
	}

	return GB_OK;
}

/***************************************************************************/
/* For given series, issue date, and redemption date, determine and set the*/
/* nopay flag.                                                             */
/***************************************************************************/
gbStatus
gb_rules_determine_nopay (gbSeries  series,
			  gbDate    idate,
			  gbDate    rdate,
			  gboolean *nopay_flag)
{
	switch( series ) {
	case GB_SERIES_S:
		*nopay_flag = (rdate - idate) < 6;
		break;

	case GB_SERIES_E:
		*nopay_flag = (rdate - idate) < 6;
		break;

	case GB_SERIES_I:
	case GB_SERIES_EE:
		if ( idate < GB_DATE(FEB,2003) ) {
			*nopay_flag = (rdate - idate) < 6;
		} else {
			*nopay_flag = (rdate - idate) < 12;
		}
		break;
		
	default:
		*nopay_flag = TRUE;
		return GB_ERROR_BAD_SERIES;
	}

	return GB_OK;
}


