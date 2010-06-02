/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  table-model.c:  gbonds redemption table model module
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

#include <gnome.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "table-model.h"
#include "marshal.h"
#include "rules.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define REDEMPTION_DATA_DIR gnome_program_locate_file (NULL,\
					               GNOME_FILE_DOMAIN_APP_DATADIR,\
					               "gbonds/",\
					               FALSE, NULL)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct _RedemptionEntry {

	gbSeries  series;       /* Bond Series */
	gint      r_year;       /* Redemption Year (see gbDate) */
	gint      r_month;      /* Redemption Month (see gbDate) */
	gint      i_year;       /* Issue Year (see gbDate) */
	gdouble  r_value[12];   /* Redemption value vs. Issue month ( 0 = NO PAY ) */

} RedemptionEntry;

struct _gbTableModelPrivate {
	GList    *table; /* List of redemption entries */
};

enum {
	CHANGED,
	LAST_SIGNAL
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = {0};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void   gb_table_model_class_init          (gbTableModelClass *klass);
static void   gb_table_model_instance_init       (gbTableModel      *doc);
static void   gb_table_model_finalize            (GObject           *object);


static gchar *get_home_data_dir                  (void);


static GList *append_tables_from_dir             (GList             *table,
						  const gchar       *dirname);

static GList *append_table_from_file             (GList             *table,
						  const gchar       *fname);

static GList *append_entry_from_line             (GList             *table,
						  const gchar       *line);

static void   free_table                         (GList             *table);

static gint   extract_int                        (const gchar       *ptr,
						  gint               digits );




/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gb_table_model_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo info = {
			sizeof (gbTableModelClass),
			NULL,
			NULL,
			(GClassInitFunc) gb_table_model_class_init,
			NULL,
			NULL,
			sizeof (gbTableModel),
			0,
			(GInstanceInitFunc) gb_table_model_instance_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "gbTableModel", &info, 0);
	}

	return type;
}

