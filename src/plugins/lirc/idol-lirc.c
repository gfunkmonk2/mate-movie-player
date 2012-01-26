/* 
 *  Copyright (C) 2002 James Willcox  <jwillcox@mate.org>
 *            (C) 2007 Jan Arne Petersen <jpetersen@jpetersen.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 *
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


#include <config.h>

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <string.h>

#include <unistd.h>
#include <lirc/lirc_client.h>

#include "idol-plugin.h"
#include "idol.h"

#define IDOL_TYPE_LIRC_PLUGIN		(idol_lirc_plugin_get_type ())
#define IDOL_LIRC_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_LIRC_PLUGIN, IdolLircPlugin))
#define IDOL_LIRC_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_LIRC_PLUGIN, IdolLircPluginClass))
#define IDOL_IS_LIRC_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_LIRC_PLUGIN))
#define IDOL_IS_LIRC_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_LIRC_PLUGIN))
#define IDOL_LIRC_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_LIRC_PLUGIN, IdolLircPluginClass))

typedef struct
{
	IdolPlugin   parent;

	GIOChannel *lirc_channel;
	struct lirc_config *lirc_config;

	IdolObject *idol;
} IdolLircPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolLircPluginClass;

/* strings that we recognize as commands from lirc */
#define IDOL_IR_COMMAND_PLAY "play"
#define IDOL_IR_COMMAND_PAUSE "pause"
#define IDOL_IR_COMMAND_STOP "stop"
#define IDOL_IR_COMMAND_NEXT "next"
#define IDOL_IR_COMMAND_PREVIOUS "previous"
#define IDOL_IR_COMMAND_SEEK_FORWARD "seek_forward"
#define IDOL_IR_COMMAND_SEEK_BACKWARD "seek_backward"
#define IDOL_IR_COMMAND_VOLUME_UP "volume_up"
#define IDOL_IR_COMMAND_VOLUME_DOWN "volume_down"
#define IDOL_IR_COMMAND_FULLSCREEN "fullscreen"
#define IDOL_IR_COMMAND_QUIT "quit"
#define IDOL_IR_COMMAND_UP "up"
#define IDOL_IR_COMMAND_DOWN "down"
#define IDOL_IR_COMMAND_LEFT "left"
#define IDOL_IR_COMMAND_RIGHT "right"
#define IDOL_IR_COMMAND_SELECT "select"
#define IDOL_IR_COMMAND_MENU "menu"
#define IDOL_IR_COMMAND_PLAYPAUSE "play_pause"
#define IDOL_IR_COMMAND_ZOOM_UP "zoom_up"
#define IDOL_IR_COMMAND_ZOOM_DOWN "zoom_down"
#define IDOL_IR_COMMAND_EJECT "eject"
#define IDOL_IR_COMMAND_PLAY_DVD "play_dvd"
#define IDOL_IR_COMMAND_MUTE "mute"
#define IDOL_IR_COMMAND_TOGGLE_ASPECT "toggle_aspect"

#define IDOL_IR_SETTING "setting_"
#define IDOL_IR_SETTING_TOGGLE_REPEAT "setting_repeat"
#define IDOL_IR_SETTING_TOGGLE_SHUFFLE "setting_shuffle"

G_MODULE_EXPORT GType register_idol_plugin	(GTypeModule *module);
GType	idol_lirc_plugin_get_type		(void) G_GNUC_CONST;

