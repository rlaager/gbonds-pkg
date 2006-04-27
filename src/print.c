/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  print.c:  Savings Bond Print module
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


#include <config.h>

#include <time.h>
#include <ctype.h>
#include <libgnomeprint/gnome-print-job.h>

#include "print.h"

#define GBONDS_PRINT_HEADER_FONT "sans"
#define GBONDS_PRINT_HEADER_FONT_SIZE 11
#define GBONDS_PRINT_BODY_FONT "courier"
#define GBONDS_PRINT_BODY_FONT_SIZE 10

#define GBONDS_PRINT_BONDS_PER_PAGE 50

/*===========================================*/
/* Private types.                            */
/*===========================================*/
typedef struct _PrintInfo {
  /* gnome print context */
  GnomePrintContext *pc;

  /* gnome print config */
  GnomePrintConfig *config;

  /* paper characteristics */
  gdouble page_width;
  gdouble page_height;
  gdouble margin_top;
  gdouble margin_bottom;
  gdouble margin_left;
  gdouble margin_right;

  /* position of main printing area margins */
  gdouble x_lmargin, x_rmargin;
  gdouble y_tmargin, y_bmargin;

  /* Current font info */
  GnomeFont *font;
  gchar     *font_name;
  gint      font_size;
  gdouble   font_spacing;
  gboolean  font_italic;

  /* Current position */
  gdouble x, y;

} PrintInfo;

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static void print_page_header( PrintInfo *pi,
			       gchar *title, gchar *name,
			       gint i_page, gint n_pages,
			       const gchar *date_string );
static void print_empty_inventory( PrintInfo *pi );
static void print_redemption_date( PrintInfo *pi, gchar *rdate );
static void print_table_headers( PrintInfo *pi );
static void print_bond_info( PrintInfo *pi, gbDocBondInfo *info );
static void print_legend( PrintInfo *pi );
static void print_summary( PrintInfo *pi,
			   gchar *rdate, gint n_bonds,
			   gdouble inventory_value,
			   gdouble redemption_value,
			   gdouble total_interest );

static PrintInfo *print_info_new( GnomePrintJob *job );
static void print_info_free( PrintInfo *pi );

static void print_font( PrintInfo *pi, gchar *name, gint size );
static void print_bold( PrintInfo *pi );
static void print_bold_end( PrintInfo *pi );
static void print_show( PrintInfo *pi, gchar *text );
static void print_show_right( PrintInfo *pi, gchar *text );
static void print_show_center( PrintInfo *pi, gchar *text );
static void print_goto_line( PrintInfo *pi, gint n );
static void print_goto_next_line( PrintInfo *pi );
static void print_goto_header( PrintInfo *pi );
static void print_goto_footer( PrintInfo *pi );
static void print_goto_eol( PrintInfo *pi );
static void print_goto_center( PrintInfo *pi );
static void print_goto_column( PrintInfo *pi, gint n );
static void print_hbar( PrintInfo *pi, gint n );


