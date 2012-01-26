/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Philip Withnall <philip@tecnocode.co.uk>
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
 * Monday 7th February 2005: Christian Schaller: Add excemption clause.
 * See license_change file for details.
 *
 * Author: Bastien Nocera <hadess@hadess.net>, Philip Withnall <philip@tecnocode.co.uk>
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "idol-skipto-plugin.h"
#include "idol-skipto.h"

struct IdolSkiptoPluginPrivate
{
	IdolSkipto	*st;
	guint		handler_id_stream_length;
	guint		handler_id_seekable;
	guint		handler_id_key_press;
	guint		ui_merge_id;
	GtkActionGroup	*action_group;
};

G_MODULE_EXPORT GType register_idol_plugin		(GTypeModule *module);

static void idol_skipto_plugin_finalize		(GObject *object);
static gboolean impl_activate				(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate				(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER_EXTENDED(IdolSkiptoPlugin, idol_skipto_plugin, IDOL_PLUGIN_REGISTER_TYPE(idol_skipto))

static void
idol_skipto_plugin_class_init (IdolSkiptoPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IdolSkiptoPluginPrivate));

	object_class->finalize = idol_skipto_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_skipto_plugin_init (IdolSkiptoPlugin *plugin)
{
	plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
						    IDOL_TYPE_SKIPTO_PLUGIN,
						    IdolSkiptoPluginPrivate);
	plugin->priv->st = NULL;
}

static void
destroy_dialog (IdolSkiptoPlugin *plugin)
{
	IdolSkiptoPluginPrivate *priv = plugin->priv;

	if (priv->st != NULL) {
		g_object_remove_weak_pointer (G_OBJECT (priv->st),
					      (gpointer *)&(priv->st));
		gtk_widget_destroy (GTK_WIDGET (priv->st));
		priv->st = NULL;
	}
}

static void
idol_skipto_plugin_finalize (GObject *object)
{
	IdolSkiptoPlugin *plugin = IDOL_SKIPTO_PLUGIN (object);

	destroy_dialog (plugin);

	G_OBJECT_CLASS (idol_skipto_plugin_parent_class)->finalize (object);
}

static void
idol_skipto_update_from_state (IdolObject *idol,
				IdolSkiptoPlugin *plugin)
{
	gint64 _time;
	gboolean seekable;
	GtkAction *action;
	IdolSkiptoPluginPrivate *priv = plugin->priv;

	g_object_get (G_OBJECT (idol),
				"stream-length", &_time,
				"seekable", &seekable,
				NULL);

	if (priv->st != NULL) {
		idol_skipto_update_range (priv->st, _time);
		idol_skipto_set_seekable (priv->st, seekable);
	}

	/* Update the action's sensitivity */
	action = gtk_action_group_get_action (priv->action_group, "skip-to");
	gtk_action_set_sensitive (action, seekable);
}

static void
property_notify_cb (IdolObject *idol,
		    GParamSpec *spec,
		    IdolSkiptoPlugin *plugin)
{
	idol_skipto_update_from_state (idol, plugin);
}

static void
skip_to_response_callback (GtkDialog *dialog, gint response, IdolSkiptoPlugin *plugin)
{
	if (response != GTK_RESPONSE_OK) {
		destroy_dialog (plugin);
		return;
	}

	gtk_widget_hide (GTK_WIDGET (dialog));

	idol_action_seek_time (plugin->idol,
				idol_skipto_get_range (plugin->priv->st),
				TRUE);
	destroy_dialog (plugin);
}

