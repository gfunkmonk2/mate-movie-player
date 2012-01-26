/* idol-interface.c
 *
 *  Copyright (C) 2005 Bastien Nocera
 *
 *  The Mate Library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  The Mate Library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with the Mate Library; see the file COPYING.LIB.  If not,
 *  write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301  USA.
 *
 *  Author: Bastien Nocera <hadess@hadess.net>
 * 
 * The Idol project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Idol. This
 * permission are above and beyond the permissions granted by the GPL license
 * Idol is covered by.
 *
 * Monday 7th February 2005: Christian Schaller: Add exception clause.
 * See license_change file for details.
 *
 */

/**
 * SECTION:idol-interface
 * @short_description: interface utility/loading/error functions
 * @stability: Unstable
 * @include: idol-interface.h
 *
 * A collection of interface utility functions, for loading interfaces and displaying errors.
 **/

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <mateconf/mateconf-client.h>

#include "idol-interface.h"
#include "idol-private.h"

static GtkWidget *
idol_interface_error_dialog (const char *title, const char *reason,
		GtkWindow *parent)
{
	GtkWidget *error_dialog;

	if (reason == NULL)
		g_warning ("idol_action_error called with reason == NULL");

	error_dialog =
		gtk_message_dialog_new (NULL,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				"%s", title);
	gtk_message_dialog_format_secondary_text
		(GTK_MESSAGE_DIALOG (error_dialog), "%s", reason);

	idol_interface_set_transient_for (GTK_WINDOW (error_dialog),
				GTK_WINDOW (parent));
	gtk_window_set_title (GTK_WINDOW (error_dialog), ""); /* as per HIG */
	gtk_container_set_border_width (GTK_CONTAINER (error_dialog), 5);
	gtk_dialog_set_default_response (GTK_DIALOG (error_dialog),
			GTK_RESPONSE_OK);
	gtk_window_set_modal (GTK_WINDOW (error_dialog), TRUE);

	return error_dialog;
}

/**
 * idol_interface_error:
 * @title: the error title
 * @reason: the error reason (secondary text)
 * @parent: the error dialogue's parent #GtkWindow
 *
 * Display a modal error dialogue with @title as its primary error text, and @reason
 * as its secondary text.
 **/
void
idol_interface_error (const char *title, const char *reason,
		GtkWindow *parent)
{
	GtkWidget *error_dialog;

	error_dialog = idol_interface_error_dialog (title, reason, parent);

	g_signal_connect (G_OBJECT (error_dialog), "response", G_CALLBACK
			(gtk_widget_destroy), error_dialog);

	gtk_window_present (GTK_WINDOW (error_dialog));
}

/**
 * idol_interface_error_blocking:
 * @title: the error title
 * @reason: the error reason (secondary text)
 * @parent: the error dialogue's parent #GtkWindow
 *
 * Display a modal error dialogue like idol_interface_error() which blocks until the user has
 * dismissed it.
 **/
void
idol_interface_error_blocking (const char *title, const char *reason,
		GtkWindow *parent)
{
	GtkWidget *error_dialog;

	error_dialog = idol_interface_error_dialog (title, reason, parent);

	gtk_dialog_run (GTK_DIALOG (error_dialog));
	gtk_widget_destroy (error_dialog);
}

/**
 * idol_interface_error_with_link:
 * @title: the error title
 * @reason: the error reason (secondary text)
 * @uri: the URI to open
 * @label: a label for the URI's button, or %NULL to use @uri as the label
 * @parent: the error dialogue's parent #GtkWindow
 * @idol: a #IdolObject
 *
 * Display a modal error dialogue like idol_interface_error(),
 * but add a button which will open @uri in a browser window.
 **/
void
idol_interface_error_with_link (const char *title, const char *reason,
				 const char *uri, const char *label, GtkWindow *parent, Idol *idol)
{
	GtkWidget *error_dialog, *link_button, *hbox;

	if (label == NULL)
		label = uri;

	error_dialog = idol_interface_error_dialog (title, reason, parent);
	link_button = gtk_link_button_new_with_label (uri, label);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), link_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (error_dialog))), hbox, TRUE, FALSE, 0);
	gtk_widget_show_all (hbox);

	gtk_dialog_set_default_response (GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);

	g_signal_connect (G_OBJECT (error_dialog), "response", G_CALLBACK
			(gtk_widget_destroy), error_dialog);

	gtk_window_present (GTK_WINDOW (error_dialog));
}

