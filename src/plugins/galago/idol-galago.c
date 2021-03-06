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
#include <libgalago/galago.h>

#include "idol-plugin.h"
#include "idol.h"

#define IDOL_TYPE_GALAGO_PLUGIN		(idol_galago_plugin_get_type ())
#define IDOL_GALAGO_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_GALAGO_PLUGIN, IdolGalagoPlugin))
#define IDOL_GALAGO_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_GALAGO_PLUGIN, IdolGalagoPluginClass))
#define IDOL_IS_GALAGO_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_GALAGO_PLUGIN))
#define IDOL_IS_GALAGO_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_GALAGO_PLUGIN))
#define IDOL_GALAGO_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_GALAGO_PLUGIN, IdolGalagoPluginClass))

typedef struct
{
	IdolPlugin	parent;

	guint		handler_id_fullscreen;
	guint		handler_id_playing;
	gboolean	idle; /* Whether we're idle */
	GalagoPerson	*me; /* Me! */
} IdolGalagoPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolGalagoPluginClass;


G_MODULE_EXPORT GType register_idol_plugin	(GTypeModule *module);
GType idol_galago_plugin_get_type		(void) G_GNUC_CONST;

static void idol_galago_plugin_init		(IdolGalagoPlugin *plugin);
static void idol_galago_plugin_dispose		(GObject *object);
static void idol_galago_plugin_finalize	(GObject *object);
static gboolean impl_activate			(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate			(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER(IdolGalagoPlugin, idol_galago_plugin)

static void
idol_galago_plugin_class_init (IdolGalagoPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	object_class->dispose = idol_galago_plugin_dispose;
	object_class->finalize = idol_galago_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_galago_plugin_init (IdolGalagoPlugin *plugin)
{
	if (galago_init (PACKAGE_NAME, GALAGO_INIT_FEED) == FALSE
	    || galago_is_connected () == FALSE) {
		g_warning ("Failed to initialise libgalago.");
		return;
	}

	/* Get "me" and list accounts */
	plugin->me = galago_get_me (GALAGO_REMOTE, TRUE);
}

static void
idol_galago_plugin_dispose (GObject *object)
{
	IdolGalagoPlugin *plugin = IDOL_GALAGO_PLUGIN (object);

	if (plugin->me != NULL) {
		g_object_unref (plugin->me);
		plugin->me = NULL;
	}

	G_OBJECT_CLASS (idol_galago_plugin_parent_class)->dispose (object);
}

static void
idol_galago_plugin_finalize (GObject *object)
{
	if (galago_is_connected ())
		galago_uninit ();

	G_OBJECT_CLASS (idol_galago_plugin_parent_class)->finalize (object);
}

static void
idol_galago_set_idleness (IdolGalagoPlugin *plugin, gboolean idle)
{
	GList *account;
	GalagoPresence *presence;

	if (galago_is_connected () == FALSE)
		return;

	if (plugin->idle == idle)
		return;

	plugin->idle = idle;
	for (account = galago_person_get_accounts (plugin->me, TRUE); account != NULL; account = g_list_next (account)) {
		presence = galago_account_get_presence ((GalagoAccount *)account->data, TRUE);
		if (presence != NULL)
			galago_presence_set_idle (presence, idle, time (NULL));
	}
}

static void
idol_galago_update_from_state (IdolObject *idol,
				IdolGalagoPlugin *plugin)
{
	if (idol_is_playing (idol) != FALSE
	    && idol_is_fullscreen (idol) != FALSE) {
		idol_galago_set_idleness (plugin, TRUE);
	} else {
		idol_galago_set_idleness (plugin, FALSE);
	}
}

static void
property_notify_cb (IdolObject *idol,
		    GParamSpec *spec,
		    IdolGalagoPlugin *plugin)
{
	idol_galago_update_from_state (idol, plugin);
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	IdolGalagoPlugin *pi = IDOL_GALAGO_PLUGIN (plugin);

	if (!galago_is_connected ()) {
		g_set_error_literal (error, IDOL_PLUGIN_ERROR, IDOL_PLUGIN_ERROR_ACTIVATION,
                                     _("Could not connect to the Galago daemon."));
		return FALSE;
	}

	pi->handler_id_fullscreen = g_signal_connect (G_OBJECT (idol),
				"notify::fullscreen",
				G_CALLBACK (property_notify_cb),
				pi);
	pi->handler_id_playing = g_signal_connect (G_OBJECT (idol),
				"notify::playing",
				G_CALLBACK (property_notify_cb),
				pi);

	/* Force setting the current status */
	idol_galago_update_from_state (idol, pi);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	IdolGalagoPlugin *pi = IDOL_GALAGO_PLUGIN (plugin);

	g_signal_handler_disconnect (G_OBJECT (idol), pi->handler_id_fullscreen);
	g_signal_handler_disconnect (G_OBJECT (idol), pi->handler_id_playing);

	idol_galago_set_idleness (pi, FALSE);
}