static void
run_skip_to_dialog (IdolSkiptoPlugin *plugin)
{
	IdolSkiptoPluginPrivate *priv = plugin->priv;

	if (idol_is_seekable (plugin->idol) == FALSE)
		return;

	if (priv->st != NULL) {
		gtk_window_present (GTK_WINDOW (priv->st));
		idol_skipto_set_current (priv->st, idol_get_current_time
					  (plugin->idol));
		return;
	}

	priv->st = IDOL_SKIPTO (idol_skipto_new (plugin));
	g_signal_connect (G_OBJECT (priv->st), "delete-event",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	g_signal_connect (G_OBJECT (priv->st), "response",
			  G_CALLBACK (skip_to_response_callback), plugin);
	g_object_add_weak_pointer (G_OBJECT (priv->st),
				   (gpointer *)&(priv->st));
	idol_skipto_update_from_state (plugin->idol, plugin);
	idol_skipto_set_current (priv->st,
				  idol_get_current_time (plugin->idol));
}

static void
skip_to_action_callback (GtkAction *action, IdolSkiptoPlugin *plugin)
{
	run_skip_to_dialog (plugin);
}

static gboolean
on_window_key_press_event (GtkWidget *window, GdkEventKey *event, IdolSkiptoPlugin *plugin)
{

	if (event->state == 0 || !(event->state & GDK_CONTROL_MASK))
		return FALSE;

	switch (event->keyval) {
		case GDK_k:
		case GDK_K:
			run_skip_to_dialog (plugin);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	GtkWindow *window;
	GtkUIManager *manager;
	IdolSkiptoPlugin *pi = IDOL_SKIPTO_PLUGIN (plugin);
	IdolSkiptoPluginPrivate *priv = pi->priv;

	char *builder_path;
	const GtkActionEntry menu_entries[] = {
		{ "skip-to", GTK_STOCK_JUMP_TO, N_("_Skip to..."), "<Control>K", N_("Skip to a specific time"), G_CALLBACK (skip_to_action_callback) }
	};

	builder_path = idol_plugin_find_file (IDOL_PLUGIN (plugin), "skipto.ui");
	if (builder_path == NULL) {
		g_set_error_literal (error, IDOL_PLUGIN_ERROR, IDOL_PLUGIN_ERROR_ACTIVATION,
                                     _("Could not load the \"Skip to\" dialog interface."));
		return FALSE;
	}
	g_free (builder_path);

	pi->idol = idol;
	priv->handler_id_stream_length = g_signal_connect (G_OBJECT (idol),
				"notify::stream-length",
				G_CALLBACK (property_notify_cb),
				pi);
	priv->handler_id_seekable = g_signal_connect (G_OBJECT (idol),
				"notify::seekable",
				G_CALLBACK (property_notify_cb),
				pi);

	/* Key press handler */
	window = idol_get_main_window (idol);
	priv->handler_id_key_press = g_signal_connect (G_OBJECT(window),
				"key-press-event", 
				G_CALLBACK (on_window_key_press_event),
				pi);
	g_object_unref (window);

	/* Install the menu */
	priv->action_group = gtk_action_group_new ("skip-to_group");
	gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group, menu_entries,
				G_N_ELEMENTS (menu_entries), pi);

	manager = idol_get_ui_manager (idol);

	gtk_ui_manager_insert_action_group (manager, priv->action_group, -1);
	g_object_unref (priv->action_group);

	priv->ui_merge_id = gtk_ui_manager_new_merge_id (manager);
	gtk_ui_manager_add_ui (manager, priv->ui_merge_id,
			       "/ui/tmw-menubar/go/skip-forward", "skip-to",
			       "skip-to", GTK_UI_MANAGER_AUTO, TRUE);

	idol_skipto_update_from_state (idol, pi);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	GtkWindow *window;
	GtkUIManager *manager;
	IdolSkiptoPluginPrivate *priv = IDOL_SKIPTO_PLUGIN (plugin)->priv;

	g_signal_handler_disconnect (G_OBJECT (idol),
				     priv->handler_id_stream_length);
	g_signal_handler_disconnect (G_OBJECT (idol),
				     priv->handler_id_seekable);

	if (priv->handler_id_key_press != 0) {
		window = idol_get_main_window (idol);
		g_signal_handler_disconnect (G_OBJECT(window),
					     priv->handler_id_key_press);
		priv->handler_id_key_press = 0;
		g_object_unref (window);
	}

	/* Remove the menu */
	manager = idol_get_ui_manager (idol);
	gtk_ui_manager_remove_ui (manager, priv->ui_merge_id);
	gtk_ui_manager_remove_action_group (manager, priv->action_group);
}