/******************************************************************************/
/* Print the inventory.                                                       */
/******************************************************************************/
void
gb_print (GnomePrintJob    *job,
	  gbView           *view)
{
  PrintInfo     *pi;
  gint           i_page, n_pages;
  gint           i_bond, n_bonds;
  gchar         *page_str;
  gchar         *basename, *title;
  GList         *p;
  gbDocBond     *p_bond;
  gbDocBondInfo *info;
  gbStatus       status;
  gdouble        inventory_value=0.0, redemption_value=0.0, total_interest=0.0;
  time_t         t;
  struct tm     *tm;
  gchar         *date_string;

  pi = print_info_new (job);

  basename = gb_doc_get_short_name (view->doc);
  title    = gb_doc_get_title (view->doc);

  /* Format date string */
  t = time(NULL); tm = localtime(&t);
  date_string = g_strdup_printf( "%02d/%02d/%4d %2d:%02d %s",
				 tm->tm_mon+1, tm->tm_mday, tm->tm_year+1900,
				 (tm->tm_hour%12) ? tm->tm_hour%12 : 12,
				 tm->tm_min,
				 (tm->tm_hour/12) ? "PM" : "AM" );

  /* Count bonds in inventory */
  n_bonds = 0;
  for ( p=view->doc->list; p!=NULL; p=p->next ) {
    n_bonds++;
  }

  /* Calculate total number of pages */
  if ( n_bonds == 0 ) {
    n_pages = 1;
  }
  else {
    n_pages = ( (n_bonds-1) / GBONDS_PRINT_BONDS_PER_PAGE ) + 2;
  }

  i_page = 1;
  i_bond = 0;

  /* Begin new page */
  page_str = g_strdup_printf( "%d", i_page );
  gnome_print_beginpage( pi->pc, page_str );
  g_free( page_str );
  print_page_header( pi, title, basename, i_page, n_pages, date_string );

  if ( view->doc->list == NULL ) {
    print_empty_inventory( pi );
    gnome_print_showpage( pi->pc );
  }
  else {
    /*---------------------------------------------*/
    /* Traverse bond list and print information.   */
    /*---------------------------------------------*/
    print_goto_line( pi, 3 );
    print_redemption_date( pi, view->rdate );
    for ( p=view->doc->list; p!=NULL; p=p->next ) {
      p_bond = (gbDocBond *)p->data;

      if ( (i_bond % GBONDS_PRINT_BONDS_PER_PAGE) == 0 ) {

	if ( i_bond != 0 ) {
	  /* Finish old page */
	  gnome_print_showpage( pi->pc );

	  /* Begin new page */
	  i_page++;
	  page_str = g_strdup_printf( "%d", i_page );
	  gnome_print_beginpage( pi->pc, page_str );
	  g_free( page_str );
	  print_page_header( pi,
			     title, basename, i_page, n_pages, date_string );
	  print_goto_line( pi, 3 );
	  print_redemption_date( pi, view->rdate );
	}

	print_table_headers( pi );
      }

      info = gb_doc_bond_get_info( p_bond, view->rdate, &status );
      if ( info == NULL ) {
	return;
      }
      print_bond_info( pi, info );

      i_bond++;
      inventory_value += info->value;
      total_interest  += info->interest;
      if ( !info->nopay_flag ) redemption_value += info->value;

      gb_doc_bond_free_info( info );
      info = NULL;
    }
    gnome_print_showpage( pi->pc );

    /*---------------------------------------------*/
    /* Finish up with a summary page.              */
    /*---------------------------------------------*/
    i_page++;
    page_str = g_strdup_printf( "%d", i_page );
    gnome_print_beginpage( pi->pc, page_str );
    g_free( page_str );
    print_page_header( pi, title, basename, i_page, n_pages, date_string );
    print_summary( pi,
		   view->rdate, n_bonds,
		   inventory_value, redemption_value, total_interest );
    print_legend( pi );
    gnome_print_showpage( pi->pc );
  }

  print_info_free( pi );
  g_free( basename );
  g_free( title );
  g_free( date_string );
}




/*==================================================*/
/* PRIVATE.  Print page header and footer           */
/*==================================================*/
static void
print_page_header( PrintInfo   *pi,
		   gchar       *title,
		   gchar       *name,
		   gint         i_page,
		   gint         n_pages,
		   const gchar *date_string )
{
  gchar *string;

  if ( title == NULL ) {
    title = "";
  }
  if ( name == NULL ) {
    name = _("Unsaved Inventory");
  }

  print_goto_header( pi );

  print_font( pi, GBONDS_PRINT_HEADER_FONT, GBONDS_PRINT_HEADER_FONT_SIZE );
  print_goto_center( pi );
  print_show_center( pi, title );
  print_goto_next_line( pi );

  print_font( pi, GBONDS_PRINT_HEADER_FONT, GBONDS_PRINT_HEADER_FONT_SIZE-2 );
  print_goto_center( pi );
  print_show_center( pi, _("Savings Bond Inventory Report") );

  print_goto_footer( pi );
  print_font( pi, GBONDS_PRINT_HEADER_FONT, GBONDS_PRINT_HEADER_FONT_SIZE-2 );

  string = g_strdup_printf( "%s  %s", date_string, name );
  print_show( pi, string );
  g_free( string );

  print_goto_eol( pi );
  string = g_strdup_printf( _("%d of %d"), i_page, n_pages );
  print_show_right( pi, string );
  g_free( string );
}