static void idol_lirc_plugin_init		(IdolLircPlugin *plugin);
static void idol_lirc_plugin_finalize		(GObject *object);
static gboolean impl_activate			(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate			(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER(IdolLircPlugin, idol_lirc_plugin)

static void
idol_lirc_plugin_class_init (IdolLircPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	object_class->finalize = idol_lirc_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_lirc_plugin_init (IdolLircPlugin *plugin)
{
}

static void
idol_lirc_plugin_finalize (GObject *object)
{
	G_OBJECT_CLASS (idol_lirc_plugin_parent_class)->finalize (object);
}

static char *
idol_lirc_get_url (const char *str)
{
	char *s;

	if (str == NULL)
		return NULL;
	s = strchr (str, ':');
	if (s == NULL)
		return NULL;
	return g_strdup (s + 1);
}

static IdolRemoteSetting
idol_lirc_to_setting (const gchar *str, char **url)
{
	if (strcmp (str, IDOL_IR_SETTING_TOGGLE_REPEAT) == 0)
		return IDOL_REMOTE_SETTING_REPEAT;
	else if (strcmp (str, IDOL_IR_SETTING_TOGGLE_SHUFFLE) == 0)
		return IDOL_REMOTE_SETTING_SHUFFLE;
	else
		return -1;
}

static IdolRemoteCommand
idol_lirc_to_command (const gchar *str, char **url)
{
	if (strcmp (str, IDOL_IR_COMMAND_PLAY) == 0)
		return IDOL_REMOTE_COMMAND_PLAY;
	else if (strcmp (str, IDOL_IR_COMMAND_PAUSE) == 0)
		return IDOL_REMOTE_COMMAND_PAUSE;
	else if (strcmp (str, IDOL_IR_COMMAND_PLAYPAUSE) == 0)
		return IDOL_REMOTE_COMMAND_PLAYPAUSE;
	else if (strcmp (str, IDOL_IR_COMMAND_STOP) == 0)
		return IDOL_REMOTE_COMMAND_STOP;
	else if (strcmp (str, IDOL_IR_COMMAND_NEXT) == 0)
		return IDOL_REMOTE_COMMAND_NEXT;
	else if (strcmp (str, IDOL_IR_COMMAND_PREVIOUS) == 0)
		return IDOL_REMOTE_COMMAND_PREVIOUS;
	else if (g_str_has_prefix (str, IDOL_IR_COMMAND_SEEK_FORWARD) != FALSE) {
		*url = idol_lirc_get_url (str);
		return IDOL_REMOTE_COMMAND_SEEK_FORWARD;
	} else if (g_str_has_prefix (str, IDOL_IR_COMMAND_SEEK_BACKWARD) != FALSE) {
		*url = idol_lirc_get_url (str);
		return IDOL_REMOTE_COMMAND_SEEK_BACKWARD;
	} else if (strcmp (str, IDOL_IR_COMMAND_VOLUME_UP) == 0)
		return IDOL_REMOTE_COMMAND_VOLUME_UP;
	else if (strcmp (str, IDOL_IR_COMMAND_VOLUME_DOWN) == 0)
		return IDOL_REMOTE_COMMAND_VOLUME_DOWN;
	else if (strcmp (str, IDOL_IR_COMMAND_FULLSCREEN) == 0)
		return IDOL_REMOTE_COMMAND_FULLSCREEN;
	else if (strcmp (str, IDOL_IR_COMMAND_QUIT) == 0)
		return IDOL_REMOTE_COMMAND_QUIT;
	else if (strcmp (str, IDOL_IR_COMMAND_UP) == 0)
		return IDOL_REMOTE_COMMAND_UP;
	else if (strcmp (str, IDOL_IR_COMMAND_DOWN) == 0)
		return IDOL_REMOTE_COMMAND_DOWN;
	else if (strcmp (str, IDOL_IR_COMMAND_LEFT) == 0)
		return IDOL_REMOTE_COMMAND_LEFT;
	else if (strcmp (str, IDOL_IR_COMMAND_RIGHT) == 0)
		return IDOL_REMOTE_COMMAND_RIGHT;
	else if (strcmp (str, IDOL_IR_COMMAND_SELECT) == 0)
		return IDOL_REMOTE_COMMAND_SELECT;
	else if (strcmp (str, IDOL_IR_COMMAND_MENU) == 0)
		return IDOL_REMOTE_COMMAND_DVD_MENU;
	else if (strcmp (str, IDOL_IR_COMMAND_ZOOM_UP) == 0)
		return IDOL_REMOTE_COMMAND_ZOOM_UP;
	else if (strcmp (str, IDOL_IR_COMMAND_ZOOM_DOWN) == 0)
		return IDOL_REMOTE_COMMAND_ZOOM_DOWN;
	else if (strcmp (str, IDOL_IR_COMMAND_EJECT) == 0)
		return IDOL_REMOTE_COMMAND_EJECT;
	else if (strcmp (str, IDOL_IR_COMMAND_PLAY_DVD) == 0)
		return IDOL_REMOTE_COMMAND_PLAY_DVD;
	else if (strcmp (str, IDOL_IR_COMMAND_MUTE) == 0)
		return IDOL_REMOTE_COMMAND_MUTE;
	else if (strcmp (str, IDOL_IR_COMMAND_TOGGLE_ASPECT) == 0)
		return IDOL_REMOTE_COMMAND_TOGGLE_ASPECT;
	else
		return IDOL_REMOTE_COMMAND_UNKNOWN;
}

static gboolean
idol_lirc_read_code (GIOChannel *source, GIOCondition condition, IdolLircPlugin *pi)
{
	char *code;
	char *str = NULL, *url = NULL;
	int ok;
	IdolRemoteCommand cmd;

	if (condition & (G_IO_ERR | G_IO_HUP)) {
		/* LIRC connection broken. */
		return FALSE;
	}

	/* this _could_ block, but it shouldn't */
	lirc_nextcode (&code);

	if (code == NULL) {
		/* the code was incomplete or something */
		return TRUE;
	}

	do {
		ok = lirc_code2char (pi->lirc_config, code, &str);

		if (ok != 0) {
			/* Couldn't convert lirc code to string. */
			break;
		}

		if (str == NULL) {
			/* there was no command associated with the code */
			break;
		}

		if (g_str_has_prefix (str, IDOL_IR_SETTING) != FALSE) {
			IdolRemoteSetting setting;

			setting = idol_lirc_to_setting (str, &url);
			if (setting >= 0) {
				gboolean value;

				value = idol_action_remote_get_setting (pi->idol, setting);
				idol_action_remote_set_setting (pi->idol, setting, !value);
			}
		} else {
			cmd = idol_lirc_to_command (str, &url);
			idol_action_remote (pi->idol, cmd, url);
		}
		g_free (url);
	} while (TRUE);

	g_free (code);

	return TRUE;
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolLircPlugin *pi = IDOL_LIRC_PLUGIN (plugin);
	char *path;
	int fd;

	pi->idol = g_object_ref (idol);

	fd = lirc_init ("Idol", 0);
	if (fd < 0) {
		g_set_error_literal (error, IDOL_PLUGIN_ERROR, IDOL_PLUGIN_ERROR_ACTIVATION,
                                     _("Couldn't initialize lirc."));
		return FALSE;
	}

	/* Load the default Idol setup */
	path = idol_plugin_find_file (plugin, "idol_lirc_default");
	if (path == NULL || lirc_readconfig (path, &pi->lirc_config, NULL) == -1) {
		g_free (path);
		g_set_error_literal (error, IDOL_PLUGIN_ERROR, IDOL_PLUGIN_ERROR_ACTIVATION,
                                     _("Couldn't read lirc configuration."));
		close (fd);
		return FALSE;
	}
	g_free (path);

	/* Load the user config, doesn't matter if it's not there */
	lirc_readconfig (NULL, &pi->lirc_config, NULL);

	pi->lirc_channel = g_io_channel_unix_new (fd);
	g_io_channel_set_encoding (pi->lirc_channel, NULL, NULL);
	g_io_channel_set_buffered (pi->lirc_channel, FALSE);
	g_io_add_watch (pi->lirc_channel, G_IO_IN | G_IO_ERR | G_IO_HUP,
			(GIOFunc) idol_lirc_read_code, pi);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolLircPlugin *pi = IDOL_LIRC_PLUGIN (plugin);
	GError *error = NULL;

	if (pi->lirc_channel) {
		g_io_channel_shutdown (pi->lirc_channel, FALSE, &error);
		if (error != NULL) {
			g_warning ("Couldn't destroy lirc connection: %s",
				   error->message);
			g_error_free (error);
		}
		pi->lirc_channel = NULL;
	}

	if (pi->lirc_config) {
		lirc_freeconfig (pi->lirc_config);
		pi->lirc_config = NULL;

		lirc_deinit ();
	}

	if (pi->idol) {
		g_object_unref (pi->idol);
		pi->idol = NULL;
	}
}
