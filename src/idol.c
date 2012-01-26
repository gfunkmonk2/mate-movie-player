/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2001-2007 Bastien Nocera <hadess@hadess.net>
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
 *
 */

#include "config.h"

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#ifdef GDK_WINDOWING_X11
/* X11 headers */
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif

#include "idol.h"
#include "idol-private.h"
#include "idol-interface.h"
#include "idol-options.h"
#include "idol-menu.h"
#include "idol-session.h"
#include "idol-uri.h"
#include "idol-preferences.h"
#include "idol-sidebar.h"
#include "video-utils.h"

static void
long_action (void)
{
	while (gtk_events_pending ())
		gtk_main_iteration ();
}

static UniqueResponse
idol_message_received_cb (UniqueApp         *app,
			   int                command,
			   UniqueMessageData *message_data,
			   guint              time_,
			   Idol             *idol)
{
	char *url;

	if (message_data != NULL)
		url = unique_message_data_get_text (message_data);
	else
		url = NULL;

	idol_action_remote (idol, command, url);

	g_free (url);

	return UNIQUE_RESPONSE_OK;
}

static void
about_url_hook (GtkAboutDialog *about,
	        const char *link,
	        gpointer user_data)
{
	GError *error = NULL;

	if (!gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (about)),
	                   link,
	                   gtk_get_current_event_time (),
	                   &error))
	{
	        idol_interface_error (_("Could not open link"),
	                               error->message,
	                               GTK_WINDOW (about));
	        g_error_free (error);
	}
}


static void
about_email_hook (GtkAboutDialog *about,
		  const char *email_address,
		  gpointer user_data)
{
	char *escaped, *uri;

	escaped = g_uri_escape_string (email_address, NULL, FALSE);
	uri = g_strdup_printf ("mailto:%s", escaped);
	g_free (escaped);

	about_url_hook (about, uri, user_data);
	g_free (uri);
}

/* Debug log message handler: discards debug messages unless Idol is run with IDOL_DEBUG=1.
 * If we're building in the source tree, enable debug messages by default. */
static void
debug_handler (const char *log_domain,
               GLogLevelFlags log_level,
               const char *message,
               MateConfClient *gc)
{
	static int debug = -1;

	if (debug < 0)
		debug = mateconf_client_get_bool (gc, MATECONF_PREFIX"/debug", NULL);

	if (debug)
		g_log_default_handler (log_domain, log_level, message, NULL);
}