/*==================================================*/
/* PRIVATE.  Print "empty inventory" message.       */
/*==================================================*/
static void
print_empty_inventory (PrintInfo *pi)
{
  print_font( pi, GBONDS_PRINT_BODY_FONT, GBONDS_PRINT_BODY_FONT_SIZE );

  print_goto_line( pi, 2 );
  print_show( pi, _("(Empty Inventory)") );
}




/*==================================================*/
/* PRIVATE.  Print redemption date                  */
/*==================================================*/
static void
print_redemption_date (PrintInfo *pi,
		       gchar     *rdate)
{
  print_font( pi, GBONDS_PRINT_BODY_FONT, GBONDS_PRINT_BODY_FONT_SIZE );


  print_bold( pi );
  print_show( pi, _("Redemption Date: ") );
  print_bold_end( pi );

  print_show( pi, rdate );

  print_goto_next_line( pi );
}




/*==================================================*/
/* PRIVATE.  Print table headers                    */
/*==================================================*/
static void
print_table_headers (PrintInfo *pi)
{
  gint n = 5, m=1;

  gnome_print_setlinewidth( pi->pc, 1.0 );

  print_font( pi, GBONDS_PRINT_BODY_FONT, GBONDS_PRINT_BODY_FONT_SIZE );
  print_bold( pi );

  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _("Serial Number") );
  print_goto_column( pi, m );
  print_hbar( pi, 13 );
  m += 14;

  print_goto_line( pi, n );
  print_goto_column( pi, m );
  print_show( pi, _(" Denom. &") );
  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _("  Series") );
  print_goto_column( pi, m );
  print_hbar( pi, 10 );
  m += 11;
  
  print_goto_line( pi, n );
  print_goto_column( pi, m );
  print_show( pi, _(" Issue") );
  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _(" Date") );
  print_goto_column( pi, m );
  print_hbar( pi, 7 );
  m += 8;

  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _("  Value") );
  print_goto_column( pi, m );
  print_hbar( pi, 10 );
  m += 11;
  
  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _(" Interest") );
  print_goto_column( pi, m );
  print_hbar( pi, 10 );
  m += 11;
  
  print_goto_line( pi, n );
  print_goto_column( pi, m );
  print_show( pi, _(" Yield") );
  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _("To Date") );
  print_goto_column( pi, m );
  print_hbar( pi, 7 );
  m += 8;
  
  print_goto_line( pi, n );
  print_goto_column( pi, m );
  print_show( pi, _(" Next") );
  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _("Accrual") );
  print_goto_column( pi, m );
  print_hbar( pi, 7 );
  m += 8;
  
  print_goto_line( pi, n );
  print_goto_column( pi, m );
  print_show( pi, _(" Final") );
  print_goto_line( pi, n+1 );
  print_goto_column( pi, m );
  print_show( pi, _("Maturity") );
  print_goto_column( pi, m );
  print_hbar( pi, 8 );

  print_bold_end( pi );
  print_goto_line( pi, n+2 );
}




/*==================================================*/
/* PRIVATE.  Print bond information (table line)    */
/*==================================================*/
static void
print_bond_info (PrintInfo     *pi,
		 gbDocBondInfo *info)
{
  gchar *line;

  print_font( pi, GBONDS_PRINT_BODY_FONT, GBONDS_PRINT_BODY_FONT_SIZE );

  print_goto_next_line( pi );
  line = g_strdup_printf(  "%-13s %7s %-2s %7s %10s %10s %6s  %7s %7s %s",
			   info->sn, info->denom_str, info->series,
			   info->issue_date, info->value_str,
			   info->interest_str, info->yield_str,
			   info->next_accrual, info->final_maturity,
			   info->flag_str );
  print_show( pi, line );
  g_free( line );
}




/*==================================================*/
/* PRIVATE.  Print legend at bottom of each page.   */
/*==================================================*/
static void
print_legend (PrintInfo *pi)
{
  print_font( pi, GBONDS_PRINT_BODY_FONT, GBONDS_PRINT_BODY_FONT_SIZE );

  print_goto_next_line( pi );
  print_goto_next_line( pi );

  print_goto_next_line( pi );
  print_show( pi, _("Notes:") );

  print_goto_next_line( pi );
  print_show( pi, _("* = Bond is not yet eligible for redemption.") );

  print_goto_next_line( pi );
  print_show( pi, _("M = Bond has reached final maturity.") );
}




