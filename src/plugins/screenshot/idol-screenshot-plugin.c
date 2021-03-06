/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2008 Philip Withnall <philip@tecnocode.co.uk>
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
 * Monday 7th February 2005: Christian Schaller: Add exception clause.
 * See license_change file for details.
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <mateconf/mateconf-client.h>

#ifdef HAVE_XFREE
#include <X11/XF86keysym.h>
#endif

#include "idol-screenshot-plugin.h"
#include "idol-screenshot.h"
#include "idol-gallery.h"
#include "idol-uri.h"
#include "backend/bacon-video-widget.h"

struct IdolScreenshotPluginPrivate {
	Idol *idol;
	BaconVideoWidget *bvw;

	gulong got_metadata_signal;
	gulong notify_logo_mode_signal;
	gulong key_press_event_signal;

	guint mateconf_id;
	gboolean save_to_disk;

	guint ui_merge_id;
	GtkActionGroup *action_group;
};

static gboolean impl_activate				(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate				(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER (IdolScreenshotPlugin, idol_screenshot_plugin)

static void
idol_screenshot_plugin_class_init (IdolScreenshotPluginClass *klass)
{
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IdolScreenshotPluginPrivate));

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_screenshot_plugin_init (IdolScreenshotPlugin *plugin)
{
	plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, IDOL_TYPE_SCREENSHOT_PLUGIN, IdolScreenshotPluginPrivate);
}

static void
take_screenshot_action_cb (GtkAction *action, IdolScreenshotPlugin *self)
{
	IdolScreenshotPluginPrivate *priv = self->priv;
	GdkPixbuf *pixbuf;
	GtkWidget *dialog;
	GError *err = NULL;

	if (bacon_video_widget_get_logo_mode (priv->bvw) != FALSE)
		return;

	if (bacon_video_widget_can_get_frames (priv->bvw, &err) == FALSE) {
		if (err == NULL)
			return;

		idol_action_error (_("Idol could not get a screenshot of the video."), err->message, priv->idol);
		g_error_free (err);
		return;
	}

	pixbuf = bacon_video_widget_get_current_frame (priv->bvw);
	if (pixbuf == NULL) {
		idol_action_error (_("Idol could not get a screenshot of the video."), _("This is not supposed to happen; please file a bug report."), priv->idol);
		return;
	}

	dialog = idol_screenshot_new (priv->idol, IDOL_PLUGIN (self), pixbuf);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_object_unref (pixbuf);
}

