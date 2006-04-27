/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  doc-sbw.c:  gbonds sbw import module
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
#include <string.h>

#include "doc-sbw.h"
#include "sbw4.h"
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static gbDoc *read_sbw2 (FILE *fp, gbStatus *status);
static gbDoc *read_sbw4 (FILE *fp, gbStatus *status);

/****************************************************************************/
/* Open and read doc from sbw file.                                         */
/****************************************************************************/
gbDoc *
gb_doc_sbw_open (const gchar    *filename,
		 gbDocSBWStatus *status)
{
	FILE      *fp;
	gchar      line[256];
	gint       version = 0;
	gbDoc     *doc = NULL;

	gb_debug (DEBUG_XML, "START");

	fp = fopen( filename, "r" );
	if ( fp == NULL ) {
		g_warning( _("Cannot open \"%s\""), filename );
		*status = GB_ERROR_OPEN_SBW_CANNOT_OPEN_FILE;
		return NULL;
	}

	/* Look for magic numbers to determine version. */
	/* 1st try the SBW 2 or 3 format */
	fgets( line, 256, fp );
	rewind( fp );
	if ((strncmp("\"SBW 2\"", line, 7) == 0) || (strncmp("\"SBW 3\"", line, 7) == 0)) {
		version = 2;
	} else {
		/* Failing that try for version 4 */
		fseek( fp, sizeof(SBW4_Head), SEEK_SET );
		fread( line, SBW4_CBOND_SIZE, 1, fp);
		rewind( fp );
		if ( strncmp( "CBond", line, sizeof(SBW4_CBOND_SIZE)) == 0 ) {
			version = 4;
		}
	}

	/* Now import using the proper format. */
	switch (version) {
	case 2:
		g_message( "Importing SBW 2 or 3 file" );
		doc = read_sbw2 (fp, status);
		break;
	case 4:
		g_message( "Importing SBW 4 file" );
		doc = read_sbw4 (fp, status);
		break;
	default:
		g_warning( _("Problem parsing SBW file, bad magic number.") );
		*status = GB_ERROR_OPEN_SBW_BAD_MAGIC;
	}

	fclose( fp );

	gb_debug (DEBUG_XML, "END");

	return doc;
}

/*-------------------------------------------------------------------------*/
/* PRIVATE.  Read an SBW 2 or 3 file.                                      */
/*-------------------------------------------------------------------------*/
static gbDoc *
read_sbw2 (FILE *fp, gbStatus *status)
{
	gbDoc     *doc;
	gchar     *p_title;
	gchar    **field;
	gchar     *p_series, *p_idate, *p_denom, *p_sn;
	gdouble    xdenom;
	gint       i, n;
	gchar      line[256];
	gbDocBond *p_bond;

	doc = GB_DOC(gb_doc_new());
	*status = GB_OK;

	fgets( line, 256, fp );  /* "SBW 2" or "SBW 3" */
	if ( (strncmp( "\"SBW 2\"", line, 7 ) != 0) &&
	     (strncmp( "\"SBW 3\"", line, 7 ) != 0) ) {
		g_warning( _("Problem parsing SBW file, bad magic number.") );
		*status = GB_ERROR_OPEN_SBW_BAD_MAGIC;
		g_object_unref (G_OBJECT(doc));
		return NULL;
	}

	fgets( line, 256, fp );  /* "title" */
	p_title = strtok( line, "\"" ); /* Strip quotes */
	gb_doc_set_title (doc, p_title);

	fgets( line, 256, fp );  /* "redemption date" -- IGNORE */

	fgets( line, 256, fp );  /* N */
	if ( sscanf( line, "%d", &n ) != 1 /* Line should contain only 1 field */ ) {
		g_warning( _("Problem parsing SBW file, line 4 should have N.") );
		*status = GB_ERROR_OPEN_SBW_PARSE;
		g_object_unref (G_OBJECT(doc));
		return NULL;
	}

	for ( i=0; i<n; i++ ) {

		/* Read bond information.  1 bond per line.  We only need the first 4  */
		/* fields -- the remaining fields are dependent on the redemption date */
		/* and just get recalculated.                                          */
		fgets( line, 256, fp );
		field = g_strsplit( line, ",", 4 );
		p_sn     = strtok( field[0], "\"" ); /* Strip quotes */
		p_denom  = strtok( field[1], "\"" ); /* Strip quotes */
		p_series = strtok( field[2], "\"" ); /* Strip quotes */
		p_idate  = strtok( field[3], "\"" ); /* Strip quotes */

		sscanf( p_denom, "%lf", &xdenom );

		p_bond = gb_doc_bond_new( p_series, p_idate, xdenom, p_sn, status );
		g_strfreev( field );
		if ( *status != GB_OK ) {
			g_warning( _("Cannot create bond, status = %d"), *status );
			g_object_unref (G_OBJECT(doc));
			return NULL;
		}

		*status = gb_doc_add_bond( doc, p_bond );
		if ( *status != GB_OK ) {
			gb_doc_bond_free( p_bond );
			g_warning( _("Cannot add bond to list, status = %d"), *status );
			g_object_unref (G_OBJECT(doc));
			return NULL;
		}
	}

	return doc;
}