int
main (int argc, char **argv)
{
	Idol *idol;
	MateConfClient *gc;
	GError *error = NULL;
	GOptionContext *context;
	GOptionGroup *baconoptiongroup;
	char *sidebar_pageid;

	bindtextdomain (GETTEXT_PACKAGE, MATELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

#ifdef GDK_WINDOWING_X11
	if (XInitThreads () == 0)
	{
		gtk_init (&argc, &argv);
		g_set_application_name (_("Idol Movie Player"));
		idol_action_error_and_exit (_("Could not initialize the thread-safe libraries."), _("Verify your system installation. Idol will now exit."), NULL);
	}
#endif

	g_thread_init (NULL);
	g_type_init ();

	/* Handle command line arguments */
	context = g_option_context_new (N_("- Play movies and songs"));
	baconoptiongroup = bacon_video_widget_get_option_group();
	g_option_context_add_main_entries (context, all_options, GETTEXT_PACKAGE);
	g_option_context_set_translation_domain(context, GETTEXT_PACKAGE);
	g_option_context_add_group (context, baconoptiongroup);

	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	idol_session_add_options (context);
	if (g_option_context_parse (context, &argc, &argv, &error) == FALSE) {
		g_print (_("%s\nRun '%s --help' to see a full list of available command line options.\n"),
				error->message, argv[0]);
		g_error_free (error);
	        g_option_context_free (context);
		idol_action_exit (NULL);
	}
	g_option_context_free (context);

	g_set_application_name (_("Idol Movie Player"));
	gtk_window_set_default_icon_name ("idol");
	g_setenv("PULSE_PROP_media.role", "video", TRUE);
	gtk_about_dialog_set_url_hook (about_url_hook, NULL, NULL);
	gtk_about_dialog_set_email_hook (about_email_hook, NULL, NULL);

	gc = mateconf_client_get_default ();
	if (gc == NULL)
	{
		idol_action_error_and_exit (_("Idol could not initialize the configuration engine."), _("Make sure that MATE is properly installed."), NULL);
	}

	/* Debug log handling */
	g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, (GLogFunc) debug_handler, gc);

	/* Build the main Idol object */
	idol = g_object_new (IDOL_TYPE_OBJECT, NULL);
	idol->gc = gc;

	/* IPC stuff */
	if (optionstate.notconnectexistingsession == FALSE) {
		idol->app = unique_app_new ("org.mate.Idol", NULL);
		idol_options_register_remote_commands (idol);
		if (unique_app_is_running (idol->app) != FALSE) {
			idol_options_process_for_server (idol->app, &optionstate);
			gdk_notify_startup_complete ();
			idol_action_exit (idol);
		} else {
			idol_options_process_early (idol, &optionstate);
		}
	} else {
		idol_options_process_early (idol, &optionstate);
	}

	/* Main window */
	idol->xml = idol_interface_load ("idol.ui", TRUE, NULL, idol);
	if (idol->xml == NULL)
		idol_action_exit (NULL);

	idol->win = GTK_WIDGET (gtk_builder_get_object (idol->xml, "idol_main_window"));

	/* Menubar */
	idol_ui_manager_setup (idol);

	/* The sidebar */
	playlist_widget_setup (idol);

	/* The rest of the widgets */
	idol->state = STATE_STOPPED;
	idol->seek = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_seek_hscale"));
	idol->seekadj = gtk_range_get_adjustment (GTK_RANGE (idol->seek));
	idol->volume = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_volume_button"));
	idol->statusbar = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_statusbar"));
	idol->seek_lock = FALSE;
	idol->fs = idol_fullscreen_new (GTK_WINDOW (idol->win));
	gtk_scale_button_set_adjustment (GTK_SCALE_BUTTON (idol->fs->volume),
					 gtk_scale_button_get_adjustment (GTK_SCALE_BUTTON (idol->volume)));
	gtk_range_set_adjustment (GTK_RANGE (idol->fs->seek), idol->seekadj);

	idol_session_setup (idol, argv);
	idol_setup_file_monitoring (idol);
	idol_setup_file_filters ();
	idol_setup_play_disc (idol);
	idol_callback_connect (idol);
	sidebar_pageid = idol_setup_window (idol);

	/* Show ! gtk_main_iteration trickery to show all the widgets
	 * we have so far */
	if (optionstate.fullscreen == FALSE) {
		gtk_widget_show (idol->win);
		idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (idol->win));
		long_action ();
	} else {
		gtk_widget_realize (idol->win);
	}

	idol->controls_visibility = IDOL_CONTROLS_UNDEFINED;

	/* Show ! (again) the video widget this time. */
	video_widget_create (idol);
	gtk_widget_grab_focus (GTK_WIDGET (idol->bvw));
	idol_fullscreen_set_video_widget (idol->fs, idol->bvw);

	if (optionstate.fullscreen != FALSE) {
		gtk_widget_show (idol->win);
		gdk_flush ();
		idol_action_fullscreen (idol, TRUE);
	}

	/* The prefs after the video widget is connected */
	idol_setup_preferences (idol);

	idol_setup_recent (idol);

	/* Command-line handling */
	idol_options_process_late (idol, &optionstate);

	/* Initialise all the plugins, and set the default page, in case
	 * it comes from a plugin */
	idol_object_plugins_init (idol);
	idol_sidebar_set_current_page (idol, sidebar_pageid, FALSE);
	g_free (sidebar_pageid);

	if (idol->session_restored != FALSE) {
		idol_session_restore (idol, optionstate.filenames);
	} else if (optionstate.filenames != NULL && idol_action_open_files (idol, optionstate.filenames)) {
		idol_action_play_pause (idol);
	} else {
		idol_action_set_mrl (idol, NULL, NULL);
	}

	/* Set the logo at the last minute so we won't try to show it before a video */
	bacon_video_widget_set_logo (idol->bvw, "idol");

	if (optionstate.fullscreen == FALSE)
		gdk_window_set_cursor (gtk_widget_get_window (idol->win), NULL);

	if (idol->app != NULL) {
		g_signal_connect (idol->app, "message-received",
				  G_CALLBACK (idol_message_received_cb), idol);
	}

	gtk_main ();

	return 0;
}
