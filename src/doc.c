/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  doc.c:  gbonds document module
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

#include "doc.h"
#include "marshal.h"
#include "util.h"
#include "rules.h"
#include "table.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _gbDocPrivate {

	gchar    *title;

	gchar    *filename;
	gint      untitled_instance;
	gboolean  modified_flag;

};

enum {
	CHANGED,
	NAME_CHANGED,
	MODIFIED_CHANGED,
	LAST_SIGNAL
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = {0};

static guint untitled = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gb_doc_class_init    (gbDocClass       *klass);
static void gb_doc_instance_init (gbDoc            *doc);
static void gb_doc_finalize      (GObject          *object);

static gint insert_compare_fct   (const gbDocBond  *a,
				  const gbDocBond  *b);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gb_doc_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static GTypeInfo info = {
			sizeof (gbDocClass),
			NULL,
			NULL,
			(GClassInitFunc) gb_doc_class_init,
			NULL,
			NULL,
			sizeof (gbDoc),
			0,
			(GInstanceInitFunc) gb_doc_instance_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "gbDoc", &info, 0);
	}

	return type;
}

static void
gb_doc_class_init (gbDocClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gb_debug (DEBUG_DOC, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gb_doc_finalize;

	signals[CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (gbDocClass, changed),
			      NULL, NULL,
			      gb_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[NAME_CHANGED] =
		g_signal_new ("name_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (gbDocClass, name_changed),
			      NULL, NULL,
			      gb_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[MODIFIED_CHANGED] =
		g_signal_new ("modified_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (gbDocClass, modified_changed),
			      NULL, NULL,
			      gb_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gb_debug (DEBUG_DOC, "END");
}

static void
gb_doc_instance_init (gbDoc *doc)
{
	gb_debug (DEBUG_DOC, "START");

	doc->private = g_new0 (gbDocPrivate, 1);

	gb_debug (DEBUG_DOC, "END");
}

static void
gb_doc_finalize (GObject *object)
{
	gbDoc *doc;
	GList *p;

	gb_debug (DEBUG_DOC, "START");

	g_return_if_fail (object && GB_IS_DOC (object));

	doc = GB_DOC (object);

	for (p=doc->list; p != NULL; p=p->next) {
		gb_doc_bond_free (p->data);
		p->data = NULL;
	}
	g_list_free (doc->list);

	g_free (doc->private->title);
	g_free (doc->private->filename);
	g_free (doc->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gb_debug (DEBUG_DOC, "END");
}

GObject *
gb_doc_new (void)
{
	gbDoc *doc;

	gb_debug (DEBUG_DOC, "START");

	doc = g_object_new (gb_doc_get_type(), NULL);

	doc->private->modified_flag = FALSE;

	gb_debug (DEBUG_DOC, "END");

	return G_OBJECT (doc);
}


/****************************************************************************/
/* return filename.                                                         */
/****************************************************************************/
gchar *
gb_doc_get_filename (gbDoc *doc)
{
	gb_debug (DEBUG_DOC, "");

	return g_strdup ( doc->private->filename );
}

/****************************************************************************/
/* return short filename.                                                   */
/****************************************************************************/
gchar *
gb_doc_get_short_name (gbDoc *doc)
{
	gb_debug (DEBUG_DOC, "");

	if ( doc->private->filename == NULL ) {

		if ( doc->private->untitled_instance == 0 ) {
			doc->private->untitled_instance = ++untitled;
		}

		return g_strdup_printf ( "%s %d", _("Untitled"),
					 doc->private->untitled_instance );

	} else {
		gchar *temp_name, *short_name;

		temp_name = g_path_get_basename ( doc->private->filename );
		short_name = gb_util_remove_extension (temp_name);
		g_free (temp_name);

		return short_name;
	}
}

/****************************************************************************/
/* Is doc modified?                                                         */
/****************************************************************************/
gboolean
gb_doc_is_modified (gbDoc *doc)
{
	gb_debug (DEBUG_DOC, "return %d", doc->private->modified_flag);
	return doc->private->modified_flag;
}

/****************************************************************************/
/* Is doc untitled?                                                         */
/****************************************************************************/
gboolean
gb_doc_is_untitled (gbDoc *doc)
{
	gb_debug (DEBUG_DOC, "return %d",(doc->private->filename == NULL));
	return (doc->private->filename == NULL);
}

/****************************************************************************/
/* Can undo?                                                                */
/****************************************************************************/
gboolean
gb_doc_can_undo (gbDoc *doc)
{
	return FALSE;
}


/****************************************************************************/
/* Can redo?                                                                */
/****************************************************************************/
gboolean
gb_doc_can_redo (gbDoc *doc)
{
	return FALSE;
}


/****************************************************************************/
/* Set filename.                                                            */
/****************************************************************************/
void
gb_doc_set_filename (gbDoc     *doc,
		       const gchar *filename)
{
	doc->private->filename = g_strdup (filename);

	g_signal_emit (G_OBJECT(doc), signals[NAME_CHANGED], 0);
}

/****************************************************************************/
/* Clear modified flag.                                                     */
/****************************************************************************/
void
gb_doc_clear_modified (gbDoc *doc)
{

	if ( doc->private->modified_flag ) {

		doc->private->modified_flag = FALSE;

		g_signal_emit (G_OBJECT(doc), signals[MODIFIED_CHANGED], 0);
	}

}

/****************************************************************************/
/* Add bond to inventory.                                                   */
/****************************************************************************/
gbStatus
gb_doc_add_bond (gbDoc         *doc,
		 gbDocBond     *p_bond)
{
	GList     *li;
	gbDocBond *p_bond_li;

	gb_debug (DEBUG_DOC, "START");

	/* If bond has a serial number, check for duplicate serial numbers */
	/* Bonds from imported SBW inventories may not have SNs */
	if ( p_bond->sn != NULL ) {

		for ( li=doc->list; li!=NULL; li=li->next ) {
			p_bond_li = (gbDocBond *)li->data;
			if ( p_bond_li->sn != NULL ) {
				if ( strcmp( p_bond->sn, p_bond_li->sn ) == 0 ) {
					g_warning( _("Duplicate SN = %s"), p_bond->sn );
					return GB_ERROR_DUPLICATE_SN;
				}
			}
		}
		
	}

	doc->list = g_list_insert_sorted( doc->list, p_bond,
					  (GCompareFunc)insert_compare_fct );

	gb_debug (DEBUG_DOC, "Added bond %s @ list = %p", p_bond->sn, doc->list);

	g_signal_emit (G_OBJECT(doc), signals[CHANGED], 0);

	if ( !doc->private->modified_flag ) {
		doc->private->modified_flag = TRUE;
		g_signal_emit (G_OBJECT(doc), signals[MODIFIED_CHANGED], 0);
	}

	gb_debug (DEBUG_DOC, "END");

	return GB_OK;
}

/*-------------------------------------------------------------------------*/
/* PRIVATE.  Compare two bonds for sorting by issue date, and sn.          */
/*-------------------------------------------------------------------------*/
static gint
insert_compare_fct (const gbDocBond  *a,
		    const gbDocBond  *b)
{
	if ( a->idate > b->idate ) {
		return +1;
	}
	else {
		if ( a->idate < b->idate ) {
			return -1;
		}
		else {
			if ( a->sn == NULL ) return +1;
			if ( b->sn == NULL ) return -1;
			return strcmp( b->sn, a->sn );
		}
	}
}

/****************************************************************************/
/* Delete bond from inventory.                                              */
/****************************************************************************/
gbStatus
gb_doc_delete_bond (gbDoc         *doc,
		    gbDocBond     *p_bond)
{
	GList *p;

	gb_debug (DEBUG_DOC, "START");

	p = g_list_find( doc->list, p_bond );
	if ( p != NULL ) {
		doc->list = g_list_remove_link( doc->list, p );
		gb_doc_bond_free( p->data );
		p->data = NULL;
		g_list_free_1( p );

		g_signal_emit (G_OBJECT(doc), signals[CHANGED], 0);

		if ( !doc->private->modified_flag ) {
			doc->private->modified_flag = TRUE;
			g_signal_emit (G_OBJECT(doc), signals[MODIFIED_CHANGED], 0);
		}

	}
	else {
		g_warning( _("Bond not found!") );
		return GB_ERROR_DELETE_BOND_NOT_FOUND;
	}

	gb_debug (DEBUG_DOC, "END");

	return GB_OK;
}

/****************************************************************************/
/* Get inventory title.                                                     */
/****************************************************************************/
gchar *
gb_doc_get_title (gbDoc         *doc)
{
	return g_strdup( doc->private->title );
}

/****************************************************************************/
/* Set inventory title.                                                     */
/****************************************************************************/
void
gb_doc_set_title (gbDoc         *doc,
		  const gchar   *title)
{
	g_free( doc->private->title );
	doc->private->title = g_strdup( title );

	g_signal_emit (G_OBJECT(doc), signals[CHANGED], 0);

	g_signal_emit (G_OBJECT(doc), signals[NAME_CHANGED], 0);

	if ( !doc->private->modified_flag ) {
		doc->private->modified_flag = TRUE;
		g_signal_emit (G_OBJECT(doc), signals[MODIFIED_CHANGED], 0);
	}
}

/****************************************************************************/
/* Create a new bond, from series, issue date, and denom.  Will also       */
/* store SN, but will not parse it.                                        */
/****************************************************************************/
gbDocBond *
gb_doc_bond_new (gchar         *series,
		 const gchar   *idate,
		 gdouble        denom,
		 const gchar   *sn,
		 gbStatus      *status)
{
	gbDocBond *p_bond=NULL;

	gb_debug (DEBUG_DOC, "START");

	/*-------------------------------------*/
	/* Allocate storage for bond structure */
	/*-------------------------------------*/
	p_bond = g_new0( gbDocBond, 1 );
	if ( p_bond == NULL ) {
		g_warning( _("Cannot allocate bond!") );
		*status = GB_ERROR_MEM_ALLOC;
		return NULL;
	}

	/*---------------------------------------------------------*/
	/* Set fields                                              */
	/*---------------------------------------------------------*/
	p_bond->denom = denom;
	p_bond->sn = g_strdup( sn );

	/*--------------------------------*/
	/* Parse and set series field     */
	/*--------------------------------*/
	*status = gb_series_parse( series, &p_bond->series );
	if ( *status != GB_OK ) {
		g_warning( _("Bad series") );
		gb_doc_bond_free( p_bond );
		p_bond = NULL;
		return NULL;
	}

	/*--------------------------------*/
	/* Parse and set issue date field */
	/*--------------------------------*/
	*status = gb_date_parse( idate, &p_bond->idate );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot parse issue date") );
		gb_doc_bond_free( p_bond );
		p_bond = NULL;
		return NULL;
	}

	/*--------------------------------------------------------*/
	/* Make sure series/denomination combination are valid.   */
	/*--------------------------------------------------------*/
	*status = gb_rules_test_series_denom( p_bond->series, p_bond->denom );
	if ( *status != GB_OK ) {
		g_warning( _("Bad denomination") );
		gb_doc_bond_free( p_bond );
		p_bond = NULL;
		return NULL;
	}

	/*-------------------------------------------*/
	/* Make sure series and issue date are valid */
	/*-------------------------------------------*/
	*status = gb_rules_test_issue( p_bond->series, p_bond->idate );
	if ( *status != GB_OK ) {
		g_warning( _("Bad issue date") );
		gb_doc_bond_free( p_bond );
		p_bond = NULL;
		return NULL;
	}

	/*-----------------------*/
	/* Determine issue price */
	/*-----------------------*/
	*status = gb_rules_determine_issue (p_bond->series, p_bond->denom, &p_bond->issue);
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine issue price") );
		gb_doc_bond_free( p_bond );
		p_bond = NULL;
		return NULL;
	}

	/*-------------------------------*/
	/* Determine final maturity date */
	/*-------------------------------*/
	*status = gb_rules_determine_maturity (p_bond->series, p_bond->idate,
					       &p_bond->mdate);
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine maturity data") );
		gb_doc_bond_free( p_bond );
		p_bond = NULL;
		return NULL;
	}

	*status = GB_OK;

	gb_debug (DEBUG_DOC, "END");

	return p_bond;
}

/****************************************************************************/
/* Create a new bond, from ascii serial number and issue date.              */
/****************************************************************************/
gbDocBond     *gb_doc_bond_new_from_sn_idate  (gchar         *sn,
					       const gchar   *idate,
					       gbStatus      *status)
{
	gbDocBond *p_bond=NULL;

	gb_debug (DEBUG_DOC, "START");

	/*-------------------------------------*/
	/* Allocate storage for bond structure */
	/*-------------------------------------*/
	p_bond = g_new0( gbDocBond, 1 );
	if ( p_bond == NULL ) {
		g_warning( _("Cannot allocate bond!") );
		*status = GB_ERROR_MEM_ALLOC;
		return NULL;
	}

	/*---------------------------------------------------------*/
	/* Parse serial number, set series and denomination fields */
	/*---------------------------------------------------------*/
	*status = gb_serial_number_parse( sn, &p_bond->series, &p_bond->denom );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot parse SN") );
		gb_doc_bond_free( p_bond );
		return NULL;
	}
	p_bond->sn = g_strdup( sn );

	/*--------------------------------*/
	/* Parse and set issue date field */
	/*--------------------------------*/
	*status = gb_date_parse( idate, &p_bond->idate );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot parse issue date") );
		gb_doc_bond_free( p_bond );
		return NULL;
	}

	/*--------------------------------------------------------*/
	/* Make sure series/denomination combination are valid.   */
	/*--------------------------------------------------------*/
	*status = gb_rules_test_series_denom( p_bond->series, p_bond->denom );
	if ( *status != GB_OK ) {
		g_warning( _("Bad denomination") );
		gb_doc_bond_free( p_bond );
		return NULL;
	}

	/*-------------------------------------------*/
	/* Make sure series and issue date are valid */
	/*-------------------------------------------*/
	*status = gb_rules_test_issue( p_bond->series, p_bond->idate );
	if ( *status != GB_OK ) {
		g_warning( _("Bad issue date") );
		gb_doc_bond_free( p_bond );
		return NULL;
	}

	/*-----------------------*/
	/* Determine issue price */
	/*-----------------------*/
	*status = gb_rules_determine_issue (p_bond->series, p_bond->denom, &p_bond->issue);
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine issue price") );
		gb_doc_bond_free( p_bond );
		return NULL;
	}

	/*-------------------------------*/
	/* Determine final maturity date */
	/*-------------------------------*/
	*status = gb_rules_determine_maturity (p_bond->series, p_bond->idate,
					       &p_bond->mdate);
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine maturity data") );
		gb_doc_bond_free( p_bond );
		return NULL;
	}

	*status = GB_OK;

	gb_debug (DEBUG_DOC, "END");

	return p_bond;
}

