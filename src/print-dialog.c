/*
 *  (GBONDS) GNOME based Savings Bond Inventory Program
 *
 *  print-dialog.c:  Print dialog module
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

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print-paper.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-job-preview.h>

#include "print-dialog.h"
#include "hig.h"
#include "print.h"
#include "doc.h"

#include "debug.h"

/*===========================================*/
/* Private types.                            */
/*===========================================*/

/*===========================================*/
/* Private globals.                          */
/*===========================================*/

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static void print_response (GtkDialog *dlg,
			    gint       response,
			    gbView    *view);


/*****************************************************************************/
/* "Print" dialog.                                                           */
/*****************************************************************************/
void
gb_print_dialog (gbView *view, BonoboWindow *win)
{
	GtkWidget *dlg;

	g_return_if_fail (view && GB_IS_VIEW(view));
	g_return_if_fail (win && BONOBO_IS_WINDOW(win));

	dlg = gnome_print_dialog_new (NULL, (guchar*)_("Print inventory"), 0);

	gtk_window_set_transient_for (GTK_WINDOW(dlg), GTK_WINDOW(win));

	g_signal_connect (G_OBJECT(dlg), "response",
                          G_CALLBACK (print_response), view);

	gtk_widget_show (dlg);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Print "response" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
print_response (GtkDialog *dlg,
		gint       response,
		gbView     *view)
{
	GnomePrintConfig *config;
	GnomePrintJob    *job;
	GtkWidget        *preview;

	switch (response) {

	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		config = gnome_print_dialog_get_config (GNOME_PRINT_DIALOG(dlg));
		job = gnome_print_job_new (config);
		gb_print (job, view);
		gnome_print_job_close (job);
		gnome_print_job_print (job);
		g_object_unref (G_OBJECT(job));
		break;

	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		config = gnome_print_dialog_get_config (GNOME_PRINT_DIALOG(dlg));
		job = gnome_print_job_new (config);
		gb_print (job, view);
		gnome_print_job_close (job);
		preview = gnome_print_job_preview_new (job, (guchar*)_("Print preview"));
		gtk_widget_show (preview);
		g_object_unref (G_OBJECT(job));
		break;

	default:
		break;

	}

	gtk_widget_destroy (GTK_WIDGET (dlg));
}

