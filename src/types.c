/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  types.c:  Basic Savings Bond Types module.
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


#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "types.h"

/*=========================================================================*/
/* Convert an gbSERIES to a printable string.                              */
/*=========================================================================*/
const gchar *
gb_series_fmt (gbSeries series)
{
	switch (series) {
	case GB_SERIES_E:  return "E";
	case GB_SERIES_S:  return "S";
	case GB_SERIES_EE: return "EE";
	case GB_SERIES_I:  return "I";
	default:      return "?";
	}
}

/*=========================================================================*/
/* Convert a gbDATE to a printable string.                                 */
/*=========================================================================*/
gchar *
gb_date_fmt (gbDate date)
{
	return g_strdup_printf ("%2d/%4d", GB_MONTH(date), GB_YEAR(date));
}

/*=========================================================================*/
/* Convert a gbVALUE to a printable string.  (Only supports USD, which     */
/* should be ok since this is for U.S. savings bonds only.)                */
/*=========================================================================*/
gchar *
gb_value_fmt (gdouble value, gboolean cent_flag)
{
	gchar string[256], *p=string;
	gint npow;
	gint n = 0, i, digit;
	gdouble dpow;

	if ( value <= 0.0 ) {
		if ( cent_flag ) {
			return g_strdup( "$0.00" );
		}
		else {
			return g_strdup( "$0" );
		}
	}
	else {
		value += 0.005; /* Round to nearest cent */
		npow = (int)(log10( value ));
		if ( npow < 0 ) npow = 0;

		p[n++] = '$';
		dpow = pow( 10.0, (double)npow );
		for ( i=npow; (i>=0) && (n<255); i-- ) {
			digit = (int)( value / dpow );
			p[n++] = digit + '0';
			value = value - ((double)digit * dpow);
			dpow = dpow / 10.0;
			if ( i && ((i % 3) == 0) ) {
				p[n++] = ',';
			}
		}
		if ( cent_flag ) {
			p[n++] = '.';
			for ( i=0; (i<2) && (n<255); i++ ) {
				digit = (int)( value / dpow );
				p[n++] = digit + '0';
				value = value - ((double)digit * dpow);
				dpow = dpow / 10.0;
			}
		}
	}
	
	p[n] = 0;

	return g_strdup( string );
}

/*=========================================================================*/
/* Parse serial number, extract series and denomination.                   */
/*=========================================================================*/
extern gbStatus
gb_serial_number_parse (gchar *sn, gbSeries *series, gdouble *denom)
{
	gchar *p;
	gint digits;

	/*-----------------------------------------*/
	/* Check for NULL pointer and empty string */
	/*-----------------------------------------*/
	if ( sn == NULL ) {
		return GB_ERROR_PARSE_SN_NULL;
	}
	if ( !sn[0] ) {
		return GB_ERROR_PARSE_SN_EMPTY;
	}

	/*--------------------------------*/
	/* Check and extract denomination */
	/*--------------------------------*/
	sn[0] = toupper( sn[0] );
	switch ( sn[0] ) {
	case 'Q': *denom =    25; break;
	case 'L': *denom =    50; break;
	case 'K': *denom =    75; break;
	case 'C': *denom =   100; break;
	case 'R': *denom =   200; break;
	case 'D': *denom =   500; break;
	case 'M': *denom =  1000; break;
	case 'V': *denom =  5000; break;
	case 'X': *denom = 10000; break;
	default:
		return GB_ERROR_PARSE_SN_BAD_DENOM;
	}

	/*-------------------------------------------------------------------*/
	/* Check number of digits, and position p at series at end of number */
	/*-------------------------------------------------------------------*/
	digits = 0;
	for ( p = &sn[1]; isdigit(*p) && (digits < 11); p++ ) {
		digits++;
	}
	if ( ( digits < 1 ) ) {
		return GB_ERROR_PARSE_SN_DIGITS;
	}
	if ( ( digits > 10 ) ) {
		return GB_ERROR_PARSE_SN_DIGITS;
	}

	/*--------------------------------------------------------*/
	/* Check and extract series, p should now point at series */
	/*--------------------------------------------------------*/
	p[0] = toupper( p[0] );
	switch ( p[0] ) {
	case 0: /* Empty => Series S */
		*series = GB_SERIES_S;
		break;
		
	case 'E': /* E => Series E or EE */
		p[1] = toupper( p[1] );
		switch ( p[1] ) {
		case 0: /* Just E = > Series E */
			*series = GB_SERIES_E;
			break;
		case 'E':
			if ( p[2] == 0 ) {
				*series = GB_SERIES_EE; /* EE => Series EE */
			}
			else {
				return GB_ERROR_PARSE_SN_BAD_SERIES; /* Extra character(s) */
			}
			break;
		default: /* E? */
			return GB_ERROR_PARSE_SN_BAD_SERIES;
			break;
		}
		break;

	case 'I':
		if ( p[1] == 0 ) {
			*series = GB_SERIES_I; /* I => Series I */
		}
		else {
			return GB_ERROR_PARSE_SN_BAD_SERIES; /* Extra character(s) */
		}
		break;

	default:
		return GB_ERROR_PARSE_SN_BAD_SERIES; /* ? */
		break;
	}

	return GB_OK;
}

