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

#include "idol-plugin.h"
#include "idol.h"
#include "idol-scrsaver.h"
#include "backend/bacon-video-widget.h"

#define IDOL_TYPE_SCREENSAVER_PLUGIN		(idol_screensaver_plugin_get_type ())
#define IDOL_SCREENSAVER_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_SCREENSAVER_PLUGIN, IdolScreensaverPlugin))
#define IDOL_SCREENSAVER_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_SCREENSAVER_PLUGIN, IdolScreensaverPluginClass))
#define IDOL_IS_SCREENSAVER_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_SCREENSAVER_PLUGIN))
#define IDOL_IS_SCREENSAVER_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_SCREENSAVER_PLUGIN))
#define IDOL_SCREENSAVER_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_SCREENSAVER_PLUGIN, IdolScreensaverPluginClass))

typedef struct
{
	IdolPlugin   parent;
	IdolObject  *idol;
	BaconVideoWidget *bvw;

	IdolScrsaver *scr;
	guint          handler_id_playing;
	guint          handler_id_metadata;
	guint          handler_id_mateconf;
} IdolScreensaverPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolScreensaverPluginClass;


G_MODULE_EXPORT GType register_idol_plugin		(GTypeModule *module);
GType	idol_screensaver_plugin_get_type		(void) G_GNUC_CONST;

static void idol_screensaver_plugin_finalize		(GObject *object);
static gboolean impl_activate				(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate				(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER(IdolScreensaverPlugin, idol_screensaver_plugin)

static void
idol_screensaver_plugin_class_init (IdolScreensaverPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	object_class->finalize = idol_screensaver_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_screensaver_plugin_init (IdolScreensaverPlugin *plugin)
{
	plugin->scr = idol_scrsaver_new ();
	g_object_set (plugin->scr,
		      "reason", _("Playing a movie"),
		      NULL);
}

static void
idol_screensaver_plugin_finalize (GObject *object)
{
	IdolScreensaverPlugin *plugin = IDOL_SCREENSAVER_PLUGIN (object);

	g_object_unref (plugin->scr);

	G_OBJECT_CLASS (idol_screensaver_plugin_parent_class)->finalize (object);
}

static void
idol_screensaver_update_from_state (IdolObject *idol,
				     IdolScreensaverPlugin *pi)
{
	gboolean lock_screensaver_on_audio, can_get_frames;
	BaconVideoWidget *bvw;
	MateConfClient *gc;

	bvw = BACON_VIDEO_WIDGET (idol_get_video_widget ((Idol *)(idol)));
	gc = mateconf_client_get_default ();

	lock_screensaver_on_audio = mateconf_client_get_bool (gc, 
							   MATECONF_PREFIX"/lock_screensaver_on_audio",
							   NULL);
	can_get_frames = bacon_video_widget_can_get_frames (bvw, NULL);

	if (idol_is_playing (idol) != FALSE && can_get_frames)
		idol_scrsaver_disable (pi->scr);
	else if (idol_is_playing (idol) != FALSE && !lock_screensaver_on_audio)
		idol_scrsaver_disable (pi->scr);
	else
		idol_scrsaver_enable (pi->scr);

	g_object_unref (gc);
}

static void
property_notify_cb (IdolObject *idol,
		    GParamSpec *spec,
		    IdolScreensaverPlugin *pi)
{
	idol_screensaver_update_from_state (idol, pi);
}

static void
got_metadata_cb (BaconVideoWidget *bvw, IdolScreensaverPlugin *pi)
{
	idol_screensaver_update_from_state (pi->idol, pi);
}

static void
lock_screensaver_on_audio_changed_cb (MateConfClient *client, guint cnxn_id,
				      MateConfEntry *entry, IdolScreensaverPlugin *pi)
{
	idol_screensaver_update_from_state (pi->idol, pi);
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolScreensaverPlugin *pi = IDOL_SCREENSAVER_PLUGIN (plugin);
	MateConfClient *gc;

	pi->bvw = BACON_VIDEO_WIDGET (idol_get_video_widget (idol));

	gc = mateconf_client_get_default ();
	mateconf_client_add_dir (gc, MATECONF_PREFIX,
			      MATECONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	pi->handler_id_mateconf = mateconf_client_notify_add (gc, MATECONF_PREFIX"/lock_screensaver_on_audio",
							(MateConfClientNotifyFunc) lock_screensaver_on_audio_changed_cb,
							plugin, NULL, NULL);
	g_object_unref (gc);

	pi->handler_id_playing = g_signal_connect (G_OBJECT (idol),
						   "notify::playing",
						   G_CALLBACK (property_notify_cb),
						   pi);
	pi->handler_id_metadata = g_signal_connect (G_OBJECT (pi->bvw),
						    "got-metadata",
						    G_CALLBACK (got_metadata_cb),
						    pi);

	pi->idol = g_object_ref (idol);

	/* Force setting the current status */
	idol_screensaver_update_from_state (idol, pi);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolScreensaverPlugin *pi = IDOL_SCREENSAVER_PLUGIN (plugin);
	MateConfClient *gc;

	gc = mateconf_client_get_default ();
	mateconf_client_notify_remove (gc, pi->handler_id_mateconf);
	g_object_unref (gc);

	if (pi->handler_id_playing != 0) {
		g_signal_handler_disconnect (G_OBJECT (idol), pi->handler_id_playing);
		pi->handler_id_playing = 0;
	}
	if (pi->handler_id_metadata != 0) {
		g_signal_handler_disconnect (G_OBJECT (pi->bvw), pi->handler_id_metadata);
		pi->handler_id_metadata = 0;
	}

	g_object_unref (pi->idol);
	g_object_unref (pi->bvw);

	idol_scrsaver_enable (pi->scr);
}
