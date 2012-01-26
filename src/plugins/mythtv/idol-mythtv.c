/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Bastien Nocera <hadess@hadess.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 * The Idol project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Idol. This
 * permission are above and beyond the permissions granted by the GPL license
 * Idol is covered by.
 *
 * See license_change file for details.
 *
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <mateconf/mateconf-client.h>
#include <gmodule.h>
#include <string.h>

#include "idol-video-list.h"
#include "idol-cell-renderer-video.h"
#include "video-utils.h"
#include "idol-plugin.h"
#include "idol.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <gmyth/gmyth_backendinfo.h>
#include <gmyth/gmyth_file_transfer.h>
#include <gmyth/gmyth_scheduler.h>
#include <gmyth/gmyth_epg.h>
#include <gmyth/gmyth_util.h>
#include <gmyth_upnp.h>


#define IDOL_TYPE_MYTHTV_PLUGIN		(idol_mythtv_plugin_get_type ())
#define IDOL_MYTHTV_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_MYTHTV_PLUGIN, IdolMythtvPlugin))
#define IDOL_MYTHTV_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_MYTHTV_PLUGIN, IdolMythtvPluginClass))
#define IDOL_IS_MYTHTV_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_MYTHTV_PLUGIN))
#define IDOL_IS_MYTHTV_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_MYTHTV_PLUGIN))
#define IDOL_MYTHTV_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_MYTHTV_PLUGIN, IdolMythtvPluginClass))

#define MYTHTV_SIDEBAR_RECORDINGS "mythtv-recordings"
#define MYTHTV_SIDEBAR_LIVETV "mythtv-livetv"

enum {
	FILENAME_COL,
	URI_COL,
	THUMBNAIL_COL,
	NAME_COL,
	DESCRIPTION_COL,
	NUM_COLS
};

typedef struct
{
	IdolPlugin parent;

	GList *lst_b_info;
	GMythUPnP *upnp;

	IdolObject *idol;
	MateConfClient *client;

	GtkWidget *sidebar_recordings;
	GtkWidget *sidebar_livetv;
} IdolMythtvPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolMythtvPluginClass;


G_MODULE_EXPORT GType register_idol_plugin	(GTypeModule *module);
GType	idol_mythtv_plugin_get_type		(void) G_GNUC_CONST;
static void idol_mythtv_plugin_finalize	(GObject *object);
static gboolean impl_activate			(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate			(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER(IdolMythtvPlugin, idol_mythtv_plugin)

#define MAX_THUMB_SIZE 500 * 1024 * 1024
#define THUMB_HEIGHT 32

static GdkPixbuf *
get_thumbnail (IdolMythtvPlugin *plugin, GMythBackendInfo *b_info, char *fname)
{
	GMythFileTransfer *transfer;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	GMythFileReadResult res;
	guint64 to_read;
	GByteArray *data;

	if (gmyth_util_file_exists (b_info, fname) == FALSE)
		return NULL;

	transfer = gmyth_file_transfer_new (b_info);
	if (gmyth_file_transfer_open(transfer, fname) == FALSE)
		return NULL;

	to_read = gmyth_file_transfer_get_filesize (transfer);
	/* Leave if the thumbnail is just too big */
	if (to_read > MAX_THUMB_SIZE) {
		gmyth_file_transfer_close (transfer);
		return NULL;
	}

	loader = gdk_pixbuf_loader_new_with_type ("png", NULL);
	data = g_byte_array_sized_new (to_read);

	res = gmyth_file_transfer_read(transfer, data, to_read, FALSE);
	if (gdk_pixbuf_loader_write (loader, data->data, to_read, NULL) == FALSE) {
		res = GMYTH_FILE_READ_ERROR;
	}

	gmyth_file_transfer_close (transfer);
	g_object_unref (transfer);
	g_byte_array_free (data, TRUE);

	if (res != GMYTH_FILE_READ_OK && res != GMYTH_FILE_READ_EOF) {
		g_object_unref (loader);
		return NULL;
	}

	if (gdk_pixbuf_loader_close (loader, NULL) == FALSE) {
		g_object_unref (loader);
		return NULL;
	}

	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	if (pixbuf != NULL)
		g_object_ref (pixbuf);
	g_object_unref (loader);

	if (pixbuf == NULL)
		return NULL;

	if (gdk_pixbuf_get_height (pixbuf) != THUMB_HEIGHT) {
		GdkPixbuf *scaled;
		int height, width;

		height = THUMB_HEIGHT;
		width = gdk_pixbuf_get_width (pixbuf) * height / gdk_pixbuf_get_height (pixbuf);
		scaled = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_BILINEAR);
		g_object_unref (pixbuf);

		return scaled;
	}

	return pixbuf;
}

static GtkWidget *
create_treeview (IdolMythtvPlugin *plugin)
{
	IdolCellRendererVideo *renderer;
	GtkWidget *treeview;
	GtkTreeModel *model;

	/* Treeview and model */
	model = GTK_TREE_MODEL (gtk_list_store_new (NUM_COLS,
						    G_TYPE_STRING,
						    G_TYPE_STRING,
						    G_TYPE_OBJECT,
						    G_TYPE_STRING,
						    G_TYPE_STRING));

	treeview = GTK_WIDGET (g_object_new (IDOL_TYPE_VIDEO_LIST,
					     "idol", plugin->idol,
					     "mrl-column", URI_COL,
					     "tooltip-column", DESCRIPTION_COL,
					     NULL));

	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview),
				 GTK_TREE_MODEL (model));

	renderer = idol_cell_renderer_video_new (TRUE);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview), 0,
						     _("Recordings"), GTK_CELL_RENDERER (renderer),
						     "thumbnail", THUMBNAIL_COL,
						     "title", NAME_COL,
						     NULL);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);

	return treeview;
}