/*==================================================*/
/* PRIVATE.  Print inventory summary information.   */
/*==================================================*/
static void
print_summary (PrintInfo *pi,
	       gchar *rdate, gint n_bonds,
	       gdouble inventory_value,
	       gdouble redemption_value,
	       gdouble total_interest)
{
  gchar *string;
  gchar *buf;

  print_font( pi, GBONDS_PRINT_BODY_FONT, GBONDS_PRINT_BODY_FONT_SIZE );

  print_goto_line( pi, 3 );
  print_bold( pi );
  print_show( pi, _("Summary") );
  print_bold_end( pi );

  print_goto_next_line( pi );

  print_goto_next_line( pi );
  print_bold( pi );
  print_show( pi, _("Redemption Date:  ") );
  print_bold_end( pi );
  string = g_strdup_printf( "%12s", rdate );
  print_show( pi, string );
  g_free( string );

  print_goto_next_line( pi );
  print_bold( pi );
  print_show( pi, _("Number of Bonds:  ") );
  print_bold_end( pi );
  string = g_strdup_printf( "%12d", n_bonds );
  print_show( pi, string );
  g_free( string );

  print_goto_next_line( pi );
  print_bold( pi );
  print_show( pi, _("Inventory Value:  ") );
  print_bold_end( pi );
  buf = gb_value_fmt( inventory_value, TRUE );
  string = g_strdup_printf( "%12s", buf );
  g_free( buf );
  print_show( pi, string );
  g_free( string );

  print_goto_next_line( pi );
  print_bold( pi );
  print_show( pi, _("Redemption Value: ") );
  print_bold_end( pi );
  buf = gb_value_fmt( redemption_value, TRUE );
  string = g_strdup_printf( "%12s", buf );
  g_free( buf );
  print_show( pi, string );
  g_free( string );

  print_goto_next_line( pi );
  print_bold( pi );
  print_show( pi, _("Total Interest:   ") );
  print_bold_end( pi );
  buf = gb_value_fmt( total_interest, TRUE );
  string = g_strdup_printf( "%12s", buf );
  g_free( buf );
  print_show( pi, string );
  g_free( string );
}




/*==================================================*/
/* PRIVATE.  new print info structure               */
/*==================================================*/
static PrintInfo *
print_info_new (GnomePrintJob *job)
{
  PrintInfo *pi = g_new0( PrintInfo, 1 );

  pi->pc = gnome_print_job_get_context (job);

  pi->config = gnome_print_job_get_config (job);

  gnome_print_config_get_length (pi->config,
				 GNOME_PRINT_KEY_PAPER_WIDTH,
				 &pi->page_width,
				 NULL);
  gnome_print_config_get_length (pi->config,
				 GNOME_PRINT_KEY_PAPER_HEIGHT,
				 &pi->page_height,
				 NULL);

  gnome_print_config_get_length (pi->config,
				 GNOME_PRINT_KEY_PAGE_MARGIN_TOP,
				 &pi->margin_top,
				 NULL);
  gnome_print_config_get_length (pi->config,
				 GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM,
				 &pi->margin_bottom,
				 NULL);
  gnome_print_config_get_length (pi->config,
				 GNOME_PRINT_KEY_PAGE_MARGIN_LEFT,
				 &pi->margin_left,
				 NULL);
  gnome_print_config_get_length (pi->config,
				 GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT,
				 &pi->margin_right,
				       NULL);

  pi->x_lmargin = pi->margin_left;
  pi->x_rmargin = pi->page_width - pi->margin_right;
  pi->y_tmargin = pi->page_height - pi->margin_top;
  pi->y_bmargin = pi->margin_bottom;
  
  pi->font_name = NULL;

  return pi;
}




/*==================================================*/
/* PRIVATE.  free print info structure              */
/*==================================================*/
static void print_info_free( PrintInfo *pi )
{
  gnome_print_context_close( pi->pc );

  g_free( pi->font_name );
  pi->font_name = NULL;

  g_free( pi );
}




/*==================================================*/
/* PRIVATE.  Set new font.                          */
/*==================================================*/
static void print_font( PrintInfo *pi, gchar *name, gint size )
{
  pi->font = gnome_font_find_closest_from_weight_slant (name,
							GNOME_FONT_BOOK,
							FALSE,
							size);
  gnome_print_setfont( pi->pc, pi->font );
  
  g_free( pi->font_name );
  pi->font_name = g_strdup( name );
  pi->font_size    = size;
  pi->font_spacing = size;
  pi->font_italic  = FALSE;
}