static void
take_gallery_response_cb (GtkDialog *dialog,
			  int response_id,
			  IdolScreenshotPlugin *self)
{
	if (response_id != GTK_RESPONSE_OK)
		gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
take_gallery_action_cb (GtkAction *action, IdolScreenshotPlugin *self)
{
	Idol *idol = self->priv->idol;
	GtkDialog *dialog;

	if (bacon_video_widget_get_logo_mode (self->priv->bvw) != FALSE)
		return;

	dialog = GTK_DIALOG (idol_gallery_new (idol, IDOL_PLUGIN (self)));

	g_signal_connect (dialog, "response",
			  G_CALLBACK (take_gallery_response_cb), self);
	gtk_widget_show (GTK_WIDGET (dialog));
}

static gboolean
window_key_press_event_cb (GtkWidget *window, GdkEventKey *event, IdolScreenshotPlugin *self)
{
	switch (event->keyval) {
#ifdef HAVE_XFREE
	case XF86XK_Save:
		take_screenshot_action_cb (NULL, self);
		break;
#endif /* HAVE_XFREE */
	case GDK_s:
	case GDK_S:
		if (event->state & GDK_CONTROL_MASK)
			take_screenshot_action_cb (NULL, self);
		else
			return FALSE;
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static void
update_state (IdolScreenshotPlugin *self)
{
	IdolScreenshotPluginPrivate *priv = self->priv;
	gboolean sensitive;
	GtkAction *action;

	sensitive = bacon_video_widget_can_get_frames (priv->bvw, NULL) &&
		    (bacon_video_widget_get_logo_mode (priv->bvw) == FALSE) &&
		    priv->save_to_disk;

	action = gtk_action_group_get_action (priv->action_group, "take-screenshot");
	gtk_action_set_sensitive (action, sensitive);
	action = gtk_action_group_get_action (priv->action_group, "take-gallery");
	gtk_action_set_sensitive (action, sensitive);
}

static void
got_metadata_cb (BaconVideoWidget *bvw, IdolScreenshotPlugin *self)
{
	update_state (self);
}

static void
notify_logo_mode_cb (GObject *object, GParamSpec *pspec, IdolScreenshotPlugin *self)
{
	update_state (self);
}

static void
disable_save_to_disk_changed_cb (MateConfClient *client, guint connection_id, MateConfEntry *entry, IdolScreenshotPlugin *self)
{
	self->priv->save_to_disk = !mateconf_client_get_bool (client, "/desktop/mate/lockdown/disable_save_to_disk", NULL);
}

static gboolean
impl_activate (IdolPlugin *plugin, IdolObject *idol, GError **error)
{
	GtkWindow *window;
	GtkUIManager *manager;
	MateConfClient *client;
	IdolScreenshotPlugin *self = IDOL_SCREENSHOT_PLUGIN (plugin);
	IdolScreenshotPluginPrivate *priv = self->priv;
	const GtkActionEntry menu_entries[] = {
		{ "take-screenshot", "camera-photo", N_("Take _Screenshot..."), "<Ctrl>S", N_("Take a screenshot"), G_CALLBACK (take_screenshot_action_cb) },
		{ "take-gallery", NULL, N_("Create Screenshot _Gallery..."), NULL, N_("Create a gallery of screenshots"), G_CALLBACK (take_gallery_action_cb) }
	};

	priv->idol = idol;
	priv->bvw = BACON_VIDEO_WIDGET (idol_get_video_widget (idol));
	priv->got_metadata_signal = g_signal_connect (G_OBJECT (priv->bvw),
						      "got-metadata",
						      G_CALLBACK (got_metadata_cb),
						      self);
	priv->notify_logo_mode_signal = g_signal_connect (G_OBJECT (priv->bvw),
							  "notify::logo-mode",
							  G_CALLBACK (notify_logo_mode_cb),
							  self);

	/* Key press handler */
	window = idol_get_main_window (idol);
	priv->key_press_event_signal = g_signal_connect (G_OBJECT (window),
							 "key-press-event", 
							 G_CALLBACK (window_key_press_event_cb),
							 self);
	g_object_unref (window);

	/* Install the menu */
	priv->action_group = gtk_action_group_new ("screenshot_group");
	gtk_action_group_set_translation_domain (priv->action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->action_group, menu_entries,
				      G_N_ELEMENTS (menu_entries), self);

	manager = idol_get_ui_manager (idol);

	gtk_ui_manager_insert_action_group (manager, priv->action_group, -1);
	g_object_unref (priv->action_group);

	priv->ui_merge_id = gtk_ui_manager_new_merge_id (manager);
	gtk_ui_manager_add_ui (manager, priv->ui_merge_id,
			       "/ui/tmw-menubar/edit/repeat-mode", "take-screenshot",
			       "take-screenshot", GTK_UI_MANAGER_AUTO, TRUE);
	gtk_ui_manager_add_ui (manager, priv->ui_merge_id,
			       "/ui/tmw-menubar/edit/repeat-mode", "take-gallery",
			       "take-gallery", GTK_UI_MANAGER_AUTO, TRUE);
	gtk_ui_manager_add_ui (manager, priv->ui_merge_id,
			       "/ui/tmw-menubar/edit/repeat-mode", NULL,
			       NULL, GTK_UI_MANAGER_SEPARATOR, TRUE);

	/* Set up a MateConf watch for lockdown keys */
	client = mateconf_client_get_default ();
	priv->mateconf_id = mateconf_client_notify_add (client, "/desktop/mate/lockdown/disable_save_to_disk",
						  (MateConfClientNotifyFunc) disable_save_to_disk_changed_cb,
						  self, NULL, NULL);
	disable_save_to_disk_changed_cb (client, priv->mateconf_id, NULL, self);
	g_object_unref (client);

	/* Update the menu entries' states */
	update_state (self);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin, IdolObject *idol)
{
	IdolScreenshotPluginPrivate *priv = IDOL_SCREENSHOT_PLUGIN (plugin)->priv;
	GtkWindow *window;
	GtkUIManager *manager;
	MateConfClient *client;

	/* Disconnect signal handlers */
	g_signal_handler_disconnect (G_OBJECT (priv->bvw), priv->got_metadata_signal);
	g_signal_handler_disconnect (G_OBJECT (priv->bvw), priv->notify_logo_mode_signal);

	window = idol_get_main_window (idol);
	g_signal_handler_disconnect (G_OBJECT (window), priv->key_press_event_signal);
	g_object_unref (window);

	/* Disconnect from MateConf */
	client = mateconf_client_get_default ();
	mateconf_client_notify_remove (client, priv->mateconf_id);
	g_object_unref (client);

	/* Remove the menu */
	manager = idol_get_ui_manager (idol);
	gtk_ui_manager_remove_ui (manager, priv->ui_merge_id);
	gtk_ui_manager_remove_action_group (manager, priv->action_group);

	g_object_unref (priv->bvw);
}

static char *
make_filename_for_dir (const char *directory, const char *format, const char *movie_title)
{
	char *fullpath, *filename;
	guint i = 1;

	filename = g_strdup_printf (_(format), movie_title, i);
	fullpath = g_build_filename (directory, filename, NULL);

	while (g_file_test (fullpath, G_FILE_TEST_EXISTS) != FALSE && i < G_MAXINT) {
		i++;
		g_free (filename);
		g_free (fullpath);

		filename = g_strdup_printf (_(format), movie_title, i);
		fullpath = g_build_filename (directory, filename, NULL);
	}

	g_free (fullpath);

	return filename;
}

gchar *
idol_screenshot_plugin_setup_file_chooser (const char *filename_format, const char *movie_title)
{
	MateConfClient *client;
	char *path, *filename, *full, *uri;
	GFile *file;

	/* Set the default path */
	client = mateconf_client_get_default ();
	path = mateconf_client_get_string (client, "/apps/idol/screenshot_save_path", NULL);
	g_object_unref (client);

	/* Default to the Pictures directory */
	if (path == NULL || path[0] == '\0') {
		g_free (path);
		path = idol_pictures_dir ();
		/* No pictures dir, then it's the home dir */
		if (path == NULL)
			path = g_strdup (g_get_home_dir ());
	}

	filename = make_filename_for_dir (path, filename_format, movie_title);

	/* Build the URI */
	full = g_build_filename (path, filename, NULL);
	g_free (path);
	g_free (filename);

	file = g_file_new_for_path (full);
	uri = g_file_get_uri (file);
	g_free (full);
	g_object_unref (file);

	return uri;
}

void
idol_screenshot_plugin_update_file_chooser (const char *uri)
{
	MateConfClient *client;
	char *dir;
	GFile *file, *parent;

	file = g_file_new_for_uri (uri);
	parent = g_file_get_parent (file);
	g_object_unref (file);

	dir = g_file_get_path (parent);
	g_object_unref (parent);

	client = mateconf_client_get_default ();
	mateconf_client_set_string (client,
				 "/apps/idol/screenshot_save_path",
				 dir, NULL);
	g_free (dir);
}