static void
gb_table_model_class_init (gbTableModelClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gb_debug (DEBUG_TABLE, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gb_table_model_finalize;

	signals[CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (gbTableModelClass, changed),
			      NULL, NULL,
			      gb_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gb_debug (DEBUG_TABLE, "END");
}

static void
gb_table_model_instance_init (gbTableModel *table_model)
{
	gb_debug (DEBUG_TABLE, "START");

	table_model->private = g_new0 (gbTableModelPrivate, 1);

	gb_debug (DEBUG_TABLE, "END");
}

static void
gb_table_model_finalize (GObject *object)
{
	gbTableModel *table_model;

	gb_debug (DEBUG_TABLE, "START");

	g_return_if_fail (object && GB_IS_TABLE_MODEL (object));

	table_model = GB_TABLE_MODEL (object);

	free_table ( table_model->private->table );
	g_free (table_model->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gb_debug (DEBUG_TABLE, "END");
}

/*****************************************************************************/
/* New table model object.                                                   */
/*****************************************************************************/
GObject *
gb_table_model_new (void)
{
	gbTableModel *table_model;

	gb_debug (DEBUG_TABLE, "START");

	table_model = g_object_new (gb_table_model_get_type(), NULL);

	gb_table_model_update (table_model);

	gb_debug (DEBUG_TABLE, "END");

	return G_OBJECT (table_model);
}

/*****************************************************************************/
/* Update table model from disk.                                             */
/*****************************************************************************/
void
gb_table_model_update (gbTableModel *table_model)
{
	gchar *home_data_dir;
	GList *table;

	gb_debug (DEBUG_TABLE, "START");

	g_return_if_fail (table_model && GB_IS_TABLE_MODEL (table_model));

	if ( table_model->private->table != NULL ) {
		free_table (table_model->private->table);
		table_model->private->table = NULL;
	}

	table = NULL;

	table = append_tables_from_dir( table, REDEMPTION_DATA_DIR );

	home_data_dir = get_home_data_dir();
	table = append_tables_from_dir( table, home_data_dir );
	g_free( home_data_dir );

	if ( table == NULL ) {
		g_warning( _("No redemption data found!") );
	}

	table_model->private->table = table;

	g_signal_emit (G_OBJECT(table_model), signals[CHANGED], 0);

	gb_debug (DEBUG_TABLE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  get '~/.gbonds' directory path.                                */
/*--------------------------------------------------------------------------*/
static gchar *
get_home_data_dir (void)
{
	gchar *dir = gnome_util_prepend_user_home( ".gbonds" );

	/* Try to create ~/.gbonds directory.  If it already exists, no problem. */
	mkdir( dir, 0775 );

	return dir;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all redemption table files in the given directory.        */
/* Append to redemption table.                                              */
/*--------------------------------------------------------------------------*/
static GList *
append_tables_from_dir (GList       *table,
			const gchar *dirname)
{
	DIR *dp;
	struct dirent *d_entry;
	gchar *filename, *extension;
	gchar *full_filename = NULL;

	gb_debug (DEBUG_TABLE, "START");

	if ( dirname == NULL ) return NULL;

	dp = opendir( dirname );
	if ( dp == NULL ) return NULL;

	while ( (d_entry = readdir(dp)) != NULL ) {

		filename = d_entry->d_name;
		extension = strrchr( filename, '.' );
		
		if ( extension != NULL ) {

			if ( g_strcasecmp( extension, ".asc" ) == 0 ) {
	
				gb_debug (DEBUG_TABLE, "d=\"%s\", f=\"%s\"",
					  dirname, filename);

				full_filename = g_build_filename (dirname, filename, NULL);
				table = append_table_from_file (table, full_filename);
				g_free( full_filename );

			}

		}

	}
  
	closedir( dp );

	gb_debug (DEBUG_TABLE, "END");

	return table;
}

/*=========================================================================*/
/* Read a redemption table from a file, and add it to the given table.     */
/* New files are released by the U.S. Treasury every six months            */
/* (see http://publicdebt.treas.gov/).                                     */
/*=========================================================================*/
static GList *
append_table_from_file (GList       *table,
			const gchar *fname )
{
	FILE  *fp;
	gchar  line[256];

	gb_debug (DEBUG_TABLE, "START");
	gb_debug (DEBUG_TABLE, "f=\"%s\"", fname);

	fp = fopen( fname, "r" );
	if ( fp == NULL ) {
		return table;
	}

	while ( fgets( line, 255, fp ) ) {
		table = append_entry_from_line (table, line);
	}

	fclose( fp );

	gb_debug (DEBUG_TABLE, "END");

	return table;
}

/*=========================================================================*/
/* Add a new redemption table entry from the given string.  This string is */
/* a line from a redemption table file (sbXXXXXX.asc).  New files are      */
/* released by the U.S. Treasury every six months (see                     */
/* http://publicdebt.treas.gov/).                                          */
/*=========================================================================*/
static GList *
append_entry_from_line (GList       *table,
			const gchar *line)
{
	RedemptionEntry *p_entry;
	gint             i, i_char, n_char;

	/* Allocate a new table entry */
	p_entry = g_new0( RedemptionEntry, 1 );
	if ( p_entry == NULL ) {
		g_warning( _("Cannot allocate redemption table entry.") );
		return table;
	}

	switch (line[0]) {
	case 'I': p_entry->series = GB_SERIES_I;  break;
	case 'E': p_entry->series = GB_SERIES_E;  break;
	case 'N': p_entry->series = GB_SERIES_EE; break;
	case 'S': p_entry->series = GB_SERIES_S;  break;
	default:
		g_free( p_entry );
		p_entry = NULL;
		g_warning (_("Problem parsing redemption table entry, unknown series."));
		return table;
	}

	p_entry->r_year  = extract_int( &line[1], 4 );
	p_entry->r_month = extract_int( &line[5], 2 );
	p_entry->i_year  = extract_int( &line[7], 4 );
	if ( !p_entry->r_year || !p_entry->r_month || !p_entry->i_year ) {
		g_free( p_entry );
		p_entry = NULL;
		g_warning (_("Problem parsing redemption table entry, bad date strings."));
		return table;
	}

	i_char = 11;
	n_char = strlen( line ) - 1;
	for ( i=0; i<12; i++ ) {
		if ( i_char < n_char ) {
			p_entry->r_value[i] =
				((gdouble)extract_int(&line[i_char],6)) / 100.0;
		}
		else {
			p_entry->r_value[i] = 0.0;
		}
		i_char += 6;
	}

	/* Add new table entry to top of table */
	table = g_list_prepend( table, p_entry );

	return table;
}

/*=========================================================================*/
/* PRIVATE.  Free up previously allocated redemption table.                */
/*=========================================================================*/
void
free_table (GList *table)
{
	GList *p;

	for ( p=table; p!=NULL; p=p->next ) {
		g_free( p->data );
		p->data = NULL;
	}
	g_list_free( table );

	return;
}

/*=========================================================================*/
/* PRIVATE.  Converts a given n ASCII characters to the integer they       */
/* represent.  Returns zero, if 1st char is non-numeric.                   */
/*=========================================================================*/
static gint
extract_int (const gchar *ptr,
	     gint         digits )
{
	gint i, ret;

	if ( !isdigit(ptr[0]) ) {
		return 0;
	}

	ret = (*ptr++ - '0');
	for ( i=1; i<digits; i++ ) {
		ret = (ret*10) + (*ptr++ - '0');
	}

	return ret;
}

/*****************************************************************************/
/* Get a list of formatted valid redemption dates from redemption table.     */
/*****************************************************************************/
GList *
gb_table_model_get_rdate_list (gbTableModel       *table_model,
			       gint                max_dates )
{
	gbDate           date_min, date_max, date, limit_date_min;
	gint             i, n_dates;
	GList           *rtable = NULL;
	gchar           *date_str;

	if (!table_model->private->table) {
		return NULL;
	}

	date_min = gb_table_model_get_rdate_min (table_model);
	date_max = gb_table_model_get_rdate_max (table_model);

	/* Limit dates? */
	if ( max_dates > 0 ) {
		limit_date_min = date_max - max_dates + 1;
		if ( limit_date_min > date_min ) {
			date_min = limit_date_min;
		}
	}

	/* Assume all contiguous dates are present. */
	n_dates = date_max - date_min + 1;
	for ( i=0; i<n_dates ; i++ ) {
		date = date_min + i;
		date_str = gb_date_fmt (date);
		rtable = g_list_append (rtable, date_str);
	}

	return rtable;
}

/*****************************************************************************/
/* Free a previously allocated rdate list.                                   */
/*****************************************************************************/
void
gb_table_model_free_rdate_list (GList *rtable)
{
	GList *p;

	for ( p=rtable; p!=NULL; p=p->next ) {
		g_free (p->data);
		p->data = NULL;
	}
	g_list_free (rtable);
	return;
}

/*****************************************************************************/
/* Get today's date, even if we don't have any redemption data.              */
/*****************************************************************************/
gbDate
gb_table_model_get_rdate_today (void)
{
	gbDate     date;
	time_t     t;
	struct tm *tm;

	t = time(NULL); tm = localtime(&t);
	date = GB_DATE( tm->tm_mon+1, tm->tm_year+1900 );

	return date;
}

/*****************************************************************************/
/* Get valid redemption date most closely matching today's date.             */
/*****************************************************************************/
gbDate
gb_table_model_get_best_rdate_today (gbTableModel       *table_model)
{
	gbDate date_max, date_min, date;

	date_min = gb_table_model_get_rdate_min (table_model);
	date_max = gb_table_model_get_rdate_max (table_model);

	date = gb_table_model_get_rdate_today();
	if ( date < date_min ) date = date_min;
	if ( date > date_max ) date = date_max;

	return date;
}

/*****************************************************************************/
/* Get maximum valid redemption date.                                        */
/*****************************************************************************/
gbDate
gb_table_model_get_rdate_max (gbTableModel       *table_model)
{
	GList           *p;
	RedemptionEntry *p_entry;
	gbDate           date_max, date;

	p = table_model->private->table;
	if ( p == NULL ) {
		return GB_DATE(0,0);
	}
	p_entry = (RedemptionEntry *)p->data;
	date_max = GB_DATE( p_entry->r_month, p_entry->r_year );

	for ( p = p->next; p != NULL; p = p->next ) {
		p_entry = (RedemptionEntry *)p->data;
		date = GB_DATE( p_entry->r_month, p_entry->r_year );
		if ( date > date_max ) date_max = date;
	}

	return date_max;
}

/*****************************************************************************/
/* Get minimum valid redemption date.                                        */
/*****************************************************************************/
gbDate
gb_table_model_get_rdate_min (gbTableModel       *table_model)
{
	GList           *p;
	RedemptionEntry *p_entry;
	gbDate           date_min, date;

	p = table_model->private->table;
	if ( p == NULL ) {
		return GB_DATE(0,0);
	}
	p_entry = (RedemptionEntry *)p->data;
	date_min = GB_DATE( p_entry->r_month, p_entry->r_year );

	for ( p = p->next; p != NULL; p = p->next ) {
		p_entry = (RedemptionEntry *)p->data;
		date = GB_DATE( p_entry->r_month, p_entry->r_year );
		if ( date < date_min ) date_min = date;
	}

	return date_min;
}

/*****************************************************************************/
/* For a given bond series and redemption date, determine the current        */
/* value of bond.                                                            */
/*****************************************************************************/
gdouble
gb_table_model_get_value (gbTableModel       *table_model,
			  gbSeries            series,
			  gdouble             denom,
			  gdouble             issue_price,
			  gbDate              idate,
			  gbDate              rdate,
			  gboolean           *nopay_flag)
{
	GList           *p;
	RedemptionEntry *p_entry;
	gdouble          rvalue;

	for ( p = table_model->private->table; p != NULL; p = p->next ) {
		p_entry = (RedemptionEntry *)p->data;
		if ( series == p_entry->series ) {
			if ( GB_YEAR(rdate) == p_entry->r_year ) {
				if ( GB_MONTH(rdate) == p_entry->r_month ) {
					if ( GB_YEAR(idate) == p_entry->i_year ) {
						rvalue = p_entry->r_value[GB_MONTH(idate)-1];
						if ( rvalue > 0.0 ) {
							rvalue *= denom/25.0;
							*nopay_flag = FALSE;
						}
						else {
							rvalue = issue_price;
							*nopay_flag = TRUE;
						}
						return rvalue;
					}
				}
			}
		}
	}

	gb_rules_determine_nopay (series, idate, rdate, nopay_flag);
	if ( *nopay_flag ) {
		return issue_price;
	}

	g_warning (_("No matching redemption value"));
	*nopay_flag = TRUE;
	return 0.0;
}

