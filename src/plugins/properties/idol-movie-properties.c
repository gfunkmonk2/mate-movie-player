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
#include <gmodule.h>
#include <string.h>
#include <bacon-video-widget-properties.h>

#include "idol-plugin.h"
#include "idol.h"

#define IDOL_TYPE_MOVIE_PROPERTIES_PLUGIN		(idol_movie_properties_plugin_get_type ())
#define IDOL_MOVIE_PROPERTIES_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_MOVIE_PROPERTIES_PLUGIN, IdolMoviePropertiesPlugin))
#define IDOL_MOVIE_PROPERTIES_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_MOVIE_PROPERTIES_PLUGIN, IdolMoviePropertiesPluginClass))
#define IDOL_IS_MOVIE_PROPERTIES_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_MOVIE_PROPERTIES_PLUGIN))
#define IDOL_IS_MOVIE_PROPERTIES_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_MOVIE_PROPERTIES_PLUGIN))
#define IDOL_MOVIE_PROPERTIES_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_MOVIE_PROPERTIES_PLUGIN, IdolMoviePropertiesPluginClass))

typedef struct
{
	IdolPlugin   parent;

	GtkWidget    *props;
	guint         handler_id_stream_length;
} IdolMoviePropertiesPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolMoviePropertiesPluginClass;


G_MODULE_EXPORT GType register_idol_plugin		(GTypeModule *module);
GType	idol_movie_properties_plugin_get_type		(void) G_GNUC_CONST;

static gboolean impl_activate				(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate				(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER(IdolMoviePropertiesPlugin, idol_movie_properties_plugin)

static void
idol_movie_properties_plugin_class_init (IdolMoviePropertiesPluginClass *klass)
{
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_movie_properties_plugin_init (IdolMoviePropertiesPlugin *plugin)
{
}

static void
stream_length_notify_cb (IdolObject *idol,
			 GParamSpec *arg1,
			 IdolMoviePropertiesPlugin *plugin)
{
	gint64 stream_length;

	g_object_get (G_OBJECT (idol),
		      "stream-length", &stream_length,
		      NULL);

	bacon_video_widget_properties_from_time
		(BACON_VIDEO_WIDGET_PROPERTIES (plugin->props),
		 stream_length);
}

static void
idol_movie_properties_plugin_file_opened (IdolObject *idol,
					   const char *mrl,
					   IdolMoviePropertiesPlugin *plugin)
{
	GtkWidget *bvw;

	bvw = idol_get_video_widget (idol);
	bacon_video_widget_properties_update
		(BACON_VIDEO_WIDGET_PROPERTIES (plugin->props), bvw);
	g_object_unref (bvw);
	gtk_widget_set_sensitive (plugin->props, TRUE);
}

static void
idol_movie_properties_plugin_file_closed (IdolObject *idol,
					   IdolMoviePropertiesPlugin *plugin)
{
        /* Reset the properties and wait for the signal*/
        bacon_video_widget_properties_reset
		(BACON_VIDEO_WIDGET_PROPERTIES (plugin->props));
	gtk_widget_set_sensitive (plugin->props, FALSE);
}

static void
idol_movie_properties_plugin_metadata_updated (IdolObject *idol,
						const char *artist, 
						const char *title, 
						const char *album,
						guint track_num,
						IdolMoviePropertiesPlugin *plugin)
{
	GtkWidget *bvw;

	bvw = idol_get_video_widget (idol);
	bacon_video_widget_properties_update
		(BACON_VIDEO_WIDGET_PROPERTIES (plugin->props), bvw);
	g_object_unref (bvw);
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolMoviePropertiesPlugin *pi;

	pi = IDOL_MOVIE_PROPERTIES_PLUGIN (plugin);

	pi->props = bacon_video_widget_properties_new ();
	gtk_widget_show (pi->props);
	idol_add_sidebar_page (idol,
				"properties",
				_("Properties"),
				pi->props);
	gtk_widget_set_sensitive (pi->props, FALSE);

	g_signal_connect (G_OBJECT (idol),
			  "file-opened",
			  G_CALLBACK (idol_movie_properties_plugin_file_opened),
			  plugin);
	g_signal_connect (G_OBJECT (idol),
			  "file-closed",
			  G_CALLBACK (idol_movie_properties_plugin_file_closed),
			  plugin);
	g_signal_connect (G_OBJECT (idol),
			  "metadata-updated",
			  G_CALLBACK (idol_movie_properties_plugin_metadata_updated),
			  plugin);
	pi->handler_id_stream_length = g_signal_connect (G_OBJECT (idol),
							 "notify::stream-length",
							 G_CALLBACK (stream_length_notify_cb),
							 plugin);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolMoviePropertiesPlugin *pi;

	pi = IDOL_MOVIE_PROPERTIES_PLUGIN (plugin);

	g_signal_handler_disconnect (G_OBJECT (idol), pi->handler_id_stream_length);
	g_signal_handlers_disconnect_by_func (G_OBJECT (idol),
					      idol_movie_properties_plugin_metadata_updated,
					      plugin);
	g_signal_handlers_disconnect_by_func (G_OBJECT (idol),
					      idol_movie_properties_plugin_file_opened,
					      plugin);
	g_signal_handlers_disconnect_by_func (G_OBJECT (idol),
					      idol_movie_properties_plugin_file_closed,
					      plugin);
	pi->handler_id_stream_length = 0;
	idol_remove_sidebar_page (idol, "properties");
}
