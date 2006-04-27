/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  doc.h:  gbonds document module header file
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
#ifndef __DOC_H__
#define __DOC_H__

#include <glib-object.h>
#include "types.h"

G_BEGIN_DECLS

/*=======================================================================*/
/* Type Definitions                                                      */
/*=======================================================================*/

/*------------------------*/
/* Basic bond structure   */
/*------------------------*/
typedef struct {

  gchar     *sn;     /* Serial Number */
  gbSeries   series; /* Series (see gbSeries) */
  gdouble    denom;  /* Denomination */
  gbDate     idate;  /* Issue Date */
  gdouble    issue;  /* Issue Price */
  gbDate     mdate;  /* Maturity Date */

} gbDocBond;

/*------------------------------*/
/* Bond information structure   */
/*------------------------------*/
typedef struct {
  gchar        *sn;
  gchar        *series;
  gdouble      denom;
  gchar        *denom_str;
  gchar        *issue_date;
  gdouble      issue_price;
  gchar        *issue_price_str;
  gdouble      value;
  gchar        *value_str;
  gdouble      interest;
  gchar        *interest_str;
  gdouble      yield;
  gchar        *yield_str;
  gchar        *last_accrual;
  gchar        *next_accrual;
  gchar        *final_maturity;
  gboolean     nopay_flag;
  gboolean     matured_flag;
  gboolean     exchangeable_flag;
  gchar        *flag_str;
} gbDocBondInfo;


/*------------------------*/
/* Inventory document     */
/*------------------------*/
#define GB_TYPE_DOC            (gb_doc_get_type ())
#define GB_DOC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOC, gbDoc))
#define GB_DOC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GB_TYPE_DOC, gbDocClass))
#define GB_IS_DOC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_DOC))
#define GB_IS_DOC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GB_TYPE_DOC))

typedef struct _gbDoc          gbDoc;
typedef struct _gbDocClass     gbDocClass;

typedef struct _gbDocPrivate   gbDocPrivate;

struct _gbDoc {
	GObject       object;

	GList        *list;  /* List of gbBonds */

	gbDocPrivate *private;
};

struct _gbDocClass {
	GObjectClass         parent_class;

	void (*changed)          (gbDoc *doc, gpointer user_data);

	void (*name_changed)     (gbDoc *doc, gpointer user_data);

	void (*modified_changed) (gbDoc *doc, gpointer user_data);

};


/*=======================================================================*/
/* Function Prototypes                                                   */
/*=======================================================================*/

GType          gb_doc_get_type                (void);

GObject       *gb_doc_new                     (void);

gchar         *gb_doc_get_filename            (gbDoc         *doc);

gchar         *gb_doc_get_short_name          (gbDoc         *doc);

gboolean       gb_doc_is_modified             (gbDoc         *doc);

gboolean       gb_doc_is_untitled             (gbDoc         *doc);

gboolean       gb_doc_can_undo                (gbDoc         *doc);

gboolean       gb_doc_can_redo                (gbDoc         *doc);


void           gb_doc_set_filename            (gbDoc         *doc,
					       const gchar   *filename);

void           gb_doc_clear_modified          (gbDoc         *doc);


gbStatus       gb_doc_add_bond                (gbDoc         *doc,
					       gbDocBond     *p_bond);

gbStatus       gb_doc_delete_bond             (gbDoc         *doc,
					       gbDocBond     *p_bond);

gchar         *gb_doc_get_title               (gbDoc         *doc);

void           gb_doc_set_title               (gbDoc         *doc,
					       const gchar   *title);

/* Creating and freeing bonds */
gbDocBond     *gb_doc_bond_new                (gchar         *series,
					       const gchar   *idate,
					       gdouble        denom,
					       const gchar   *sn,
					       gbStatus      *status);

gbDocBond     *gb_doc_bond_new_from_sn_idate  (gchar         *sn,
					       const gchar   *idate,
					       gbStatus      *status);

void           gb_doc_bond_free               (gbDocBond     *p_bond);

/* Extracting current information from bond */
gbDocBondInfo *gb_doc_bond_get_info           (gbDocBond     *p_bond,
					       const gchar   *rdate,
					       gbStatus      *status);

void           gb_doc_bond_free_info          (gbDocBondInfo *info);

G_END_DECLS


#endif /* __DOC_H__ */
