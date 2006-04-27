/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  types.h:  Basic Savings Bond Types module header file.
 *
 *  Copyright (C) 2000, 2001  Jim Evins <evins@snaught.com>.
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

#ifndef __TYPES_H__
#define __TYPES_H__

#include <glib.h>

/*=======================================================================*/
/* Type Definitions                                                      */
/*=======================================================================*/

/*---------------------*/
/* Bond series         */
/*---------------------*/
typedef enum {
  GB_SERIES_E,
  GB_SERIES_S,
  GB_SERIES_EE,
  GB_SERIES_I,
} gbSeries;

/*---------------------*/
/* Date                */
/*---------------------*/
typedef gint gbDate; /* Year*12 + (Month-1) */

/*  some macros for dealing with gbDate dates */

#define GB_DATE(m,y) ( 12*(y) + (m) - 1 )

#define GB_YEAR(d) ( (d)/12 )
#define GB_MONTH(d) ( ((d)%12) + 1 )

#define JAN 1
#define FEB 2
#define MAR 3
#define APR 4
#define MAY 5
#define JUN 6
#define JUL 7
#define AUG 8
#define SEP 9
#define OCT 10
#define NOV 11
#define DEC 12


/*------------------------------------------------*/
/* Standard status codes, when dealing with bonds */
/*------------------------------------------------*/
typedef enum {
  GB_OK                               =   0,

  GB_ERROR_MEM_ALLOC                  = -10,

  GB_ERROR_PARSE_SN_NULL              = -20,
  GB_ERROR_PARSE_SN_EMPTY             = -21,
  GB_ERROR_PARSE_SN_BAD_DENOM         = -22,
  GB_ERROR_PARSE_SN_DIGITS            = -23,
  GB_ERROR_PARSE_SN_BAD_SERIES        = -24,

  GB_ERROR_PARSE_SERIES_NULL          = -30,
  GB_ERROR_PARSE_SERIES_EMPTY         = -31,
  GB_ERROR_PARSE_SERIES_BAD_SERIES    = -32,

  GB_ERROR_PARSE_DATE_NULL            = -40,
  GB_ERROR_PARSE_DATE_EMPTY           = -41,
  GB_ERROR_PARSE_DATE_FORMAT          = -42,
  GB_ERROR_PARSE_DATE_MONTH           = -43,
  GB_ERROR_PARSE_DATE_YEAR            = -44,

  GB_ERROR_BAD_SERIES_DENOM           = -50,
  GB_ERROR_BAD_ISSUE_DATE             = -51,
  GB_ERROR_BAD_SERIES                 = -52,
  GB_ERROR_BAD_REDEMPTION_DATE        = -53,

  GB_ERROR_DUPLICATE_SN               = -60,

  GB_ERROR_NO_REDEMPTION_DATA         = -70,
  GB_ERROR_PARSE_REDEMPTION_LINE      = -71,

  GB_ERROR_OPEN_XML_PARSE             = -80,

  GB_ERROR_OPEN_SBW_CANNOT_OPEN_FILE  = -90,
  GB_ERROR_OPEN_SBW_BAD_MAGIC         = -91,
  GB_ERROR_OPEN_SBW_PARSE             = -92,

  GB_ERROR_SAVE_XML_FILE              = -100,

  GB_ERROR_DELETE_BOND_NOT_FOUND      = -110,

} gbStatus;


/*=======================================================================*/
/* Function Prototypes                                                   */
/*=======================================================================*/

extern const gchar *
gb_series_fmt( gbSeries series );

extern gchar *
gb_date_fmt( gbDate date );

extern gchar *
gb_value_fmt( gdouble value, gboolean cent_flag );

extern gbStatus
gb_serial_number_parse( gchar *sn, gbSeries *series, gdouble *denom );

extern gbStatus
gb_series_parse( gchar *string, gbSeries *series );

extern gbStatus
gb_date_parse( const gchar *string, gbDate *date );

extern gbStatus
gb_determine_yield( gdouble issue_price, gdouble value,
		    gbDate idate, gbDate adate,
		    gdouble *yield );

#endif /* __TYPES_H__ */