/*-------------------------------------------------------------------------*/
/* PRIVATE.  Read an SBW 4 file.                                           */
/*-------------------------------------------------------------------------*/
static gbDoc *
read_sbw4 (FILE *fp, gbStatus *status)
{
	gbDoc              *doc;
	gint                i, n;
	SBW4_Head           head;
	gchar               cbond[SBW4_CBOND_SIZE];
	SBW4_BondInfoFixed  info_fixed;
	guchar              n_bytes;
	gchar               notes[256];
	gchar               sn[256];
	gchar               series[256];
	gshort              inter_record;
	gbDocBond          *p_bond;
	gchar              *idate_string;

	doc = GB_DOC(gb_doc_new());
	*status = GB_OK;

	gb_doc_set_title (doc, _("Imported SBW4 Inventory"));
  
	fread (&head, sizeof(SBW4_Head), 1, fp);
	fread (cbond, SBW4_CBOND_SIZE, 1, fp);
	n = head.n_bonds;

	for (i=0; i<n; i++) {

		fread (&info_fixed, sizeof(SBW4_BondInfoFixed), 1, fp);

		fread (&n_bytes, sizeof(guchar), 1, fp);
		if ( n_bytes > 0 ) {
			fread (notes, n_bytes, 1, fp);
			notes[n_bytes] = 0;
		}

		fread (&n_bytes, sizeof(guchar), 1, fp);
		if ( n_bytes > 0 ) {
			fread (sn, n_bytes, 1, fp);
			sn[n_bytes] = 0;
		}

		fread (&n_bytes, sizeof(guchar), 1, fp);
		if ( n_bytes > 0 ) {
			fread (series, n_bytes, 1, fp);
			series[n_bytes] = 0;
		}

		if (i < (n-1)) {
			fread (&inter_record, sizeof(gshort), 1, fp);
		}

		if (     (strcasecmp("E", series) == 0)
		      || (strcasecmp("S", series) == 0)
		      || (strcasecmp("EE", series) == 0)
		      || (strcasecmp("I", series) == 0) ) {

			idate_string = gb_date_fmt( info_fixed.idate + SBW4_EPOCH );
			p_bond = gb_doc_bond_new( series,
						  idate_string,
						  info_fixed.denom,
						  sn,
						  status);
			g_free( idate_string );
			if ( *status != GB_OK ) {
				g_warning( _("Cannot create bond, status = %d"), *status );
				g_object_unref (G_OBJECT(doc));
				return NULL;
			}


			*status = gb_doc_add_bond( doc, p_bond );
			if ( *status != GB_OK ) {
				gb_doc_bond_free( p_bond );
				g_warning( _("Cannot add bond to list, status = %d"),
					   *status );
				g_object_unref (G_OBJECT(doc));
				return NULL;
			}
		} else {
			g_message( _("Skipping unsupported bond series (%s)"), series );
		}
	}

	return doc;
}


