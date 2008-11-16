/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  update.c:  Update redemption data druid module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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
#include <libgnomevfs/gnome-vfs.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <math.h>

#include "table.h"
#include "util.h"
#include "update.h"

#include "debug.h"

#define ICON_PIXMAP gnome_program_locate_file (NULL,\
                                               GNOME_FILE_DOMAIN_APP_PIXMAP,\
					       "gbonds.png",\
					       FALSE, NULL)

#define REDEMPTION_DATA_DIR gnome_program_locate_file (NULL,\
                                                       GNOME_FILE_DOMAIN_APP_DATADIR,\
                                                       "gbonds/",\
                                                       FALSE, NULL)

#define  DRUID_WIDTH  640
#define  DRUID_HEIGHT 400
#define  DRUID_BG_COLOR { 0, 70*256, 85*256, 80*256 }
#define  DRUID_TITLE_COLOR { 0, 255*256, 255*256, 255*256 }


/*===========================================*/
/* Private types                             */
/*===========================================*/

typedef struct {
  gchar *name;
  gchar *uri;
} Site;

typedef struct {
  gint            n;
  GList           *list;
  GnomeVFSURI     *uri;
} DirCallbackData;

typedef struct {
  gint            i, n;
  gint            total_bytes_read, total_bytes, file_bytes_read, file_bytes;
  GList           *list, *p;
  GnomeVFSURI     *uri;
  GnomeVFSHandle  *local_handle;
  gpointer        buffer;
  gint            buffer_length;
  gpointer        line_buffer;
  gint            line_offset;
} DownloadCallbackData;


/*===========================================*/
/* Private globals                           */
/*===========================================*/

GList *site_list = NULL;
gchar *site_uri;

GtkWidget *update_window = NULL;
GtkWidget *update_druid;
GtkWidget *download_page;
GtkWidget *site_entry;
GtkWidget *status_label;
GtkWidget *file_label;
GtkWidget *file_progress;
GtkWidget *total_progress;
GtkWidget *finish_page;

GnomeVFSAsyncHandle *remote_dir_handle = NULL;
GnomeVFSAsyncHandle *remote_file_handle = NULL;
gboolean update_cancel_flag = FALSE;



/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void add_start_page( GnomeDruid *wdruid,
			    GdkPixbuf *logo );
static void add_select_download_page( GnomeDruid *wdruid,
				      GdkPixbuf *logo );
static void add_downloading_page( GnomeDruid *wdruid, GdkPixbuf *logo );
static void prepare_downloading_page( GnomeDruidPage *page, GnomeDruid *druid,
				      gpointer data );

static void got_remote_list( GnomeVFSURI *uri, GList *list, gint n );
static GList *prune_list_from_dir( GList *list, gint *n,
				   const gchar *dirname );
static void get_remote_file_list( GnomeVFSURI *uri );
static void get_remote_file_list_cb( GnomeVFSAsyncHandle *handle,
				     GnomeVFSResult result,
				     GList *list,
				     guint entries_read,
				     gpointer callback_data );

static void download_files( GnomeVFSURI *uri, GList *list, gint n );
static void open_remote_file_callback( GnomeVFSAsyncHandle *handle,
				       GnomeVFSResult result,
				       gpointer callback_data );
static gchar *hash_filename( const gchar *filename );
static void read_remote_file_callback( GnomeVFSAsyncHandle *handle,
				       GnomeVFSResult result,
				       gpointer buffer,
				       GnomeVFSFileSize bytes_requested,
				       GnomeVFSFileSize bytes_read,
				       gpointer callback_data );
static void close_remote_file_callback( GnomeVFSAsyncHandle *handle,
					GnomeVFSResult result,
					gpointer callback_data );

static void download_done( DownloadCallbackData *data );
static void no_download( void );

static void add_finish_page( GnomeDruid *wdruid, GdkPixbuf *logo );
static void prepare_finish_page( GnomeDruidPage *page,
				 GnomeDruid *druid, gpointer data );

static void
finish_cb( GnomeDruidPage *page, GnomeDruid *druid, gpointer data );
static void
cancel_cb( GnomeDruid *druid, gpointer data );
static void
destroy_cb( GtkWindow *window, gpointer data );

static
GList *read_site_list( void );

static void
update_progress_bar (GtkProgressBar *bar,
		     gint            bytes,
		     gint            total);



