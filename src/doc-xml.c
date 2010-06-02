/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  doc-xml.c:  gbonds xml inventory document module
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
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "doc-xml.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define NAME_SPACE "http://snaught.com/gbonds/2.0/"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static gbDoc       *xml_to_doc      (xmlDocPtr       doc,
				     gbDocXMLStatus *status);

static gbDoc       *xml_parse_doc   (xmlNodePtr      root,
				     gbDocXMLStatus *status);

static xmlDocPtr    doc_to_xml      (gbDoc          *doc,
				     gbDocXMLStatus *status);



/****************************************************************************/
/* Open and read doc from xml file.                                         */
/****************************************************************************/
gbDoc *
gb_doc_xml_open (const gchar    *filename,
		 gbDocXMLStatus *status)
{
	xmlDocPtr  xmldoc;
	gbDoc     *doc;

	gb_debug (DEBUG_XML, "START");

	xmldoc = xmlParseFile (filename);
	if (!xmldoc) {
		g_warning (_("xmlParseFile error"));
		*status = GB_DOC_XML_ERROR_OPEN_PARSE;
		return NULL;
	}

	doc = xml_to_doc (xmldoc, status);

	xmlFreeDoc (xmldoc);

	if (doc) {
		gb_doc_set_filename (doc, filename);
		gb_doc_clear_modified (doc);
	}

	gb_debug (DEBUG_XML, "END");

	return doc;
}