/****************************************************************************/
/* Free memory of a previously allocate bond.                               */
/****************************************************************************/
void
gb_doc_bond_free (gbDocBond     *p_bond)
{
	gb_debug (DEBUG_DOC, "START");

	g_free( p_bond->sn );
	p_bond->sn = NULL;
	g_free( p_bond );

	gb_debug (DEBUG_DOC, "END");
}

/****************************************************************************/
/* Return bond information structure for given bond with given redemption   */
/* date.                                                                    */
/****************************************************************************/
gbDocBondInfo *
gb_doc_bond_get_info (gbDocBond     *p_bond,
		      const gchar   *rdate,
		      gbStatus      *status)
{
	gbTableModel  *table_model;
	gbDocBondInfo *p_info;
	gbDate         sb_rdate, sb_ladate, sb_nadate;

	gb_debug (DEBUG_DOC, "START");

	table_model = gb_table_get_model ();

	/*-------------------------------------*/
	/* Allocate info structure             */
	/*-------------------------------------*/
	p_info = g_new0( gbDocBondInfo, 1 );
	if ( p_info == NULL ) {
		g_warning( _("Cannot allocate bond information structure!") );
		*status = GB_ERROR_MEM_ALLOC;
		return NULL;
	}

	/*-------------------------------------*/
	/* Convert date string to SB_Date.     */
	/*-------------------------------------*/
	*status = gb_date_parse( rdate, &sb_rdate );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot parse redemption date") );
		gb_doc_bond_free_info( p_info );
		return NULL;
	}

	/*-------------------------------------*/
	/* Extract direct information first.   */
	/*-------------------------------------*/
	p_info->sn = g_strdup( p_bond->sn );
	p_info->series = g_strdup( gb_series_fmt( p_bond->series ) );
	p_info->denom = p_bond->denom;
	p_info->denom_str = gb_value_fmt( p_info->denom, FALSE );
	p_info->issue_date = gb_date_fmt( p_bond->idate );
	p_info->issue_price = p_bond->issue;
	p_info->issue_price_str = gb_value_fmt( p_info->issue_price, FALSE );
	p_info->final_maturity = gb_date_fmt( p_bond->mdate );
  
	/*--------------------------------------------*/
	/* Now, derive info based on redemption date. */
	/*--------------------------------------------*/
	p_info->value = gb_table_model_get_value( table_model,
						  p_bond->series,
						  p_bond->denom,
						  p_bond->issue,
						  p_bond->idate,
						  sb_rdate,
						  &p_info->nopay_flag );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine value") );
		gb_doc_bond_free_info( p_info );
		return NULL;
	}
	p_info->value_str = gb_value_fmt( p_info->value, TRUE );
	p_info->interest = p_info->value - p_info->issue_price;
	*status = gb_rules_get_last_accrual( p_bond->series,
					     p_bond->idate, sb_rdate, p_bond->mdate,
					     &sb_ladate );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine last accrual date") );
		gb_doc_bond_free_info( p_info );
		return NULL;
	}
	*status = gb_rules_get_next_accrual( p_bond->series,
					     p_bond->idate, sb_rdate, p_bond->mdate,
					     &sb_nadate );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine next accrual date") );
		gb_doc_bond_free_info( p_info );
		return NULL;
	}
	*status = gb_determine_yield( p_info->issue_price, p_info->value,
				      p_bond->idate, sb_ladate,
				      &p_info->yield );
	if ( *status != GB_OK ) {
		g_warning( _("Cannot determine yield") );
		gb_doc_bond_free_info( p_info );
		return NULL;
	}
	if ( p_bond->idate <= sb_rdate ) {
		p_info->interest_str = gb_value_fmt( p_info->interest, TRUE );
		p_info->last_accrual = gb_date_fmt( sb_ladate );
		p_info->next_accrual = gb_date_fmt( sb_nadate );
		p_info->yield_str    = g_strdup_printf( "%.2f%%", p_info->yield );
	}
	else {
		p_info->interest_str = g_strdup( "  -  " );
		p_info->last_accrual = g_strdup( "   -   " );
		p_info->next_accrual = g_strdup( "   -   " );
		p_info->yield_str    = g_strdup( "  -  " );
	}

	if ( sb_rdate >= p_bond->mdate ) {
		p_info->matured_flag = TRUE;
	}
	else {
		p_info->matured_flag = FALSE;
	}

	*status = gb_rules_determine_exchangeability( p_bond->series, p_bond->idate,
						      p_bond->mdate, sb_rdate,
						      &p_info->exchangeable_flag );
	if ( *status != GB_OK ) {
		gb_doc_bond_free_info( p_info );
		return NULL;
	}

	p_info->flag_str = g_strdup_printf( "%s%s",
					    (p_info->matured_flag ? "M" : ""),
					    (p_info->nopay_flag   ? "*" : "") );

	*status = GB_OK;

	gb_debug (DEBUG_DOC, "END");

	return p_info;
}

/****************************************************************************/
/* Free memory of a previously allocate bond information structure.         */
/****************************************************************************/
void
gb_doc_bond_free_info (gbDocBondInfo *p_info)
{
	gb_debug (DEBUG_DOC, "START");

	g_free( p_info->sn );
	p_info->sn = NULL;
	g_free( p_info->series );
	p_info->series = NULL;
	g_free( p_info->denom_str );
	p_info->denom_str = NULL;
	g_free( p_info->issue_date );
	p_info->issue_date = NULL;
	g_free( p_info->issue_price_str );
	p_info->issue_price_str = NULL;
	g_free( p_info->value_str );
	p_info->value_str = NULL;
	g_free( p_info->interest_str );
	p_info->interest_str = NULL;
	g_free( p_info->yield_str );
	p_info->yield_str = NULL;
	g_free( p_info->last_accrual );
	p_info->last_accrual = NULL;
	g_free( p_info->next_accrual );
	p_info->next_accrual = NULL;
	g_free( p_info->final_maturity );
	p_info->final_maturity = NULL;
	g_free( p_info->flag_str );
	p_info->flag_str = NULL;

	g_free( p_info );

	gb_debug (DEBUG_DOC, "END");
}