/*--------------------------------------------------------------------------*/
/* Update command.  Creates and runs druid.                                 */
/*--------------------------------------------------------------------------*/
void
gb_update_druid (void)
{
	GdkPixbuf *logo;

	gb_debug (DEBUG_UPDATE, "START");

	if ( update_window == NULL ) {
		
		remote_dir_handle = NULL;
		remote_file_handle = NULL;
		update_cancel_flag = FALSE;

		logo = gdk_pixbuf_new_from_file (ICON_PIXMAP, NULL);

		update_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size (GTK_WINDOW(update_window),
					     DRUID_WIDTH, DRUID_HEIGHT);
		update_druid = gnome_druid_new();
		gtk_container_add( GTK_CONTAINER(update_window), update_druid );

		add_start_page( GNOME_DRUID(update_druid), logo );
		add_select_download_page( GNOME_DRUID(update_druid), logo );
		add_downloading_page( GNOME_DRUID(update_druid), logo );
		add_finish_page( GNOME_DRUID(update_druid), logo );
		
		g_signal_connect( G_OBJECT(update_druid), "cancel",
				  G_CALLBACK(cancel_cb), NULL );

		g_signal_connect( G_OBJECT(update_window), "destroy",
				  G_CALLBACK(destroy_cb), NULL );

		gtk_widget_show_all( update_window );
	}

	gb_debug (DEBUG_UPDATE, "END");

}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create and add start page to druid.                            */
/*--------------------------------------------------------------------------*/
static void
add_start_page( GnomeDruid *wdruid, GdkPixbuf *logo )
{
	GtkWidget    *wpage;
	GdkColor      druid_bg_color = DRUID_BG_COLOR;
	GdkColor      druid_title_color = DRUID_TITLE_COLOR;
	gbDate        date_min, date_max, date_today;
	gchar        *date_min_string, *date_max_string, *date_today_string;
	gchar        *msg, *msg1;
	gbTableModel *table_model;

	gb_debug (DEBUG_UPDATE, "START");

	table_model = gb_table_get_model ();

	date_min   = gb_table_model_get_rdate_min (table_model);
	date_max   = gb_table_model_get_rdate_max (table_model);
	date_today = gb_table_model_get_rdate_today ();

	date_min_string   = gb_date_fmt( date_min );
	date_max_string   = gb_date_fmt( date_max );
	date_today_string = gb_date_fmt( date_today );

	if ( date_today == date_max ) {
		msg1 = g_strdup_printf(
			 _( "Current redemption data will expire at the end\n"
			    "of this month (%s)\n" ), date_today_string );
	}
	else {
		if ( date_today > date_max ) {
			msg1 = g_strdup( _( "Current redemption data has expired.\n" ) );
		}
		else {
			msg1 = g_strdup( "" );
		}
	}

	msg = g_strdup_printf(
		   _( "%s\n"
		      "GBonds is currently configured with redemption data\n"
		      "for %s - %s.\n\n"
		      "New redemption data is published by the U.S. Treasury\n"
		      "every six months.  This dialog will help you\n"
		      "download and install new redemption data.\n\n"
		      "You can hit cancel at any point to cancel this dialog."
		      ),
		   msg1, date_min_string, date_max_string );

	wpage = gnome_druid_page_edge_new_with_vals (GNOME_EDGE_START,
						     TRUE,
						     _("Update redemption tables"),
						     msg,
						     logo,
						     NULL,
						     NULL);
	gnome_druid_page_edge_set_bg_color (GNOME_DRUID_PAGE_EDGE(wpage),
					    &druid_bg_color);
	gnome_druid_page_edge_set_logo_bg_color (GNOME_DRUID_PAGE_EDGE(wpage),
						 &druid_bg_color);
	gnome_druid_page_edge_set_title_color (GNOME_DRUID_PAGE_EDGE(wpage),
					       &druid_title_color);

	gnome_druid_append_page( wdruid, GNOME_DRUID_PAGE(wpage) );

	g_free( date_min_string );
	g_free( date_max_string );
	g_free( date_today_string );
	g_free( msg );
	g_free( msg1 );

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create and add "select download site" page to druid.           */
/*--------------------------------------------------------------------------*/
static void
add_select_download_page( GnomeDruid *wdruid, GdkPixbuf *logo )
{
	GtkWidget *wvbox;
	GdkColor   druid_bg_color = DRUID_BG_COLOR;
	GdkColor   druid_title_color = DRUID_TITLE_COLOR;
	GtkWidget *whbox, *wcombo;
	GList      *name_list = NULL;
	GList      *p;
	Site       *site;

	gb_debug (DEBUG_UPDATE, "START");

	download_page = gnome_druid_page_standard_new_with_vals (_("Select download site"),
								 logo,
								 NULL);
	gnome_druid_page_standard_set_background(GNOME_DRUID_PAGE_STANDARD(download_page),
						 &druid_bg_color);
	gnome_druid_page_standard_set_logo_background(GNOME_DRUID_PAGE_STANDARD(download_page),
						      &druid_bg_color);
	gnome_druid_page_standard_set_title_foreground( GNOME_DRUID_PAGE_STANDARD(download_page),
						   &druid_title_color);
	gnome_druid_append_page( wdruid, GNOME_DRUID_PAGE(download_page) );

	wvbox = GNOME_DRUID_PAGE_STANDARD(download_page)->vbox;

	whbox = gtk_hbox_new( FALSE, 10 );
	gtk_container_set_border_width( GTK_CONTAINER(whbox), 20 );
	gtk_box_pack_start( GTK_BOX(wvbox), whbox, TRUE, TRUE, 0 );

	gtk_box_pack_start( GTK_BOX(whbox),
			    gtk_label_new( _("Download site:") ),
			    FALSE, TRUE, 0 );

	if ( site_list == NULL ) {
		site_list = read_site_list();
	}
	for ( p=site_list; p!= NULL; p=p->next ) {
		site = (Site *)p->data;
		name_list = g_list_append( name_list, site->name );
	}
	wcombo = gtk_combo_new();
	gtk_combo_set_popdown_strings( GTK_COMBO(wcombo), name_list );
	g_list_free( name_list );
	name_list = NULL;
	site_entry = GTK_COMBO(wcombo)->entry;
	gtk_entry_set_editable( GTK_ENTRY(site_entry), FALSE );
	gtk_box_pack_start( GTK_BOX(whbox), wcombo, TRUE, TRUE, 0 );

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create and add "downloading" page to druid.                    */
/*--------------------------------------------------------------------------*/
static void
add_downloading_page( GnomeDruid *wdruid, GdkPixbuf *logo )
{
	GtkWidget *wpage, *wvbox, *wvbox2, *wvbox3, *wvbox4;
	GtkWidget *whbox2, *whbox3, *whbox4;
	GtkWidget *wlabel1, *wlabel2, *wframe;
	GdkColor   druid_bg_color = DRUID_BG_COLOR;
	GdkColor   druid_title_color = DRUID_TITLE_COLOR;

	gb_debug (DEBUG_UPDATE, "START");

	wpage = gnome_druid_page_standard_new_with_vals (_("Downloading..."),
							 logo,
							 NULL);
	gnome_druid_page_standard_set_background(GNOME_DRUID_PAGE_STANDARD(wpage),
						 &druid_bg_color);
	gnome_druid_page_standard_set_logo_background(GNOME_DRUID_PAGE_STANDARD(wpage),
						      &druid_bg_color);
	gnome_druid_page_standard_set_title_foreground(GNOME_DRUID_PAGE_STANDARD(wpage),
						       &druid_title_color);
	gnome_druid_append_page( wdruid, GNOME_DRUID_PAGE(wpage) );

	wvbox = GNOME_DRUID_PAGE_STANDARD(wpage)->vbox;

	wvbox2 = gtk_vbox_new( FALSE, 10 );
	gtk_container_set_border_width( GTK_CONTAINER(wvbox2), 20 );
	gtk_box_pack_start( GTK_BOX(wvbox), wvbox2, TRUE, FALSE, 0 );

	wlabel1 = gtk_label_new( _("Please wait while GBonds downloads new redemption data.") );
	gtk_box_pack_start( GTK_BOX(wvbox2), wlabel1, FALSE, TRUE, 0 );

	wframe = gtk_frame_new( NULL );
	gtk_box_pack_start( GTK_BOX(wvbox2), wframe, FALSE, TRUE, 0 );
	whbox2 = gtk_hbox_new( FALSE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER(whbox2), 5 );
	gtk_container_add( GTK_CONTAINER(wframe), whbox2 );
	status_label = gtk_label_new( _("Status") );
	gtk_label_set_justify( GTK_LABEL(status_label), GTK_JUSTIFY_LEFT );
	gtk_box_pack_start( GTK_BOX(whbox2), status_label, FALSE, TRUE, 0 );

	wvbox3 = gtk_vbox_new( FALSE, 2 );
	gtk_box_pack_start( GTK_BOX(wvbox2), wvbox3, TRUE, FALSE, 0 );

	whbox3 = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(wvbox3), whbox3, FALSE, TRUE, 0 );
	file_label = gtk_label_new( _("File") );
	gtk_label_set_justify( GTK_LABEL(file_label), GTK_JUSTIFY_LEFT );
	gtk_box_pack_start( GTK_BOX(whbox3), file_label, FALSE, TRUE, 0 );

	file_progress = gtk_progress_bar_new();
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR(file_progress), "" );
	gtk_box_pack_start( GTK_BOX(wvbox3), file_progress, FALSE, TRUE, 0 );

	wvbox4 = gtk_vbox_new( FALSE, 2 );
	gtk_box_pack_start( GTK_BOX(wvbox2), wvbox4, TRUE, FALSE, 0 );

	whbox4 = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(wvbox4), whbox4, FALSE, TRUE, 0 );
	wlabel2 = gtk_label_new( _("Total") );
	gtk_label_set_justify( GTK_LABEL(wlabel2), GTK_JUSTIFY_LEFT );
	gtk_box_pack_start( GTK_BOX(whbox4), wlabel2, FALSE, TRUE, 0 );

	total_progress = gtk_progress_bar_new();
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR(total_progress), "" );
	gtk_box_pack_start( GTK_BOX(wvbox4), total_progress, FALSE, TRUE, 0 );

	g_signal_connect_after( G_OBJECT(wpage), "prepare",
				G_CALLBACK(prepare_downloading_page), NULL );

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare downloading page callback.  This is the routine that   */
/* kicks off the entire async download process.                             */
/*--------------------------------------------------------------------------*/
static void
prepare_downloading_page( GnomeDruidPage *page, GnomeDruid *druid,
			  gpointer data )
{
	const gchar *site_name;
	GList *p;
	Site *site;

	gb_debug (DEBUG_UPDATE, "START");

	gnome_druid_set_buttons_sensitive( druid, FALSE, FALSE, TRUE, FALSE );
	site_name = gtk_entry_get_text( GTK_ENTRY(site_entry) );
	for ( p = site_list; p != NULL; p=p->next ) {
		site = (Site *)p->data;
		if ( strcmp( site_name, site->name ) == 0 ) {
			site_uri = site->uri;
			break;
		}
	}

	get_remote_file_list( gnome_vfs_uri_new( site_uri ) );

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Begin the asynchronous process of contacting the given site    */
/* and getting a list of apropriate files from the site.                    */
/*--------------------------------------------------------------------------*/
static void get_remote_file_list( GnomeVFSURI *uri )
{
	DirCallbackData *data;
	gchar           *status_string;

	gb_debug (DEBUG_UPDATE, "START");

	status_string = g_strdup_printf( _("Connecting to \"%s\" ..."), site_uri );
	gtk_label_set_text( GTK_LABEL(status_label), status_string );
	g_free( status_string );

	data = g_new0( DirCallbackData, 1 );
	data->n = 0;
	data->list = NULL;
	data->uri = uri;

	gnome_vfs_async_load_directory_uri (&remote_dir_handle,
					    uri,
					    GNOME_VFS_FILE_INFO_DEFAULT,
					    1,
					    0,
					    get_remote_file_list_cb,
					    data);

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Asynchronous callback for each directory entry from remote     */
/* site.                                                                    */
/*--------------------------------------------------------------------------*/
static void get_remote_file_list_cb( GnomeVFSAsyncHandle *handle,
				     GnomeVFSResult result,
				     GList *list,
				     guint entries_read,
				     gpointer callback_data )
{
	DirCallbackData  *data = (DirCallbackData *)callback_data;
	GnomeVFSFileInfo *info, *safe_info;
	gchar            *ext;

	gb_debug (DEBUG_UPDATE, "START");

	if ( update_cancel_flag ) {
		gnome_vfs_async_cancel( handle );
		gtk_widget_destroy( update_window );
		update_window = NULL;
		gb_debug (DEBUG_UPDATE, "END -- CANCEL");
		return;
	}

	switch (result) {

	case GNOME_VFS_OK:
		info = (GnomeVFSFileInfo *)list->data;
		ext = strrchr( info->name, '.' );
		if ( ext &&
		     (g_strcasecmp( ext, ".asc" ) == 0 ) &&
		     (strncasecmp( info->name, "sb", 2 ) == 0) ) {
			safe_info = gnome_vfs_file_info_dup( info );
			data->list = g_list_prepend( data->list, safe_info );
			data->n++;
		}
		break;

	case GNOME_VFS_ERROR_EOF:
		got_remote_list( data->uri, data->list, data->n );
		break;

	default:
		g_warning( "Unexpected error: %d: %s\n",
			   result, gnome_vfs_result_to_string(result) );
		break;

	}


	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Called once the remote file list is completed.  Kicks off the  */
/* asynchronous process of downloading any new files in that list.          */
/*--------------------------------------------------------------------------*/
static void got_remote_list( GnomeVFSURI *uri, GList *list, gint n )
{
	gchar *home_data_dir = gb_util_get_home_data_dir();

	gb_debug (DEBUG_UPDATE, "START");

	/*
	 * Now we have a list of remote redemption files, now lets remove any
	 * files from that list that we already have locally.
	 */
	list = prune_list_from_dir( list, &n, REDEMPTION_DATA_DIR );
	list = prune_list_from_dir( list, &n, home_data_dir );

	/*
	 * Now begin download of what's left.
	 */
	download_files( uri, list, n );

	g_free( home_data_dir );

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prune list of files by removing any duplicates in the given    */
/* local directory.                                                         */
/*--------------------------------------------------------------------------*/
static GList *
prune_list_from_dir( GList *remote_list, gint *n, const gchar *dirname )
{
	GnomeVFSFileInfo *local_info, *remote_info;
	GList            *local_list;
	GList            *p_remote, *p_local;
	gchar            *local_text_uri;
	GnomeVFSResult    ret;

	gb_debug (DEBUG_UPDATE, "START");

	if ( dirname == NULL ) return remote_list;

	local_text_uri = gnome_vfs_get_uri_from_local_path( dirname );
	if ( (ret = gnome_vfs_directory_list_load (&local_list,
						   local_text_uri,
						   GNOME_VFS_FILE_INFO_DEFAULT))
	     != GNOME_VFS_OK ) {
		return remote_list;
	}

	for ( p_local=local_list; p_local != NULL; p_local=p_local->next ) {
		local_info = (GnomeVFSFileInfo *)p_local->data;

		for ( p_remote=remote_list; p_remote != NULL; p_remote=p_remote->next ) {
			remote_info = (GnomeVFSFileInfo *)p_remote->data;
			if ( g_strcasecmp( local_info->name, remote_info->name ) == 0 ) {
				remote_list = g_list_remove_link( remote_list, p_remote );
				gnome_vfs_file_info_unref( remote_info );
				p_remote->data = remote_info = NULL;
				g_list_free_1( p_remote ); p_remote = NULL;
				*n -= 1;
				break;
			}
		}

	}
  
	g_free( local_text_uri );

	gb_debug (DEBUG_UPDATE, "END");

	return remote_list;
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Start downloading files in the list from remote site.          */
/*--------------------------------------------------------------------------*/
static void download_files( GnomeVFSURI *uri, GList *list, gint n )
{
	GnomeVFSFileInfo     *info;
	GnomeVFSURI          *file_uri;
	DownloadCallbackData *data;
	gint                  total_bytes;
	GList                *p;
	gchar                *status_string;

	gb_debug (DEBUG_UPDATE, "START");

	if ( list ) {

		status_string = g_strdup_printf( _("Downloading redemption tables...") );
		gtk_label_set_text( GTK_LABEL(status_label), status_string );
		g_free( status_string );

		total_bytes = 0;
		for ( p=list; p!=NULL; p=p->next ) {
			info = (GnomeVFSFileInfo *)p->data;
			total_bytes += info->size;
		}

		data = g_new0( DownloadCallbackData, 1 );
		data->i = 0;
		data->n = n;
		data->total_bytes_read = 0;
		data->total_bytes = total_bytes;
		data->list = list;
		data->p = list;
		data->uri = uri;
		data->buffer = g_malloc0( 4096 );
		data->buffer_length = 4096;
		data->line_buffer = g_malloc0( 4096 );
		data->local_handle = NULL;

		info = (GnomeVFSFileInfo *)data->p->data;
		gb_debug (DEBUG_UPDATE, "Opening %s", info->name );
		file_uri = gnome_vfs_uri_append_file_name( data->uri, info->name );
		gb_debug (DEBUG_UPDATE, " URI = %s",
			  gnome_vfs_uri_to_string (file_uri, 0) );
  
		gnome_vfs_async_open_uri (&remote_file_handle,
					  file_uri,
					  GNOME_VFS_OPEN_READ,
					  0,
					  open_remote_file_callback,
					  data);
	}
	else {
		no_download();
	}

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Remote file open callback.  Once open, open the local file     */
/* and begin the process of copying data from the remote file to the local  */
/* file.                                                                    */
/*--------------------------------------------------------------------------*/
static void open_remote_file_callback( GnomeVFSAsyncHandle *handle,
				       GnomeVFSResult result,
				       gpointer callback_data )
{
	DownloadCallbackData *data = (DownloadCallbackData *)callback_data;
	GnomeVFSFileInfo     *info = (GnomeVFSFileInfo *)data->p->data;
	gchar                *local_path, *local_text_uri, *local_name;
	GnomeVFSHandle       *local_handle;
	gchar                *status_string;
	GnomeVFSResult        ret;

	gb_debug (DEBUG_UPDATE, "START");

	if ( update_cancel_flag ) {
		gnome_vfs_async_close( handle, close_remote_file_callback, data );
		gb_debug (DEBUG_UPDATE, "END -- CANCEL");
		return;
	}

	switch (result) {

	case GNOME_VFS_OK:
		data->file_bytes_read = 0;
		data->file_bytes      = info->size;

		status_string = g_strdup_printf( _("File: \"%s\" (%d of %d)"),
						 info->name, (data->i+1), data->n );
		gtk_label_set_text( GTK_LABEL(file_label), status_string );
		g_free( status_string );

		update_progress_bar (GTK_PROGRESS_BAR(file_progress),
				     0,
				     data->file_bytes);
		update_progress_bar (GTK_PROGRESS_BAR(total_progress),
				     data->total_bytes_read,
				     data->total_bytes);
    
		local_name = hash_filename( info->name );
		local_path = g_build_filename (gb_util_get_home_data_dir(),
					       local_name,
					       NULL);
		local_text_uri = gnome_vfs_get_uri_from_local_path( local_path );
		ret = gnome_vfs_create( &local_handle, local_text_uri,
					GNOME_VFS_OPEN_WRITE, FALSE, 0664 );
		if ( ret != GNOME_VFS_OK ) {
			g_warning( "error opening local file %s, %s\n", local_path,
				   gnome_vfs_result_to_string(ret) );
		}
		else {
			data->local_handle = local_handle;
			gnome_vfs_async_read( handle, data->buffer, data->buffer_length,
					      read_remote_file_callback, data );
		}
		g_free( local_name );
		g_free( local_path );
		g_free( local_text_uri );
		break;

	default:
		g_warning( "Open failed: %s.\n", gnome_vfs_result_to_string(result) );
		break;

	}

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Hash filename.  Create a temporary filename for downloaded     */
/* file, so that we can cancel gracefully.                                  */
/*--------------------------------------------------------------------------*/
static gchar *hash_filename( const gchar *filename )
{
	return g_strdup_printf( "#%s#", filename );
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Remote read callback.  Take a data block read from the remote  */
/* file, and copy it to the local file.                                     */
/*--------------------------------------------------------------------------*/
static void
read_remote_file_callback( GnomeVFSAsyncHandle *handle,
			   GnomeVFSResult result,
			   gpointer buffer,
			   GnomeVFSFileSize bytes_requested,
			   GnomeVFSFileSize bytes_read,
			   gpointer callback_data )
{
	DownloadCallbackData *data = (DownloadCallbackData *)callback_data;
	GnomeVFSFileSize      bytes_written;

	gb_debug (DEBUG_UPDATE, "START");

	if ( update_cancel_flag ) {
		gnome_vfs_async_close( handle, close_remote_file_callback, data );
		gb_debug (DEBUG_UPDATE, "END -- CANCEL");
		return;
	}

	switch (result) {

	case GNOME_VFS_OK:
		if ( bytes_read > 0 ) {
			data->total_bytes_read += bytes_read;
			data->file_bytes_read += bytes_read;

			update_progress_bar (GTK_PROGRESS_BAR(file_progress),
					     data->file_bytes_read,
					     data->file_bytes);
			update_progress_bar (GTK_PROGRESS_BAR(total_progress),
					     data->total_bytes_read,
					     data->total_bytes);

			gnome_vfs_write( data->local_handle,
					 buffer, bytes_read, &bytes_written );
			if ( bytes_written != bytes_read ) {
				g_warning( "Write failed: %d bytes written != %d bytes read",
					   (gint)bytes_written, (gint)bytes_read );
			}
			gnome_vfs_async_read( handle, data->buffer, data->buffer_length,
					      read_remote_file_callback, data );
		}
		else {
			gnome_vfs_async_close( handle, close_remote_file_callback, data );
			gb_debug (DEBUG_UPDATE, "0 length read");
		}
		break;

	case GNOME_VFS_ERROR_EOF:
		gb_debug (DEBUG_UPDATE, "EOF -- %" GNOME_VFS_OFFSET_FORMAT_STR,
			  bytes_read);
		gnome_vfs_async_close( handle, close_remote_file_callback, data );
		break;

	default:
		g_warning( "Read failed: %s", gnome_vfs_result_to_string(result) );
		break;

	}

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Remote file closed callback.  Close local file and open the    */
/* next remote file in the list, if any.                                    */
/*--------------------------------------------------------------------------*/
static void close_remote_file_callback( GnomeVFSAsyncHandle *handle,
					GnomeVFSResult result,
					gpointer callback_data )
{
	DownloadCallbackData *data = (DownloadCallbackData *)callback_data;
	GnomeVFSFileInfo     *info;
	GnomeVFSURI          *file_uri;

	gb_debug (DEBUG_UPDATE, "START");

	if ( data->local_handle ) {
		gnome_vfs_close( data->local_handle );
		data->local_handle = NULL;
	}

	if ( update_cancel_flag ) {
		gtk_widget_destroy( update_window );
		update_window = NULL;
		gb_debug (DEBUG_UPDATE, "END -- CANCEL");
		return;
	}

	if (result != GNOME_VFS_OK) {
		g_warning( "Close failed: %s", gnome_vfs_result_to_string(result) );
	}

	data->i++;
	data->p = data->p->next;
	if ( data->p ) {
		info = (GnomeVFSFileInfo *)data->p->data;
		gb_debug (DEBUG_UPDATE, "Opening %s", info->name );
		file_uri = gnome_vfs_uri_append_file_name( data->uri, info->name );
		gb_debug (DEBUG_UPDATE, " URI = %s",
			  gnome_vfs_uri_to_string (file_uri, 0) );
		gnome_vfs_async_cancel (handle);
		gnome_vfs_async_open_uri (&remote_file_handle,
					  file_uri,
					  GNOME_VFS_OPEN_READ,
					  0,
					  open_remote_file_callback,
					  data);
	}
	else {
		download_done( data );
	}

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Download successful.  Finish up and present an apropriate      */
/* finish page.                                                             */
/*--------------------------------------------------------------------------*/
static void
download_done( DownloadCallbackData *data )
{
	GList            *p;
	GnomeVFSFileInfo *info, tmp_info = {0};
	gchar            *hash_name;
	gchar            *hash_path, *hash_text_uri;
	gchar            *file_path, *file_text_uri;
	gchar            *date_min, *date_max, *finish_msg;
	gchar            *data_dir;
	gbTableModel     *table_model;

	gb_debug (DEBUG_UPDATE, "START");

	/* Rename downloaded files (undo name hash) */
	data_dir = gb_util_get_home_data_dir();
	for ( p=data->list; p != NULL; p=p->next ) {
		info = (GnomeVFSFileInfo *)p->data;
		hash_name = hash_filename( info->name );
		hash_path = g_build_filename( data_dir, hash_name, NULL );
		hash_text_uri = gnome_vfs_get_uri_from_local_path( hash_path );
		file_path = g_build_filename( data_dir, info->name, NULL );
		file_text_uri = gnome_vfs_get_uri_from_local_path( file_path );
		gnome_vfs_get_file_info (hash_text_uri, &tmp_info, GNOME_VFS_FILE_INFO_DEFAULT);
		if ( info->size == tmp_info.size ) {
			gnome_vfs_move( hash_text_uri, file_text_uri, FALSE );
		} else {
			g_warning ("%s: Temporary file size (%"
				   GNOME_VFS_OFFSET_FORMAT_STR
				   ") does not match remote size (%"
				   GNOME_VFS_OFFSET_FORMAT_STR
				   ").",
				   info->name, tmp_info.size, info->size);
			gnome_vfs_unlink (hash_text_uri);
		}
		g_free( hash_name );
		g_free( hash_path );
		g_free( hash_text_uri );
		g_free( file_path );
		g_free( file_text_uri );
	}
	g_free( data_dir );

	/* Now reread redemption tables */
	table_model = gb_table_get_model ();
	gb_table_model_update (table_model);

	/* customize finish page for this outcome */
	gnome_druid_page_edge_set_title( GNOME_DRUID_PAGE_EDGE(finish_page),
					 _( "Download done" ) );
	date_min = gb_date_fmt (gb_table_model_get_rdate_min (table_model));
	date_max = gb_date_fmt (gb_table_model_get_rdate_max (table_model));
	finish_msg = g_strdup_printf(
		_( "GBonds has successfully downloaded "
		   "%d new redemption files.\n\n"
		   "GBonds is now configured with redemption data\n"
		   "for %s - %s.\n" ),
		data->n, date_min, date_max );
	gnome_druid_page_edge_set_text( GNOME_DRUID_PAGE_EDGE(finish_page), finish_msg );
	g_free( date_min );
	g_free( date_max );
	g_free( finish_msg );

	remote_dir_handle = NULL;

	/* Now jump to the finish page */
	gnome_druid_set_page( GNOME_DRUID(update_druid),
			      GNOME_DRUID_PAGE(finish_page) );

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  No dowload was performed.  Finish up and present an apropriate */
/* finish page.                                                             */
/*--------------------------------------------------------------------------*/
static void
no_download( void )
{
	gchar *date_min, *date_max, *finish_msg;
	gbTableModel     *table_model;

	gb_debug (DEBUG_UPDATE, "START");

	/* customize finish page for this outcome */
	gnome_druid_page_edge_set_title( GNOME_DRUID_PAGE_EDGE(finish_page),
					 _( "No new data available" ) );
	table_model = gb_table_get_model ();
	date_min = gb_date_fmt (gb_table_model_get_rdate_min (table_model));
	date_max = gb_date_fmt (gb_table_model_get_rdate_max (table_model));
	finish_msg = g_strdup_printf(
		_( "No new redemption data available from selected\n"
		   "download site.\n\n"
		   "GBonds is still configured with redemption data\n"
		   "for %s - %s.\n" ),
		date_min, date_max );
	gnome_druid_page_edge_set_text( GNOME_DRUID_PAGE_EDGE(finish_page), finish_msg );
	g_free( date_min );
	g_free( date_max );
	g_free( finish_msg );

	remote_dir_handle = NULL;

	/* Now jump to the finish page */
	gnome_druid_set_page( GNOME_DRUID(update_druid),
			      GNOME_DRUID_PAGE(finish_page) );

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add finish page to druid.                                      */
/*--------------------------------------------------------------------------*/
static void
add_finish_page( GnomeDruid *wdruid, GdkPixbuf *logo )
{
	GdkColor   druid_bg_color = DRUID_BG_COLOR;
	GdkColor   druid_title_color = DRUID_TITLE_COLOR;

	gb_debug (DEBUG_UPDATE, "START");

	finish_page = gnome_druid_page_edge_new_with_vals (GNOME_EDGE_FINISH,
							   TRUE,
							   _("done"),
							   _("Done."),
							   logo,
							   NULL,
							   NULL);
	gnome_druid_page_edge_set_bg_color(GNOME_DRUID_PAGE_EDGE(finish_page),
					   &druid_bg_color);
	gnome_druid_page_edge_set_logo_bg_color (GNOME_DRUID_PAGE_EDGE(finish_page),
						 &druid_bg_color );
	gnome_druid_page_edge_set_title_color (GNOME_DRUID_PAGE_EDGE(finish_page),
					       &druid_title_color );
	gnome_druid_append_page( wdruid, GNOME_DRUID_PAGE(finish_page) );

	g_signal_connect_after( G_OBJECT(finish_page), "prepare",
				G_CALLBACK(prepare_finish_page), NULL );

	g_signal_connect( G_OBJECT(finish_page), "finish",
			  G_CALLBACK(finish_cb), NULL );

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare finish page callback.                                  */
/*--------------------------------------------------------------------------*/
static void
prepare_finish_page( GnomeDruidPage *page, GnomeDruid *druid, gpointer data )
{

	gb_debug (DEBUG_UPDATE, "START");

	gnome_druid_set_show_finish( GNOME_DRUID(druid), TRUE );

	gnome_druid_set_buttons_sensitive( GNOME_DRUID(druid),
					   FALSE, TRUE, FALSE, FALSE );

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  End druid.                                                     */
/*--------------------------------------------------------------------------*/
static void
finish_cb( GnomeDruidPage *page, GnomeDruid *druid, gpointer data )
{
	gb_debug (DEBUG_UPDATE, "START");

	g_signal_handlers_block_by_func( G_OBJECT(update_window),
					 G_CALLBACK(destroy_cb), NULL );

	gtk_widget_destroy( update_window );
	update_window = NULL;

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Cancel druid.                                                  */
/*--------------------------------------------------------------------------*/
static void
cancel_cb( GnomeDruid *druid, gpointer data )
{
	gb_debug (DEBUG_UPDATE, "START");

	g_signal_handlers_block_by_func( G_OBJECT(update_window),
					 G_CALLBACK(destroy_cb), NULL );
	if ( remote_dir_handle ) {

		/* let the async stuff know to exit as gracefully as possible. */
		update_cancel_flag = TRUE;

	}
	else {

		/* we haven't started any of the async stuff yet, so just destroy window */
		gtk_widget_destroy( update_window );
		update_window = NULL;

	}

	gb_debug (DEBUG_UPDATE, "END");
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Destroy druid.                                                 */
/*--------------------------------------------------------------------------*/
static void
destroy_cb( GtkWindow *window, gpointer data )
{
	gb_debug (DEBUG_UPDATE, "START");

	if ( remote_dir_handle ) {

		/* let the async stuff know to exit as gracefully as possible. */
		update_cancel_flag = TRUE;
		g_signal_stop_emission_by_name( G_OBJECT(update_window), "destroy" );

	}
	else {

		/* we haven't started any of the async stuff yet, so just destroy window */
		gtk_widget_destroy( update_window );
		update_window = NULL;

	}

	gb_debug (DEBUG_UPDATE, "END");
}




/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read download site-list file.                                  */
/*--------------------------------------------------------------------------*/
static
GList *read_site_list( void )
{
	gchar     *xml_filename;
	xmlDocPtr  doc;
	xmlNodePtr root, node;
	Site      *site;
	GList     *site_list = NULL;

	gb_debug (DEBUG_UPDATE, "START");

	xml_filename = g_build_filename( REDEMPTION_DATA_DIR,
					 "download-sites.xml",
					 NULL);
	doc = xmlParseFile( xml_filename );
	if ( !doc ) {
		g_warning( "xmlParseFile error \"%s\"", xml_filename );
		return NULL;
	}
	g_free( xml_filename );

	root = xmlDocGetRootElement( doc );
	if ( !root || !root->name ) {
		g_warning( "no document root \"%s\"", xml_filename );
		xmlFreeDoc( doc );
		return NULL;
	}
	if ( g_strcasecmp( (gchar *)root->name, "Redemption-Data-Sites" ) != 0 ) {
		g_warning( "bad root node = \"%s\"", root->name );
		xmlFreeDoc( doc );
		return NULL;
	}

	for ( node = root->xmlChildrenNode; node != NULL; node=node->next ) {
		if ( g_strcasecmp( (gchar *)node->name, "Site" ) == 0 ) {
			site = g_new0( Site, 1 );
			site->uri  = (gchar *)xmlGetProp( node, (xmlChar *)"uri" );
			site->name = (gchar *)xmlNodeGetContent( node );
			site_list = g_list_append( site_list, site );
		}
		else {
			if ( g_strcasecmp( (gchar *)node->name, "text" ) != 0 ) {
				g_warning( "bad node =  \"%s\"", node->name );
			}
		}
	}

	xmlFreeDoc( doc );

	gb_debug (DEBUG_UPDATE, "END");

	return site_list;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update progress bar.                                           */
/*--------------------------------------------------------------------------*/
static void
update_progress_bar (GtkProgressBar *bar,
		     gint            bytes,
		     gint            total)
{
	gchar *string;

	if ( total > 0 ) {

		string = g_strdup_printf ("%.0f%% (%d Bytes of %d Bytes)",
					  100.0*((gdouble)bytes/(gdouble)total),
					  bytes,
					  total);

		gtk_progress_bar_set_text (bar, string);
		gtk_progress_bar_set_fraction (bar, (gdouble)bytes/(gdouble)total);

		g_free (string);

	} else {

		gtk_progress_bar_set_text (bar, "");
		gtk_progress_bar_set_fraction (bar, 0.0);

	}
}


