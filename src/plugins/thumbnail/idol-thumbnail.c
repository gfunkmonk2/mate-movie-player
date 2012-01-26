/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Patrick Hulin <patrick.hulin@gmail.com>
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
#include <gmodule.h>
#include <string.h>

#include "idol-plugin.h"
#include "idol.h"

#define IDOL_TYPE_THUMBNAIL_PLUGIN		(idol_thumbnail_plugin_get_type ())
#define IDOL_THUMBNAIL_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_THUMBNAIL_PLUGIN, IdolThumbnailPlugin))
#define IDOL_THUMBNAIL_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_THUMBNAIL_PLUGIN, IdolThumbnailPluginClass))
#define IDOL_IS_THUMBNAIL_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_THUMBNAIL_PLUGIN))
#define IDOL_IS_THUMBNAIL_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_THUMBNAIL_PLUGIN))
#define IDOL_THUMBNAIL_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_THUMBNAIL_PLUGIN, IdolThumbnailPluginClass))

typedef struct
{
	guint file_closed_handler_id;
	guint file_opened_handler_id;
	GtkWindow *window;
	IdolObject *idol;
} IdolThumbnailPluginPrivate;

typedef struct
{
	IdolPlugin parent;
	IdolThumbnailPluginPrivate *priv;
} IdolThumbnailPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolThumbnailPluginClass;

G_MODULE_EXPORT GType register_idol_plugin	(GTypeModule *module);
GType idol_thumbnail_plugin_get_type		(void) G_GNUC_CONST;

static gboolean impl_activate			(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate			(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER (IdolThumbnailPlugin, idol_thumbnail_plugin)

static void
idol_thumbnail_plugin_class_init (IdolThumbnailPluginClass *klass)
{
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IdolThumbnailPluginPrivate));

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_thumbnail_plugin_init (IdolThumbnailPlugin *plugin)
{
	plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
						    IDOL_TYPE_THUMBNAIL_PLUGIN,
						    IdolThumbnailPluginPrivate);
}

static void
set_icon_to_default (IdolObject *idol)
{
	GtkWindow *window = NULL;
	g_return_if_fail (IDOL_IS_OBJECT (idol));

	window = idol_get_main_window (idol);
	gtk_window_set_icon (window, NULL);
	gtk_window_set_icon_name (window, "idol");
}

static void
update_from_state (IdolThumbnailPluginPrivate *priv,
		   IdolObject *idol,
		   const char *mrl)
{
	GdkPixbuf *pixbuf = NULL;
	GtkWindow *window = NULL;
	char *file_basename, *file_name, *uri_md5;
	GError *err = NULL;

	g_return_if_fail (IDOL_IS_OBJECT (idol));
	window = idol_get_main_window (idol);

	if (mrl == NULL) {
		set_icon_to_default (idol);
		return;
	}

	uri_md5 = g_compute_checksum_for_string (G_CHECKSUM_MD5,
						 mrl,
						 strlen (mrl));
	file_basename = g_strdup_printf ("%s.png", uri_md5);
	file_name = g_build_filename (g_get_home_dir (),
				      ".thumbnails",
				      "normal",
				      file_basename,
				      NULL);

	pixbuf = gdk_pixbuf_new_from_file (file_name, &err);
	/* Try loading from the "large" thumbnails if normal fails */
	if (pixbuf == NULL && err != NULL && err->domain == G_FILE_ERROR) {
		g_clear_error (&err);
		g_free (file_name);
		file_name= g_build_filename (g_get_home_dir (),
					     ".thumbnails",
					     "large",
					     file_basename,
					     NULL);

		pixbuf = gdk_pixbuf_new_from_file (file_name, &err);
	}

	g_free (uri_md5);
	g_free (file_basename);
	g_free (file_name);

	if (pixbuf == NULL) {
		if (err != NULL && err->domain != G_FILE_ERROR) {
			g_printerr ("%s\n", err->message);
		}
		set_icon_to_default (idol);
		return;
	}

	gtk_window_set_icon (window, pixbuf);

	g_object_unref (pixbuf);
}

static void
file_opened_cb (IdolObject *idol,
		const char *mrl,
		IdolThumbnailPlugin *pi)
{
	update_from_state (pi->priv, idol, mrl);
}

static void
file_closed_cb (IdolObject *idol,
		 IdolThumbnailPlugin *pi)
{
	update_from_state (pi->priv, idol, NULL);
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolThumbnailPlugin *pi = IDOL_THUMBNAIL_PLUGIN (plugin);
	char *mrl;

	pi->priv->window = idol_get_main_window (idol);
	pi->priv->idol = idol;

	pi->priv->file_opened_handler_id = g_signal_connect (G_OBJECT (idol),
							     "file-opened",
							     G_CALLBACK (file_opened_cb),
							     pi);
	pi->priv->file_closed_handler_id = g_signal_connect (G_OBJECT (idol),
							     "file-closed",
							     G_CALLBACK (file_closed_cb),
							     pi);

	g_object_get (idol, "current-mrl", &mrl, NULL);

	update_from_state (pi->priv, idol, mrl);

	g_free (mrl);

	return TRUE;
}

static void
impl_deactivate (IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolThumbnailPlugin *pi = IDOL_THUMBNAIL_PLUGIN (plugin);

	g_signal_handler_disconnect (idol, pi->priv->file_opened_handler_id);
	g_signal_handler_disconnect (idol, pi->priv->file_closed_handler_id);

	set_icon_to_default (idol);
}