/*=========================================================================*/
/* Parse series string and extract a gbSeries.                             */
/*=========================================================================*/
extern gbStatus
gb_series_parse (gchar *string, gbSeries *series)
{

	/*-----------------------------------------*/
	/* Check for NULL pointer and empty string */
	/*-----------------------------------------*/
	if ( string == NULL ) {
		return GB_ERROR_PARSE_SERIES_NULL;
	}
	if ( !string[0] ) {
		return GB_ERROR_PARSE_SERIES_EMPTY;
	}

	/*--------------------------*/
	/* Check and extract series */
	/*--------------------------*/
	string[0] = toupper( string[0] );
	switch ( string[0] ) {

	case 'E': /* E => Series E or EE */
		string[1] = toupper( string[1] );
		switch ( string[1] ) {
		case 0: /* Just E = > Series E */
			*series = GB_SERIES_E;
			break;
		case 'E':
			if ( string[2] == 0 ) {
				*series = GB_SERIES_EE; /* EE => Series EE */
			}
			else {
				return GB_ERROR_PARSE_SERIES_BAD_SERIES; /* Extra character(s) */
			}
			break;
		default: /* E? */
			return GB_ERROR_PARSE_SERIES_BAD_SERIES;
			break;
		}
		break;

	case 'I':
		if ( string[1] == 0 ) {
			*series = GB_SERIES_I; /* I => Series I */
		}
		else {
			return GB_ERROR_PARSE_SERIES_BAD_SERIES; /* Extra character(s) */
		}
		break;

	case 'S': /* Empty => Series S */
		if ( string[1] == 0 ) {
			*series = GB_SERIES_S;
		}
		else {
			return GB_ERROR_PARSE_SERIES_BAD_SERIES; /* Extra character(s) */
		}
		break;

	default:
		return GB_ERROR_PARSE_SERIES_BAD_SERIES; /* ? */
		break;
	}

	return GB_OK;
}

/*=========================================================================*/
/* Parse a date string and extract a gbDate.                               */
/*=========================================================================*/
extern gbStatus
gb_date_parse (const gchar *string, gbDate *date)
{
	gint n, month, year;

	/*-----------------------------------------*/
	/* Check for NULL pointer and empty string */
	/*-----------------------------------------*/
	if ( string == NULL ) {
		return GB_ERROR_PARSE_DATE_NULL;
	}
	if ( !string[0] ) {
		return GB_ERROR_PARSE_DATE_EMPTY;
	}

	n = sscanf( string, "%d/%d", &month, &year );
	if ( n != 2 ) {
		return GB_ERROR_PARSE_DATE_FORMAT;
	}
	if ( (month < 1) || (month >12) ) {
		return GB_ERROR_PARSE_DATE_MONTH;
	}
	if ( year < 1940 ) {
		return GB_ERROR_PARSE_DATE_YEAR;
	}

	*date = GB_DATE( month, year );

	return GB_OK;
}

/*=========================================================================*/
/* Determine the effective yield of a bond, given the initial value, its   */
/* current value, its issue date, and the last accrual date.               */
/*=========================================================================*/
gbStatus
gb_determine_yield (gdouble issue_price, gdouble value,
		    gbDate idate, gbDate adate,
		    gdouble *yield)
{
	gdouble months;

	months =   adate - idate;

	if ( months > 0 ) {
#ifdef ANNUAL_YIELD
		/* Annual Yield */
		*yield = (exp( (12.0/months) * log(value/issue_price) ) - 1.0) * 100.0;
#else
		/* Semi-Annual Yield, i.e. 6 months */
		*yield = 2.0*(exp( (6.0/months) * log(value/issue_price) ) - 1.0) * 100.0;
#endif
	}
	else {
		*yield = 0.0;
	}

	return GB_OK;

}