/****************************************************************************/
/* Read doc from xml buffer.                                                */
/****************************************************************************/
gbDoc *
gb_doc_xml_open_buffer (const gchar    *buffer,
			gbDocXMLStatus *status)
{
	xmlDocPtr  xmldoc;
	gbDoc     *doc;

	gb_debug (DEBUG_XML, "START");

	xmldoc = xmlParseDoc ((xmlChar *) buffer);
	if (!xmldoc) {
		g_warning (_("xmlParseFile error"));
		*status = GB_DOC_XML_ERROR_OPEN_PARSE;
		return NULL;
	}

	doc = xml_to_doc (xmldoc, status);

	if (doc) {
		gb_doc_clear_modified (doc);
	}

	xmlFreeDoc (xmldoc);

	gb_debug (DEBUG_XML, "END");

	return doc;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse xml doc structure and create doc.                        */
/*--------------------------------------------------------------------------*/
static gbDoc *
xml_to_doc (xmlDocPtr       xmldoc,
	    gbDocXMLStatus *status)
{
	xmlNodePtr  root;
	gbDoc      *doc;

	gb_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

	*status = GB_DOC_XML_OK;

	root = xmlDocGetRootElement (xmldoc);
	if (!root || !root->name) {
		g_warning (_("No document root"));
		*status = GB_DOC_XML_ERROR_OPEN_PARSE;
		return NULL;
	}

	doc = xml_parse_doc (root, status);

	gb_debug (DEBUG_XML, "END");

	return doc;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse xml root node and create doc.                            */
/*--------------------------------------------------------------------------*/
static gbDoc *
xml_parse_doc (xmlNodePtr      root,
	       gbDocXMLStatus *status)
{
	xmlNodePtr  node;
	gbDoc      *doc;
	xmlChar    *series, *idate, *denom, *sn, *title;
	gdouble     xdenom;
	gbDocBond  *p_bond;
	gbStatus    errno;

	gb_debug (DEBUG_XML, "START");

	*status = GB_OK;

	if ( (g_strcasecmp( (gchar *)root->name, "Bond-List" ) != 0) &&
	     (g_strcasecmp( (gchar *)root->name, "Bond-Inventory" ) != 0) ) {
		g_warning( _("Bad root node = \"%s\""), root->name );
		*status = GB_ERROR_OPEN_XML_PARSE;
		return NULL;
	}

	doc = GB_DOC(gb_doc_new ());

	for ( node=root->xmlChildrenNode; node!=NULL; node=node->next ) {

		if ( g_strcasecmp( (gchar *)node->name, "Title" ) == 0 ) {

			title = xmlNodeGetContent( node );
			gb_doc_set_title (doc, (gchar *)title);
			xmlFree (title);

		}
		else if ( g_strcasecmp( (gchar *)node->name, "Bond" ) == 0 ) {

			series = xmlGetProp( node, (xmlChar *)"series" );
			if ( !series || !series[0] ) {
				g_warning( _("Missing series property") );
				*status = GB_ERROR_OPEN_XML_PARSE;
				g_object_unref ( doc );
				return NULL;
			}

			idate  = xmlGetProp( node, (xmlChar *)"idate" );
			if ( !idate || !idate[0] ) {
				g_warning( _("Missing idate property") );
				*status = GB_ERROR_OPEN_XML_PARSE;
				g_object_unref ( doc );
				return NULL;
			}

			denom  = xmlGetProp( node, (xmlChar *)"denom" );
			if ( !denom || !denom[0] ) {
				g_warning( _("Missing denom property") );
				*status = GB_ERROR_OPEN_XML_PARSE;
				g_object_unref ( doc );
				return NULL;
			}
			sscanf( (gchar *)denom, "%lf", &xdenom );

			sn     = xmlGetProp( node, (xmlChar *)"sn" );

			p_bond = gb_doc_bond_new( (gchar *)series, (gchar *)idate, xdenom, (gchar *)sn, &errno );
			if ( errno != GB_OK ) {
				g_warning( _("Cannot create bond, status = %d"), errno );
				*status = errno;
				g_object_unref ( doc );
				return NULL;
			}
			xmlFree (series);
			xmlFree (idate);
			xmlFree (denom);
			xmlFree (sn);

			errno = gb_doc_add_bond( doc, p_bond );
			if ( errno != GB_OK ) {
				g_warning( _("Cannot add bond to list, status = %d"),
					   errno );
				gb_doc_bond_free( p_bond );
				*status = errno;
				g_object_unref ( doc );
				return NULL;
			}

		}
		else {
			if ( g_strcasecmp( (gchar *)node->name, "text" ) != 0 ) {
				g_warning( _("bad node =  \"%s\""), node->name );
			}
		}
	}

	gb_debug (DEBUG_XML, "END");

	return doc;
}

/****************************************************************************/
/* Save doc to xml doc file.                                                */
/****************************************************************************/
void
gb_doc_xml_save (gbDoc          *doc,
		 const gchar    *filename,
		 gbDocXMLStatus *status)
{
	xmlDocPtr xmldoc;
	gint      xml_ret;

	gb_debug (DEBUG_XML, "START");

	xmldoc = doc_to_xml (doc, status);

	xml_ret = xmlSaveFormatFile (filename, xmldoc, TRUE);
	xmlFreeDoc (xmldoc);
	if (xml_ret == -1) {

		g_warning (_("Problem saving xml file."));
		*status = GB_DOC_XML_ERROR_SAVE_FILE;

	} else {

		gb_doc_set_filename (doc, filename);
		gb_doc_clear_modified (doc);

	}

	gb_debug (DEBUG_XML, "END");
}

/****************************************************************************/
/* Save doc to xml buffer.                                                  */
/****************************************************************************/
gchar *
gb_doc_xml_save_buffer (gbDoc          *doc,
			gbDocXMLStatus *status)
{
	xmlDocPtr  xmldoc;
	gint       size;
	gchar     *buffer;

	gb_debug (DEBUG_XML, "START");

	xmldoc = doc_to_xml (doc, status);

	xmlDocDumpMemory (xmldoc, (xmlChar **)&buffer, &size);
	xmlFreeDoc (xmldoc);

	gb_doc_clear_modified (doc);

	gb_debug (DEBUG_XML, "END");

	return buffer;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Convert doc to xml doc structure.                              */
/*--------------------------------------------------------------------------*/
static xmlDocPtr
doc_to_xml (gbDoc          *doc,
	    gbDocXMLStatus *status)
{
	xmlDocPtr   xmldoc;
	xmlNsPtr    ns;
	xmlNodePtr  xml_bond;
	GList      *p;
	gbDocBond  *p_bond;
	gchar      *title, *idate, *denom;

	gb_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

	xmldoc = xmlNewDoc ((xmlChar *)"1.0");
	xmldoc->xmlRootNode = xmlNewDocNode (xmldoc, NULL, (xmlChar *)"Bond-Inventory", NULL);

	ns = xmlNewNs (xmldoc->xmlRootNode, (xmlChar *)NAME_SPACE, (xmlChar *)"gbonds");
	xmlSetNs (xmldoc->xmlRootNode, ns);

	title = gb_doc_get_title (doc);
	if ( title != NULL ) {
		xmlNewChild( xmldoc->xmlRootNode, NULL, (xmlChar *)"Title", (xmlChar *)title );
		g_free (title);
	}

	for ( p=doc->list; p!=NULL; p=p->next ) {
		p_bond = (gbDocBond *)p->data;

		idate = gb_date_fmt( p_bond->idate );
		denom = g_strdup_printf( "%.0f", p_bond->denom );

		xml_bond = xmlNewChild( xmldoc->xmlRootNode, NULL, (xmlChar *)"Bond", NULL );
		xmlSetProp( xml_bond, (xmlChar *)"series", (xmlChar *)gb_series_fmt( p_bond->series ) );
		xmlSetProp( xml_bond, (xmlChar *)"idate", (xmlChar *)idate );
		xmlSetProp( xml_bond, (xmlChar *)"denom", (xmlChar *)denom );
		if ( p_bond->sn != NULL ) {
			/* Imported SBW inventories may not have SNs */
			xmlSetProp( xml_bond, (xmlChar *)"sn", (xmlChar *)p_bond->sn );
		}

		g_free( idate );
		idate = NULL;
		g_free( denom );
		denom = NULL;
	}

	gb_debug (DEBUG_XML, "END");

	*status = GB_DOC_XML_OK;
	return xmldoc;
}

