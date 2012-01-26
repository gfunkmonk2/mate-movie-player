/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Jan Arne Petersen <jap@mate.org>
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
#include <dbus/dbus-glib.h>
#include <string.h>

#include "idol-marshal.h"

#include "idol-plugin.h"
#include "idol.h"

#define IDOL_TYPE_MEDIA_PLAYER_KEYS_PLUGIN		(idol_media_player_keys_plugin_get_type ())
#define IDOL_MEDIA_PLAYER_KEYS_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_MEDIA_PLAYER_KEYS_PLUGIN, IdolMediaPlayerKeysPlugin))
#define IDOL_MEDIA_PLAYER_KEYS_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_MEDIA_PLAYER_KEYS_PLUGIN, IdolMediaPlayerKeysPluginClass))
#define IDOL_IS_MEDIA_PLAYER_KEYS_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_MEDIA_PLAYER_KEYS_PLUGIN))
#define IDOL_IS_MEDIA_PLAYER_KEYS_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_MEDIA_PLAYER_KEYS_PLUGIN))
#define IDOL_MEDIA_PLAYER_KEYS_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_MEDIA_PLAYER_KEYS_PLUGIN, IdolMediaPlayerKeysPluginClass))

typedef struct
{
	IdolPlugin    parent;

	DBusGProxy    *media_player_keys_proxy;

	guint          handler_id;
} IdolMediaPlayerKeysPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolMediaPlayerKeysPluginClass;


G_MODULE_EXPORT GType register_idol_plugin		(GTypeModule *module);
GType	idol_media_player_keys_plugin_get_type		(void) G_GNUC_CONST;

static void idol_media_player_keys_plugin_finalize		(GObject *object);
static gboolean impl_activate				(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate				(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER(IdolMediaPlayerKeysPlugin, idol_media_player_keys_plugin)

static void
idol_media_player_keys_plugin_class_init (IdolMediaPlayerKeysPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	object_class->finalize = idol_media_player_keys_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_media_player_keys_plugin_init (IdolMediaPlayerKeysPlugin *plugin)
{
}

static void
idol_media_player_keys_plugin_finalize (GObject *object)
{
	G_OBJECT_CLASS (idol_media_player_keys_plugin_parent_class)->finalize (object);
}

static void
on_media_player_key_pressed (DBusGProxy *proxy, const gchar *application, const gchar *key, IdolObject *idol)
{
	if (strcmp ("Idol", application) == 0) {
		if (strcmp ("Play", key) == 0)
			idol_action_play_pause (idol);
		else if (strcmp ("Previous", key) == 0)
			idol_action_previous (idol);
		else if (strcmp ("Next", key) == 0)
			idol_action_next (idol);
		else if (strcmp ("Stop", key) == 0)
			idol_action_pause (idol);
	}
}

static gboolean
on_window_focus_in_event (GtkWidget *window, GdkEventFocus *event, IdolMediaPlayerKeysPlugin *pi)
{
	if (pi->media_player_keys_proxy != NULL) {
		dbus_g_proxy_call (pi->media_player_keys_proxy,
				   "GrabMediaPlayerKeys", NULL,
				   G_TYPE_STRING, "Idol", G_TYPE_UINT, 0, G_TYPE_INVALID,
				   G_TYPE_INVALID);
	}

	return FALSE;
}

static void
proxy_destroy (DBusGProxy *proxy,
		  IdolMediaPlayerKeysPlugin* plugin)
{
	plugin->media_player_keys_proxy = NULL;
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolMediaPlayerKeysPlugin *pi = IDOL_MEDIA_PLAYER_KEYS_PLUGIN (plugin);
	DBusGConnection *connection;
	GError *err = NULL;
	GtkWindow *window;

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &err);
	if (connection == NULL) {
		g_warning ("Error connecting to D-Bus: %s", err->message);
		return FALSE;
	}

	/* Try the mate-settings-daemon version,
	 * then the mate-control-center version of things */
	pi->media_player_keys_proxy = dbus_g_proxy_new_for_name_owner (connection,
								       "org.mate.SettingsDaemon",
								       "/org/mate/SettingsDaemon/MediaKeys",
								       "org.mate.SettingsDaemon.MediaKeys",
								       NULL);
	if (pi->media_player_keys_proxy == NULL) {
		pi->media_player_keys_proxy = dbus_g_proxy_new_for_name_owner (connection,
									       "org.mate.SettingsDaemon",
									       "/org/mate/SettingsDaemon",
									       "org.mate.SettingsDaemon",
									       &err);
	}

	dbus_g_connection_unref (connection);
	if (err != NULL) {
		gboolean daemon_not_running;
		g_warning ("Failed to create dbus proxy for org.mate.SettingsDaemon: %s",
			   err->message);
		daemon_not_running = (err->code == DBUS_GERROR_NAME_HAS_NO_OWNER);
		g_error_free (err);
		/* don't popup error if settings-daemon is not running,
 		 * ie when starting idol not under MATE desktop */
		return daemon_not_running;
	} else {
		g_signal_connect_object (pi->media_player_keys_proxy,
					 "destroy",
					 G_CALLBACK (proxy_destroy),
					 pi, 0);
	}

	dbus_g_proxy_call (pi->media_player_keys_proxy,
			   "GrabMediaPlayerKeys", NULL,
			   G_TYPE_STRING, "Idol", G_TYPE_UINT, 0, G_TYPE_INVALID,
			   G_TYPE_INVALID);

	dbus_g_object_register_marshaller (idol_marshal_VOID__STRING_STRING,
			G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_add_signal (pi->media_player_keys_proxy, "MediaPlayerKeyPressed",
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (pi->media_player_keys_proxy, "MediaPlayerKeyPressed",
			G_CALLBACK (on_media_player_key_pressed), idol, NULL);

	window = idol_get_main_window (idol);
	pi->handler_id = g_signal_connect (G_OBJECT (window), "focus-in-event",
			G_CALLBACK (on_window_focus_in_event), pi);

	g_object_unref (G_OBJECT (window));

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolMediaPlayerKeysPlugin *pi = IDOL_MEDIA_PLAYER_KEYS_PLUGIN (plugin);
	GtkWindow *window;

	if (pi->media_player_keys_proxy != NULL) {
		dbus_g_proxy_call (pi->media_player_keys_proxy,
				   "ReleaseMediaPlayerKeys", NULL,
				   G_TYPE_STRING, "Idol", G_TYPE_INVALID, G_TYPE_INVALID);
		g_object_unref (pi->media_player_keys_proxy);
		pi->media_player_keys_proxy = NULL;
	}

	if (pi->handler_id != 0) {
		window = idol_get_main_window (idol);
		if (window == NULL)
			return;

		g_signal_handler_disconnect (G_OBJECT (window), pi->handler_id);

		g_object_unref (window);
	}
}
