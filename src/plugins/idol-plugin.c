/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * heavily based on code from Rhythmbox and Pluma
 *
 * Copyright (C) 2002-2005 Paolo Maggi
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301  USA.
 *
 * Sunday 13th May 2007: Bastien Nocera: Add exception clause.
 * See license_change file for details.
 *
 */

/**
 * SECTION:idol-plugin
 * @short_description: base plugin class and loading/unloading functions
 * @stability: Unstable
 * @include: idol-plugin.h
 *
 * #IdolPlugin is a general-purpose architecture for adding plugins to Idol, with
 * derived support for different programming languages.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <mateconf/mateconf-client.h>

#include "idol-plugin.h"
#include "idol-uri.h"
#include "idol-interface.h"

G_DEFINE_TYPE (IdolPlugin, idol_plugin, G_TYPE_OBJECT)

#define IDOL_PLUGIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), IDOL_TYPE_PLUGIN, IdolPluginPrivate))

static void idol_plugin_finalise (GObject *o);
static void idol_plugin_set_property (GObject *object,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *pspec);
static void idol_plugin_get_property (GObject *object,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *pspec);

struct IdolPluginPrivate {
	char *name;
};

enum
{
	PROP_0,
	PROP_NAME
};

GQuark
idol_plugin_error_quark (void)
{
	static GQuark quark;

	if (!quark)
		quark = g_quark_from_static_string ("idol_plugin_error");

	return quark;
}

static gboolean
is_configurable (IdolPlugin *plugin)
{
	return (IDOL_PLUGIN_GET_CLASS (plugin)->create_configure_dialog != NULL);
}

static void
idol_plugin_class_init (IdolPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = idol_plugin_finalise;
	object_class->get_property = idol_plugin_get_property;
	object_class->set_property = idol_plugin_set_property;

	klass->activate = NULL;
	klass->deactivate = NULL;
	klass->create_configure_dialog = NULL;
	klass->is_configurable = is_configurable;

	/* FIXME: this should be a construction property, but due to the python plugin hack can't be */
	/**
	 * IdolPlugin:name:
	 *
	 * The plugin's name. It should be a construction property, but due to the Python plugin hack, it
	 * can't be: do not change the name after construction. Should be the same as used for naming plugin-
	 * specific resources.
	 **/
	g_object_class_install_property (object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
							      "name",
							      "name",
							      NULL,
							      G_PARAM_READWRITE /*| G_PARAM_CONSTRUCT_ONLY*/));

	g_type_class_add_private (klass, sizeof (IdolPluginPrivate));
}

static void
idol_plugin_init (IdolPlugin *plugin)
{
	/* Empty */
}

static void
idol_plugin_finalise (GObject *object)
{
	IdolPluginPrivate *priv = IDOL_PLUGIN_GET_PRIVATE (object);

	g_free (priv->name);

	G_OBJECT_CLASS (idol_plugin_parent_class)->finalize (object);
}