/*==================================================*/
/* PRIVATE.  Embolden current font.                 */
/*==================================================*/
static void print_bold( PrintInfo *pi )
{
  pi->font = gnome_font_find_closest_from_weight_slant (pi->font_name,
							GNOME_FONT_BOLD,
							pi->font_italic,
							pi->font_size);
  gnome_print_setfont( pi->pc, pi->font );
}




/*==================================================*/
/* PRIVATE.  End Embolden.                          */
/*==================================================*/
static void print_bold_end( PrintInfo *pi )
{
  pi->font = gnome_font_find_closest_from_weight_slant (pi->font_name,
							GNOME_FONT_BOOK,
							pi->font_italic,
							pi->font_size);
  gnome_print_setfont( pi->pc, pi->font );
}




/*===================================================*/
/* PRIVATE.  Show text starting at current position. */
/*===================================================*/
static void print_show( PrintInfo *pi, gchar *text )
{
  gnome_print_show( pi->pc, text );
  pi->x += gnome_font_get_width_utf8( pi->font, text );
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*===================================================*/
/* PRIVATE.  Show text anchored on the right.        */
/*===================================================*/
static void print_show_right( PrintInfo *pi, gchar *text )
{
  pi->x -= gnome_font_get_width_utf8( pi->font, text );
  gnome_print_moveto( pi->pc, pi->x, pi->y );
  gnome_print_show( pi->pc, text );
}



/*=====================================================*/
/* PRIVATE.  Show text centered about current position */
/*=====================================================*/
static void print_show_center( PrintInfo *pi, gchar *text )
{
  pi->x -= gnome_font_get_width_utf8( pi->font, text ) / 2.0;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
  gnome_print_show( pi->pc, text );
}




/*=============================================================*/
/* PRIVATE.  Goto line n.  Assume current font's line spacing. */
/*=============================================================*/
static void print_goto_line( PrintInfo *pi, gint n )
{
  pi->x = pi->x_lmargin;
  pi->y = pi->y_tmargin - n*pi->font_spacing;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*=============================================================*/
/* PRIVATE.  Goto next line.                                   */
/*=============================================================*/
static void print_goto_next_line( PrintInfo *pi )
{
  pi->x = pi->x_lmargin;
  pi->y -= pi->font_spacing;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*===================================================*/
/* PRIVATE.  Goto header, just above top margin.     */
/*===================================================*/
static void print_goto_header( PrintInfo *pi )
{
  pi->x = pi->x_lmargin;
  pi->y = pi->y_tmargin + 24;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}


/*===================================================*/
/* PRIVATE.  Goto footer, just below bottom margin.  */
/*===================================================*/
static void print_goto_footer( PrintInfo *pi )
{
  pi->x = pi->x_lmargin;
  pi->y = pi->y_bmargin - 6;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*===================================================*/
/* PRIVATE.  Goto right margin.                      */
/*===================================================*/
static void print_goto_eol( PrintInfo *pi )
{
  pi->x = pi->x_rmargin;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*===================================================*/
/* PRIVATE.  Goto center of line.                    */
/*===================================================*/
static void print_goto_center( PrintInfo *pi )
{
  pi->x = (pi->x_lmargin + pi->x_rmargin) / 2.0;
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*===============================================================*/
/* PRIVATE.  Goto column n.  Assumes current font is fixedwidth. */
/*===============================================================*/
static void print_goto_column( PrintInfo *pi, gint n )
{
  pi->x = pi->x_lmargin + (n-1)*gnome_font_get_width_utf8( pi->font, "#" );
  gnome_print_moveto( pi->pc, pi->x, pi->y );
}




/*===============================================================*/
/* PRIVATE.  Draw n column horizontal line.                      */
/*===============================================================*/
static void print_hbar( PrintInfo *pi, gint n )
{
  gnome_print_moveto( pi->pc, pi->x, pi->y - 0.3*pi->font_spacing );

  pi->x += (n)*gnome_font_get_width_utf8( pi->font, " " );
  gnome_print_lineto( pi->pc, pi->x, pi->y - 0.3*pi->font_spacing );
  gnome_print_stroke( pi->pc );
}