static void
idol_mythtv_list_recordings (IdolMythtvPlugin *tm,
			      GMythBackendInfo *b_info)
{
	GMythScheduler *scheduler;
	GtkTreeModel *model;
	GList *list, *l;

	if (b_info == NULL)
		return;

	scheduler = gmyth_scheduler_new();
	if (gmyth_scheduler_connect_with_timeout(scheduler,
						 b_info, 5) == FALSE) {
		g_message ("Couldn't connect to scheduler");
		g_object_unref (scheduler);
		return;
	}

	if (gmyth_scheduler_get_recorded_list(scheduler, &list) < 0) {
		g_message ("Couldn't get recordings list");
		gmyth_scheduler_disconnect (scheduler);
		g_object_unref (scheduler);
		return;
	}

	gmyth_scheduler_disconnect (scheduler);
	g_object_unref (scheduler);

	model = g_object_get_data (G_OBJECT (tm->sidebar_recordings), "model");

	for (l = list; l != NULL; l = l->next) {
		RecordedInfo *recorded_info = (RecordedInfo *) l->data;

		if (gmyth_util_file_exists
		    (b_info, recorded_info->basename->str)) {
		    	GtkTreeIter iter;
		    	GdkPixbuf *pixbuf;
		    	char *full_name = NULL;
		    	char *thumb_fname, *uri;

		    	if (recorded_info->subtitle->str != NULL && recorded_info->subtitle->str[0] != '\0')
		    		full_name = g_strdup_printf ("%s - %s",
		    					     recorded_info->title->str,
		    					     recorded_info->subtitle->str);
		    	thumb_fname = g_strdup_printf ("%s.png", recorded_info->basename->str);
		    	uri = g_strdup_printf ("myth://%s:%d/%s",
		    			       b_info->hostname,
		    			       b_info->port,
		    			       recorded_info->basename->str);
		    	pixbuf = get_thumbnail (tm, b_info, thumb_fname);
		    	g_free (thumb_fname);

		    	gtk_list_store_insert_with_values (GTK_LIST_STORE (model), &iter, G_MAXINT32,
		    					   FILENAME_COL, recorded_info->basename->str,
		    					   URI_COL, uri,
		    					   THUMBNAIL_COL, pixbuf,
		    					   NAME_COL, full_name ? full_name : recorded_info->title->str,
		    					   DESCRIPTION_COL, recorded_info->description->str,
		    					   -1);
		    	g_free (full_name);
		    	g_free (uri);
		}
		gmyth_recorded_info_free(recorded_info);
	}

	g_list_free (list);
}

static gint
sort_channels (GMythChannelInfo *a, GMythChannelInfo *b)
{
	int a_int, b_int;

	a_int = g_strtod (a->channel_num->str, NULL);
	b_int = g_strtod (b->channel_num->str, NULL);

	if (a_int < b_int)
		return -1;
	if (a_int > b_int)
		return 1;
	return 0;
}