GtkBuilder *
idol_interface_load (const char *name, gboolean fatal, GtkWindow *parent, gpointer user_data)
{
	GtkBuilder *builder = NULL;
	char *filename;

	filename = idol_interface_get_full_path (name);
	if (filename == NULL) {
		char *msg;

		msg = g_strdup_printf (_("Couldn't load the '%s' interface. %s"), name, _("The file does not exist."));
		if (fatal == FALSE)
			idol_interface_error (msg, _("Make sure that Idol is properly installed."), parent);
		else
			idol_interface_error_blocking (msg, _("Make sure that Idol is properly installed."), parent);

		g_free (msg);
		return NULL;
	}

	builder = idol_interface_load_with_full_path (filename, fatal, parent,
						       user_data);
	g_free (filename);

	return builder;
}

GtkBuilder *
idol_interface_load_with_full_path (const char *filename, gboolean fatal, 
				     GtkWindow *parent, gpointer user_data)
{
	GtkBuilder *builder = NULL;
	GError *error = NULL;

	if (filename != NULL) {
		builder = gtk_builder_new ();
		gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	}

	if (builder == NULL || gtk_builder_add_from_file (builder, filename, &error) == FALSE) {
		char *msg;

		msg = g_strdup_printf (_("Couldn't load the '%s' interface. %s"), filename, error->message);
		if (fatal == FALSE)
			idol_interface_error (msg, _("Make sure that Idol is properly installed."), parent);
		else
			idol_interface_error_blocking (msg, _("Make sure that Idol is properly installed."), parent);

		g_free (msg);
		g_error_free (error);

		return NULL;
	}

	gtk_builder_connect_signals (builder, user_data);

	return builder;
}

GdkPixbuf*
idol_interface_load_pixbuf (const char *name)
{
	GdkPixbuf *pix;
	char *filename;

	filename = idol_interface_get_full_path (name);
	if (filename == NULL)
		return NULL;
	pix = gdk_pixbuf_new_from_file (filename, NULL);
	g_free (filename);
	return pix;
}

char *
idol_interface_get_full_path (const char *name)
{
	char *filename;

#ifdef IDOL_RUN_IN_SOURCE_TREE
	/* Try the GtkBuilder file in the source tree first */
	filename = g_build_filename ("..", "data", name, NULL);
	if (g_file_test (filename, G_FILE_TEST_EXISTS) == FALSE)
	{
		g_free (filename);
		/* Try the local file */
		filename = g_build_filename (DATADIR,
				"idol", name, NULL);

		if (g_file_test (filename, G_FILE_TEST_EXISTS) == FALSE)
		{
			g_free (filename);
			return NULL;
		}
	}
#else
	filename = g_build_filename (DATADIR,
	                                "idol", name, NULL);
#endif

	return filename;
}

static GdkWindow *
idol_gtk_plug_get_toplevel (GtkPlug *plug)
{
	Window root, parent, *children;
	guint nchildren;
	GdkNativeWindow xid;

	g_return_val_if_fail (GTK_IS_PLUG (plug), NULL);

	xid = gtk_plug_get_id (plug);

	do
	{
		/* FIXME: multi-head */
		if (XQueryTree (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), xid, &root,
					&parent, &children, &nchildren) == 0)
		{
			g_warning ("Couldn't find window manager window");
			return NULL;
		}

		if (root == parent) {
			GdkWindow *toplevel;
			toplevel = gdk_window_foreign_new (xid);
			return toplevel;
		}

		xid = parent;
	}
	while (TRUE);
}

void
idol_interface_set_transient_for (GtkWindow *window, GtkWindow *parent)
{
	if (GTK_IS_PLUG (parent)) {
		GdkWindow *toplevel;

		gtk_widget_realize (GTK_WIDGET (window));
		toplevel = idol_gtk_plug_get_toplevel (GTK_PLUG (parent));
		if (toplevel != NULL) {
			gdk_window_set_transient_for
				(gtk_widget_get_window (GTK_WIDGET (window)), toplevel);
			g_object_unref (toplevel);
		}
	} else {
		gtk_window_set_transient_for (GTK_WINDOW (window),
				GTK_WINDOW (parent));
	}
}

char *
idol_interface_get_license (void)
{
	const char *license[] = {
		N_("Idol is free software; you can redistribute it and/or modify "
		   "it under the terms of the GNU General Public License as published by "
		   "the Free Software Foundation; either version 2 of the License, or "
		   "(at your option) any later version."),
		N_("Idol is distributed in the hope that it will be useful, "
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
		   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		   "GNU General Public License for more details."),
		N_("You should have received a copy of the GNU General Public License "
		   "along with Idol; if not, write to the Free Software Foundation, Inc., "
		   "59 Temple Place, Suite 330, Boston, MA  02111-1307  USA"),
		N_("Idol contains an exception to allow the use of proprietary "
		   "GStreamer plugins.")
	};
	return g_strjoin ("\n\n",
			  _(license[0]),
			  _(license[1]),
			  _(license[2]),
			  _(license[3]),
			  NULL);
}