static void
idol_plugin_set_property (GObject *object,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	IdolPluginPrivate *priv = IDOL_PLUGIN_GET_PRIVATE (object);

	switch (prop_id)
	{
	case PROP_NAME:
		g_free (priv->name);
		priv->name = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
idol_plugin_get_property (GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
	IdolPluginPrivate *priv = IDOL_PLUGIN_GET_PRIVATE (object);

	switch (prop_id)
	{
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * idol_plugin_activate:
 * @plugin: a #IdolPlugin
 * @idol: a #IdolObject
 * @error: return location for a #GError, or %NULL
 *
 * Activates the passed @plugin by calling its activate method.
 *
 * Return value: %TRUE on success
 **/
gboolean
idol_plugin_activate (IdolPlugin *plugin,
		       IdolObject *idol,
		       GError **error)
{
	g_return_val_if_fail (IDOL_IS_PLUGIN (plugin), FALSE);
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), FALSE);

	if (IDOL_PLUGIN_GET_CLASS (plugin)->activate)
		return IDOL_PLUGIN_GET_CLASS (plugin)->activate (plugin, idol, error);

	return TRUE;
}

/**
 * idol_plugin_deactivate:
 * @plugin: a #IdolPlugin
 * @idol: a #IdolObject
 *
 * Deactivates @plugin by calling its deactivate method.
 **/
void
idol_plugin_deactivate	(IdolPlugin *plugin,
			 IdolObject *idol)
{
	g_return_if_fail (IDOL_IS_PLUGIN (plugin));
	g_return_if_fail (IDOL_IS_OBJECT (idol));

	if (IDOL_PLUGIN_GET_CLASS (plugin)->deactivate)
		IDOL_PLUGIN_GET_CLASS (plugin)->deactivate (plugin, idol);
}

/**
 * idol_plugin_is_configurable:
 * @plugin: a #IdolPlugin
 *
 * Returns %TRUE if the plugin is configurable and has a
 * configuration dialog. It calls the plugin's
 * is_configurable method.
 *
 * Return value: %TRUE if the plugin is configurable
 **/
gboolean
idol_plugin_is_configurable (IdolPlugin *plugin)
{
	g_return_val_if_fail (IDOL_IS_PLUGIN (plugin), FALSE);

	return IDOL_PLUGIN_GET_CLASS (plugin)->is_configurable (plugin);
}

/**
 * idol_plugin_create_configure_dialog:
 * @plugin: a #IdolPlugin
 *
 * Returns the plugin's configuration dialog, as created by
 * the plugin's create_configure_dialog method.
 *
 * Return value: the configuration dialog, or %NULL
 **/
GtkWidget *
idol_plugin_create_configure_dialog (IdolPlugin *plugin)
{
	g_return_val_if_fail (IDOL_IS_PLUGIN (plugin), NULL);

	if (IDOL_PLUGIN_GET_CLASS (plugin)->create_configure_dialog)
		return IDOL_PLUGIN_GET_CLASS (plugin)->create_configure_dialog (plugin);
	else
		return NULL;
}

#define UNINSTALLED_PLUGINS_LOCATION "plugins"

GList *
idol_get_plugin_paths (void)
{
	GList *paths;
	char  *path;
	MateConfClient *client;

	paths = NULL;

	client = mateconf_client_get_default ();
	if (mateconf_client_get_bool (client, MATECONF_PREFIX"/disable_user_plugins", NULL) == FALSE) {
		path = g_build_filename (idol_data_dot_dir (), "plugins", NULL);
		paths = g_list_prepend (paths, path);
	}

#ifdef IDOL_RUN_IN_SOURCE_TREE
	path = g_build_filename (UNINSTALLED_PLUGINS_LOCATION, NULL);
	paths = g_list_prepend (paths, path);
#endif

	path = g_strdup (IDOL_PLUGIN_DIR);
	paths = g_list_prepend (paths, path);

	paths = g_list_reverse (paths);

	return paths;
}

/**
 * idol_plugin_find_file:
 * @plugin: a #IdolPlugin
 * @file: the file to find
 *
 * Finds the specified @file by looking in the plugin paths
 * listed by idol_get_plugin_paths() and then in the system
 * Idol data directory.
 *
 * This should be used by plugins to find plugin-specific
 * resource files.
 *
 * Return value: a newly-allocated absolute path for the file, or %NULL
 **/
char *
idol_plugin_find_file (IdolPlugin *plugin,
			const char *file)
{
	IdolPluginPrivate *priv = IDOL_PLUGIN_GET_PRIVATE (plugin);
	GList *paths;
	GList *l;
	char *ret = NULL;

	paths = idol_get_plugin_paths ();

	for (l = paths; l != NULL; l = l->next) {
		if (ret == NULL && priv->name) {
			char *tmp;

			tmp = g_build_filename (l->data, priv->name, file, NULL);

			if (g_file_test (tmp, G_FILE_TEST_EXISTS)) {
				ret = tmp;
				break;
			}
			g_free (tmp);
		}
	}

	g_list_foreach (paths, (GFunc)g_free, NULL);
	g_list_free (paths);

	/* global data files */
	if (ret == NULL)
		ret = idol_interface_get_full_path (file);

	/* ensure it's an absolute path, so doesn't confuse rb_glade_new et al */
	if (ret && ret[0] != '/') {
		char *pwd = g_get_current_dir ();
		char *path = g_strconcat (pwd, G_DIR_SEPARATOR_S, ret, NULL);
		g_free (ret);
		g_free (pwd);
		ret = path;
	}
	return ret;
}

/**
 * idol_plugin_load_interface:
 * @plugin: a #IdolPlugin
 * @name: interface filename
 * @fatal: %TRUE if it's a fatal error if the interface can't be loaded
 * @parent: the interface's parent #GtkWindow
 * @user_data: a pointer to be passed to each signal handler in the interface when they're called
 *
 * Loads an interface file (GtkBuilder UI file) for a plugin, given its filename and
 * assuming it's installed in the plugin's data directory.
 *
 * This should be used instead of attempting to load interfaces manually in plugins.
 *
 * Return value: the #GtkBuilder instance for the interface
 **/
GtkBuilder *
idol_plugin_load_interface (IdolPlugin *plugin, const char *name,
			     gboolean fatal, GtkWindow *parent,
			     gpointer user_data)
{
	GtkBuilder *builder = NULL;
	char *filename;

	filename = idol_plugin_find_file (plugin, name);
	builder = idol_interface_load_with_full_path (filename, fatal, parent,
						       user_data);
	g_free (filename);

	return builder;
}