static void
idol_mythtv_list_livetv (IdolMythtvPlugin *tm,
			  GMythBackendInfo *b_info)
{
	GMythEPG *epg;
	int length;
	GList *clist, *ch;
	GtkTreeModel *model;

	epg = gmyth_epg_new ();
	if (!gmyth_epg_connect (epg, b_info)) {
		g_object_unref (epg);
		return;
	}

	length = gmyth_epg_get_channel_list (epg, &clist);
	gmyth_epg_disconnect (epg);
	g_object_unref (epg);

	model = g_object_get_data (G_OBJECT (tm->sidebar_livetv), "model");
	clist = g_list_sort (clist, (GCompareFunc) sort_channels);

	for (ch = clist; ch != NULL; ch = ch->next) {
		GMythChannelInfo *info = (GMythChannelInfo *) ch->data;
		GtkTreeIter iter;
		char *uri;
		GdkPixbuf *pixbuf;

		if ((info->channel_name == NULL) || (info->channel_num == NULL))
			continue;

		pixbuf = NULL;
		if (info->channel_icon != NULL && info->channel_icon->str[0] != '\0') {
			pixbuf = get_thumbnail (tm, b_info, info->channel_icon->str);
			g_message ("icon name: %s", info->channel_icon->str);
		}

		uri = g_strdup_printf ("myth://%s:%d/?channel=%s", b_info->hostname, b_info->port, info->channel_num->str);

		gtk_list_store_insert_with_values (GTK_LIST_STORE (model), &iter, G_MAXINT32,
						   URI_COL, uri,
						   THUMBNAIL_COL, pixbuf,
						   NAME_COL, info->channel_name->str,
						   -1);
	}

	gmyth_free_channel_list (clist);
}

static void
idol_mythtv_plugin_class_init (IdolMythtvPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	object_class->finalize = idol_mythtv_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
device_found_cb (GMythUPnP *upnp,
		 GMythBackendInfo *b_info,
		 IdolMythtvPlugin *plugin)
{
	if (!g_list_find (plugin->lst_b_info, b_info)) {
		plugin->lst_b_info = g_list_append (plugin->lst_b_info,
				g_object_ref (b_info));
		idol_mythtv_list_recordings (plugin, b_info);
		idol_mythtv_list_livetv (plugin, b_info);
	}
}

static void
device_lost_cb (GMythUPnP *upnp,
				 GMythBackendInfo *info,
				 IdolMythtvPlugin *plugin)
{
	GList *elem;
	GMythBackendInfo *b_info;

	elem = g_list_find (plugin->lst_b_info, info);
	if (elem && elem->data) {
		b_info = elem->data;
		plugin->lst_b_info = g_list_remove (plugin->lst_b_info,
				b_info);
		g_object_unref (b_info);
	}

}

static void
idol_mythtv_update_binfo (IdolMythtvPlugin *plugin)
{
	GList *lst;
	GList *w;

	/* Clear old b_info */
	if (plugin->lst_b_info != NULL) {
		g_list_foreach (plugin->lst_b_info, (GFunc) g_object_unref, NULL);
		g_list_free (plugin->lst_b_info);
		plugin->lst_b_info = NULL;
	}


	/* Using GMythUPnP to search for avaliable servers */
	if (plugin->upnp == NULL) {
		plugin->upnp = gmyth_upnp_get_instance ();
		g_signal_connect (G_OBJECT (plugin->upnp),
						  "device-found",
						  G_CALLBACK (device_found_cb),
						  plugin);
		plugin->upnp = gmyth_upnp_get_instance ();
		g_signal_connect (G_OBJECT (plugin->upnp),
						  "device-lost",
						  G_CALLBACK (device_lost_cb),
						  plugin);
	}

	/* Load current servers */
	lst = gmyth_upnp_get_devices (plugin->upnp);
	for (w = lst; w != NULL; w = w->next) {
		GMythBackendInfo *b_info;

		b_info = (GMythBackendInfo *) w->data;
		plugin->lst_b_info = g_list_append (plugin->lst_b_info,
				b_info);
		idol_mythtv_list_recordings (plugin, b_info);
		idol_mythtv_list_livetv (plugin, b_info);
	}
	g_list_free (lst);
	gmyth_upnp_search (plugin->upnp);
}

static void
refresh_cb (GtkWidget *button, IdolMythtvPlugin *tm)
{
	GtkTreeModel *model;

	gtk_widget_set_sensitive (button, FALSE);
	idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (tm->sidebar_recordings));
	idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (tm->sidebar_livetv));

	model = g_object_get_data (G_OBJECT (tm->sidebar_recordings), "model");
	gtk_list_store_clear (GTK_LIST_STORE (model));
	model = g_object_get_data (G_OBJECT (tm->sidebar_livetv), "model");
	gtk_list_store_clear (GTK_LIST_STORE (model));

	idol_mythtv_update_binfo (tm);

	gdk_window_set_cursor (gtk_widget_get_window (tm->sidebar_recordings), NULL);
	gdk_window_set_cursor (gtk_widget_get_window (tm->sidebar_livetv), NULL);
	gtk_widget_set_sensitive (button, TRUE);
}

