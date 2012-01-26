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

#include "idol-plugin.h"
#include "idol.h"
#include "backend/bacon-video-widget.h"

#define IDOL_TYPE_ONTOP_PLUGIN		(idol_ontop_plugin_get_type ())
#define IDOL_ONTOP_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_ONTOP_PLUGIN, IdolOntopPlugin))
#define IDOL_ONTOP_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_ONTOP_PLUGIN, IdolOntopPluginClass))
#define IDOL_IS_ONTOP_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_ONTOP_PLUGIN))
#define IDOL_IS_ONTOP_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_ONTOP_PLUGIN))
#define IDOL_ONTOP_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_ONTOP_PLUGIN, IdolOntopPluginClass))

typedef struct
{
	guint handler_id;
	guint handler_id_metadata;
	GtkWindow *window;
	BaconVideoWidget *bvw;
	IdolObject *idol;
} IdolOntopPluginPrivate;

typedef struct
{
	IdolPlugin parent;
	IdolOntopPluginPrivate *priv;
} IdolOntopPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolOntopPluginClass;

G_MODULE_EXPORT GType register_idol_plugin	(GTypeModule *module);
GType idol_ontop_plugin_get_type		(void) G_GNUC_CONST;

static gboolean impl_activate			(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate			(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER (IdolOntopPlugin, idol_ontop_plugin)

static void
idol_ontop_plugin_class_init (IdolOntopPluginClass *klass)
{
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IdolOntopPluginPrivate));

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_ontop_plugin_init (IdolOntopPlugin *plugin)
{
	plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, IDOL_TYPE_ONTOP_PLUGIN, IdolOntopPluginPrivate);
}

static void
update_from_state (IdolOntopPluginPrivate *priv)
{
	GValue has_video = { 0, };

	bacon_video_widget_get_metadata (priv->bvw, BVW_INFO_HAS_VIDEO, &has_video);

	gtk_window_set_keep_above (priv->window,
				   (idol_is_playing (priv->idol) != FALSE &&
				    g_value_get_boolean (&has_video) != FALSE));
	g_value_unset (&has_video);
}

static void
got_metadata_cb (BaconVideoWidget *bvw, IdolOntopPlugin *pi)
{
	update_from_state (pi->priv);
}

static void
property_notify_cb (IdolObject *idol,
		    GParamSpec *spec,
		    IdolOntopPlugin *pi)
{
	update_from_state (pi->priv);
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolOntopPlugin *pi = IDOL_ONTOP_PLUGIN (plugin);

	pi->priv->window = idol_get_main_window (idol);
	pi->priv->bvw = BACON_VIDEO_WIDGET (idol_get_video_widget (idol));
	pi->priv->idol = idol;

	pi->priv->handler_id = g_signal_connect (G_OBJECT (idol),
					   "notify::playing",
					   G_CALLBACK (property_notify_cb),
					   pi);
	pi->priv->handler_id_metadata = g_signal_connect (G_OBJECT (pi->priv->bvw),
						    "got-metadata",
						    G_CALLBACK (got_metadata_cb),
						    pi);

	update_from_state (pi->priv);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolOntopPlugin *pi = IDOL_ONTOP_PLUGIN (plugin);

	g_signal_handler_disconnect (G_OBJECT (idol), pi->priv->handler_id);
	g_signal_handler_disconnect (G_OBJECT (pi->priv->bvw), pi->priv->handler_id_metadata);

	g_object_unref (pi->priv->bvw);

	/* We can't really "restore" the previous state, as there's
	 * no way to find the old state */
	gtk_window_set_keep_above (pi->priv->window, FALSE);
	g_object_unref (pi->priv->window);
}