static void
idol_mythtv_plugin_init (IdolMythtvPlugin *plugin)
{
	idol_mythtv_update_binfo (plugin);
}

static void
idol_mythtv_plugin_finalize (GObject *object)
{
	IdolMythtvPlugin *tm = IDOL_MYTHTV_PLUGIN(object);

	if (tm->lst_b_info != NULL) {
		g_list_foreach (tm->lst_b_info, (GFunc ) g_object_unref, NULL);
		g_list_free (tm->lst_b_info);
		tm->lst_b_info = NULL;
	}
	if (tm->client != NULL) {
		g_object_unref (tm->client);
		tm->client = NULL;
	}
	if (tm->upnp != NULL) {
		g_object_unref (tm->upnp);
		tm->upnp = NULL;
	}
	if (tm->sidebar_recordings != NULL) {
		gtk_widget_destroy (tm->sidebar_recordings);
		tm->sidebar_recordings = NULL;
	}
	if (tm->sidebar_livetv != NULL) {
		gtk_widget_destroy (tm->sidebar_livetv);
		tm->sidebar_livetv = NULL;
	}

	G_OBJECT_CLASS (idol_mythtv_plugin_parent_class)->finalize (object);
}

static GtkWidget *
add_sidebar (IdolMythtvPlugin *tm, const char *name, const char *label)
{
	GtkWidget *box, *scrolled, *treeview, *button;

	box = gtk_vbox_new (FALSE, 6);
	button = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (refresh_cb), tm);
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);
	treeview = create_treeview (tm);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					       GTK_WIDGET (treeview));
	gtk_container_add (GTK_CONTAINER (box), scrolled);
	gtk_box_pack_end (GTK_BOX (box), button, FALSE, FALSE, 0);
	gtk_widget_show_all (box);
	idol_add_sidebar_page (tm->idol, name, label, box);

	g_object_set_data (G_OBJECT (box), "treeview", treeview);
	g_object_set_data (G_OBJECT (box), "model",
			   gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

	return g_object_ref (box);
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolMythtvPlugin *tm = IDOL_MYTHTV_PLUGIN(plugin);

	tm->idol = g_object_ref (idol);

	tm->sidebar_recordings = add_sidebar (tm, "mythtv-recordings", _("MythTV Recordings"));
	tm->sidebar_livetv = add_sidebar (tm, "mythtv-livetv", _("MythTV LiveTV"));

	idol_mythtv_update_binfo (IDOL_MYTHTV_PLUGIN(plugin));

	/* FIXME we should only do that if it will be done in the background */
#if 0
	idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (box);
	idol_mythtv_list_recordings (IDOL_MYTHTV_PLUGIN(plugin));
	idol_mythtv_list_livetv (IDOL_MYTHTV_PLUGIN(plugin));
	gdk_window_set_cursor (gtk_widget_get_window (box), NULL);
#endif
	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolMythtvPlugin *tm = IDOL_MYTHTV_PLUGIN(plugin);

	idol_remove_sidebar_page (idol, MYTHTV_SIDEBAR_RECORDINGS);
	idol_remove_sidebar_page (idol, MYTHTV_SIDEBAR_LIVETV);

	if (tm->lst_b_info != NULL) {
		g_list_foreach (tm->lst_b_info, (GFunc ) g_object_unref, NULL);
		g_list_free (tm->lst_b_info);
		tm->lst_b_info = NULL;
	}
	if (tm->client != NULL) {
		g_object_unref (tm->client);
		tm->client = NULL;
	}
	if (tm->upnp != NULL) {
		g_object_unref (tm->upnp);
		tm->upnp = NULL;
	}
	if (tm->sidebar_recordings != NULL) {
		gtk_widget_destroy (tm->sidebar_recordings);
		tm->sidebar_recordings = NULL;
	}
	if (tm->sidebar_livetv != NULL) {
		gtk_widget_destroy (tm->sidebar_livetv);
		tm->sidebar_livetv = NULL;
	}
	g_object_unref (idol);
}
