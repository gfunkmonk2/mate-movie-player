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
 * Monday 7th February 2005: Christian Schaller: Add exception clause.
 * See license_change file for details.
 *
 */

/**
 * SECTION:idol-object
 * @short_description: main Idol object
 * @stability: Unstable
 * @include: idol.h
 *
 * #IdolObject is the core object of Idol; a singleton which controls all Idol's main functions.
 **/

#include "config.h"

#include <glib-object.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <idol-disc.h>
#include <stdlib.h>
#include <math.h>
#include <gio/gio.h>

#include <string.h>

#ifdef GDK_WINDOWING_X11
/* X11 headers */
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif

#include "idol.h"
#include "idolobject-marshal.h"
#include "idol-private.h"
#include "idol-plugins-engine.h"
#include "ev-sidebar.h"
#include "idol-playlist.h"
#include "bacon-video-widget.h"
#include "idol-statusbar.h"
#include "idol-time-label.h"
#include "idol-sidebar.h"
#include "idol-menu.h"
#include "idol-uri.h"
#include "idol-interface.h"
#include "video-utils.h"
#include "idol-dnd-menu.h"
#include "idol-preferences.h"

#define REWIND_OR_PREVIOUS 4000

#define SEEK_FORWARD_SHORT_OFFSET 15
#define SEEK_BACKWARD_SHORT_OFFSET -5

#define SEEK_FORWARD_LONG_OFFSET 10*60
#define SEEK_BACKWARD_LONG_OFFSET -3*60

#define ZOOM_UPPER 2.0
#define ZOOM_RESET 1.0
#define ZOOM_LOWER 0.1
#define ZOOM_DISABLE (ZOOM_LOWER - 1)
#define ZOOM_ENABLE (ZOOM_UPPER + 1)

#define DEFAULT_WINDOW_W 650
#define DEFAULT_WINDOW_H 500

#define VOLUME_EPSILON (1e-10)

/* casts are to shut gcc up */
static const GtkTargetEntry target_table[] = {
	{ (gchar*) "text/uri-list", 0, 0 },
	{ (gchar*) "_NETSCAPE_URL", 0, 1 }
};

static gboolean idol_action_open_files_list (Idol *idol, GSList *list);
static gboolean idol_action_load_media (Idol *idol, IdolDiscMediaType type, const char *device);
static void update_buttons (Idol *idol);
static void update_fill (Idol *idol, gdouble level);
static void update_media_menu_items (Idol *idol);
static void playlist_changed_cb (GtkWidget *playlist, Idol *idol);
static void play_pause_set_label (Idol *idol, IdolStates state);

/* Callback functions for GtkBuilder */
G_MODULE_EXPORT gboolean main_window_destroy_cb (GtkWidget *widget, GdkEvent *event, Idol *idol);
G_MODULE_EXPORT gboolean window_state_event_cb (GtkWidget *window, GdkEventWindowState *event, Idol *idol);
G_MODULE_EXPORT gboolean seek_slider_pressed_cb (GtkWidget *widget, GdkEventButton *event, Idol *idol);
G_MODULE_EXPORT void seek_slider_changed_cb (GtkAdjustment *adj, Idol *idol);
G_MODULE_EXPORT gboolean seek_slider_released_cb (GtkWidget *widget, GdkEventButton *event, Idol *idol);
G_MODULE_EXPORT void volume_button_value_changed_cb (GtkScaleButton *button, gdouble value, Idol *idol);
G_MODULE_EXPORT gboolean window_key_press_event_cb (GtkWidget *win, GdkEventKey *event, Idol *idol);
G_MODULE_EXPORT int window_scroll_event_cb (GtkWidget *win, GdkEventScroll *event, Idol *idol);
G_MODULE_EXPORT void main_pane_size_allocated (GtkWidget *main_pane, GtkAllocation *allocation, Idol *idol);
G_MODULE_EXPORT void fs_exit1_activate_cb (GtkButton *button, Idol *idol);

enum {
	PROP_0,
	PROP_FULLSCREEN,
	PROP_PLAYING,
	PROP_STREAM_LENGTH,
	PROP_SEEKABLE,
	PROP_CURRENT_TIME,
	PROP_CURRENT_MRL
};

enum {
	FILE_OPENED,
	FILE_CLOSED,
	METADATA_UPDATED,
	LAST_SIGNAL
};

static void idol_object_set_property		(GObject *object,
						 guint property_id,
						 const GValue *value,
						 GParamSpec *pspec);
static void idol_object_get_property		(GObject *object,
						 guint property_id,
						 GValue *value,
						 GParamSpec *pspec);
static void idol_object_finalize (GObject *idol);

static int idol_table_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(IdolObject, idol_object, G_TYPE_OBJECT)

static void
idol_object_class_init (IdolObjectClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;

	object_class->set_property = idol_object_set_property;
	object_class->get_property = idol_object_get_property;
	object_class->finalize = idol_object_finalize;

	/**
	 * IdolObject:fullscreen:
	 *
	 * If %TRUE, Idol is in fullscreen mode.
	 **/
	g_object_class_install_property (object_class, PROP_FULLSCREEN,
					 g_param_spec_boolean ("fullscreen", NULL, NULL,
							       FALSE, G_PARAM_READABLE));

	/**
	 * IdolObject:playing:
	 *
	 * If %TRUE, Idol is playing an audio or video file.
	 **/
	g_object_class_install_property (object_class, PROP_PLAYING,
					 g_param_spec_boolean ("playing", NULL, NULL,
							       FALSE, G_PARAM_READABLE));

	/**
	 * IdolObject:stream-length:
	 *
	 * The length of the current stream, in milliseconds.
	 **/
	g_object_class_install_property (object_class, PROP_STREAM_LENGTH,
					 g_param_spec_int64 ("stream-length", NULL, NULL,
							     G_MININT64, G_MAXINT64, 0,
							     G_PARAM_READABLE));

	/**
	 * IdolObject:current-time:
	 *
	 * The player's position (time) in the current stream, in milliseconds.
	 **/
	g_object_class_install_property (object_class, PROP_CURRENT_TIME,
					 g_param_spec_int64 ("current-time", NULL, NULL,
							     G_MININT64, G_MAXINT64, 0,
							     G_PARAM_READABLE));

	/**
	 * IdolObject:seekable:
	 *
	 * If %TRUE, the current stream is seekable.
	 **/
	g_object_class_install_property (object_class, PROP_SEEKABLE,
					 g_param_spec_boolean ("seekable", NULL, NULL,
							       FALSE, G_PARAM_READABLE));

	/**
	 * IdolObject:current-mrl:
	 *
	 * The MRL of the current stream.
	 **/
	g_object_class_install_property (object_class, PROP_CURRENT_MRL,
					 g_param_spec_string ("current-mrl", NULL, NULL,
							      NULL, G_PARAM_READABLE));

	/**
	 * IdolObject::file-opened:
	 * @idol: the #IdolObject which received the signal
	 * @mrl: the MRL of the opened stream
	 *
	 * The #IdolObject::file-opened signal is emitted when a new stream is opened by Idol.
	 */
	idol_table_signals[FILE_OPENED] =
		g_signal_new ("file-opened",
				G_TYPE_FROM_CLASS (object_class),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (IdolObjectClass, file_opened),
				NULL, NULL,
				g_cclosure_marshal_VOID__STRING,
				G_TYPE_NONE, 1, G_TYPE_STRING);

	/**
	 * IdolObject::file-closed:
	 * @idol: the #IdolObject which received the signal
	 *
	 * The #IdolObject::file-closed signal is emitted when Idol closes a stream.
	 */
	idol_table_signals[FILE_CLOSED] =
		g_signal_new ("file-closed",
				G_TYPE_FROM_CLASS (object_class),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (IdolObjectClass, file_closed),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, G_TYPE_NONE);

	/**
	 * IdolObject::metadata-updated:
	 * @idol: the #IdolObject which received the signal
	 * @artist: the name of the artist, or %NULL
	 * @title: the stream title, or %NULL
	 * @album: the name of the stream's album, or %NULL
	 * @track_number: the stream's track number
	 *
	 * The #IdolObject::metadata-updated signal is emitted when the metadata of a stream is updated, typically
	 * when it's being loaded.
	 */
	idol_table_signals[METADATA_UPDATED] =
		g_signal_new ("metadata-updated",
				G_TYPE_FROM_CLASS (object_class),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (IdolObjectClass, metadata_updated),
				NULL, NULL,
				idolobject_marshal_VOID__STRING_STRING_STRING_UINT,
				G_TYPE_NONE, 4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
}

static void
idol_object_init (IdolObject *idol)
{
	//FIXME nothing yet
}

static void
idol_object_finalize (GObject *object)
{

	G_OBJECT_CLASS (idol_object_parent_class)->finalize (object);
}

static void
idol_object_set_property (GObject *object,
			   guint property_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
idol_object_get_property (GObject *object,
			   guint property_id,
			   GValue *value,
			   GParamSpec *pspec)
{
	IdolObject *idol;

	idol = IDOL_OBJECT (object);

	switch (property_id)
	{
	case PROP_FULLSCREEN:
		g_value_set_boolean (value, idol_is_fullscreen (idol));
		break;
	case PROP_PLAYING:
		g_value_set_boolean (value, idol_is_playing (idol));
		break;
	case PROP_STREAM_LENGTH:
		g_value_set_int64 (value, bacon_video_widget_get_stream_length (idol->bvw));
		break;
	case PROP_CURRENT_TIME:
		g_value_set_int64 (value, bacon_video_widget_get_current_time (idol->bvw));
		break;
	case PROP_SEEKABLE:
		g_value_set_boolean (value, idol_is_seekable (idol));
		break;
	case PROP_CURRENT_MRL:
		g_value_set_string (value, idol->mrl);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

/**
 * idol_object_plugins_init:
 * @idol: a #IdolObject
 *
 * Initialises the plugin engine and activates all the
 * enabled plugins.
 **/
void
idol_object_plugins_init (IdolObject *idol)
{
	idol_plugins_engine_init (idol);
}

/**
 * idol_object_plugins_shutdown:
 *
 * Shuts down the plugin engine and deactivates all the
 * plugins.
 **/
void
idol_object_plugins_shutdown (void)
{
	idol_plugins_engine_shutdown ();
}

/**
 * idol_get_main_window:
 * @idol: a #IdolObject
 *
 * Gets Idol's main window and increments its reference count.
 *
 * Return value: Idol's main window
 **/
GtkWindow *
idol_get_main_window (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), NULL);

	g_object_ref (G_OBJECT (idol->win));

	return GTK_WINDOW (idol->win);
}

/**
 * idol_get_ui_manager:
 * @idol: a #IdolObject
 *
 * Gets Idol's UI manager, but does not change its reference count.
 *
 * Return value: Idol's UI manager
 **/
GtkUIManager *
idol_get_ui_manager (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), NULL);

	return idol->ui_manager;
}

/**
 * idol_get_video_widget:
 * @idol: a #IdolObject
 *
 * Gets Idol's video widget and increments its reference count.
 *
 * Return value: Idol's video widget
 **/
GtkWidget *
idol_get_video_widget (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), NULL);

	g_object_ref (G_OBJECT (idol->bvw));

	return GTK_WIDGET (idol->bvw);
}

/**
 * idol_get_video_widget_backend_name:
 * @idol: a #IdolObject
 *
 * Gets the name string of the backend video widget, typically the video library's
 * version string (e.g. what's returned by gst_version_string()). Free with g_free().
 *
 * Return value: a newly-allocated string of the name of the backend video widget
 **/
char *
idol_get_video_widget_backend_name (Idol *idol)
{
	return bacon_video_widget_get_backend_name (idol->bvw);
}

/**
 * idol_get_version:
 *
 * Gets the application name and version (e.g. "Idol 2.28.0").
 *
 * Return value: a newly-allocated string of the name and version of the application
 **/
char *
idol_get_version (void)
{
	/* Translators: %s is the idol version number */
	return g_strdup_printf (_("Idol %s"), VERSION);
}

/**
 * idol_get_current_time:
 * @idol: a #IdolObject
 *
 * Gets the current position's time in the stream as a gint64.
 *
 * Return value: the current position in the stream
 **/
gint64
idol_get_current_time (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), 0);

	return bacon_video_widget_get_current_time (idol->bvw);
}

typedef struct {
	Idol *idol;
	gchar *uri;
	gchar *display_name;
	gboolean add_to_recent;
} AddToPlaylistData;

static void
add_to_playlist_and_play_cb (IdolPlaylist *playlist, GAsyncResult *async_result, AddToPlaylistData *data)
{
	int end;
	gboolean playlist_changed;

	playlist_changed = idol_playlist_add_mrl_finish (playlist, async_result);

	if (data->add_to_recent != FALSE)
		gtk_recent_manager_add_item (data->idol->recent_manager, data->uri);
	end = idol_playlist_get_last (playlist);

	idol_signal_unblock_by_data (playlist, data->idol);

	if (playlist_changed && end != -1) {
		char *mrl, *subtitle;

		subtitle = NULL;
		idol_playlist_set_current (playlist, end);
		mrl = idol_playlist_get_current_mrl (playlist, &subtitle);
		idol_action_set_mrl_and_play (data->idol, mrl, subtitle);
		g_free (mrl);
		g_free (subtitle);
	}

	/* Free the closure data */
	g_object_unref (data->idol);
	g_free (data->uri);
	g_free (data->display_name);
	g_slice_free (AddToPlaylistData, data);
}

/**
 * idol_add_to_playlist_and_play:
 * @idol: a #IdolObject
 * @uri: the URI to add to the playlist
 * @display_name: the display name of the URI
 * @add_to_recent: if %TRUE, add the URI to the recent items list
 *
 * Add @uri to the playlist and play it immediately.
 **/
void
idol_add_to_playlist_and_play (Idol *idol,
				const char *uri,
				const char *display_name,
				gboolean add_to_recent)
{
	AddToPlaylistData *data;

	/* Block all signals from the playlist until we're finished. They're unblocked in the callback, add_to_playlist_and_play_cb.
	 * There are no concurrency issues here, since blocking the signals multiple times should require them to be unblocked the
	 * same number of times before they fire again. */
	idol_signal_block_by_data (idol->playlist, idol);

	data = g_slice_new (AddToPlaylistData);
	data->idol = g_object_ref (idol);
	data->uri = g_strdup (uri);
	data->display_name = g_strdup (display_name);
	data->add_to_recent = add_to_recent;

	idol_playlist_add_mrl (idol->playlist, uri, display_name, TRUE,
	                        NULL, (GAsyncReadyCallback) add_to_playlist_and_play_cb, data);
}

/**
 * idol_get_current_mrl:
 * @idol: a #IdolObject
 *
 * Get the MRL of the current stream, or %NULL if nothing's playing.
 * Free with g_free().
 *
 * Return value: a newly-allocated string containing the MRL of the current stream
 **/
char *
idol_get_current_mrl (Idol *idol)
{
	return idol_playlist_get_current_mrl (idol->playlist, NULL);
}

/**
 * idol_get_playlist_length:
 * @idol: a #IdolObject
 *
 * Returns the length of the current playlist.
 *
 * Return value: the playlist length
 **/
guint
idol_get_playlist_length (Idol *idol)
{
	int last;

	last = idol_playlist_get_last (idol->playlist);
	if (last == -1)
		return 0;
	return last + 1;
}

/**
 * idol_get_playlist_pos:
 * @idol: a #IdolObject
 *
 * Returns the %0-based index of the current entry in the playlist. If
 * there is no current entry in the playlist, %-1 is returned.
 *
 * Return value: the index of the current playlist entry, or %-1
 **/
int
idol_get_playlist_pos (Idol *idol)
{
	return idol_playlist_get_current (idol->playlist);
}

/**
 * idol_get_title_at_playlist_pos:
 * @idol: a #IdolObject
 * @playlist_index: the %0-based entry index
 *
 * Gets the title of the playlist entry at @index.
 *
 * Return value: the entry title at @index, or %NULL; free with g_free()
 **/
char *
idol_get_title_at_playlist_pos (Idol *idol, guint playlist_index)
{
	return idol_playlist_get_title (idol->playlist, playlist_index);
}

/**
 * idol_get_short_title:
 * @idol: a #IdolObject
 *
 * Gets the title of the current entry in the playlist.
 *
 * Return value: the current entry's title, or %NULL; free with g_free()
 **/
char *
idol_get_short_title (Idol *idol)
{
	gboolean custom;
	return idol_playlist_get_current_title (idol->playlist, &custom);
}

/**
 * idol_set_current_subtitle:
 * @idol: a #IdolObject
 * @subtitle_uri: the URI of the subtitle file to add
 *
 * Add the @subtitle_uri subtitle file to the playlist, setting it as the subtitle for the current
 * playlist entry.
 **/
void
idol_set_current_subtitle (Idol *idol, const char *subtitle_uri)
{
	idol_playlist_set_current_subtitle (idol->playlist, subtitle_uri);
}

/**
 * idol_add_sidebar_page:
 * @idol: a #IdolObject
 * @page_id: a string used to identify the page
 * @title: the page's title
 * @main_widget: the main widget for the page
 *
 * Adds a sidebar page to Idol's sidebar with the given @page_id.
 * @main_widget is added into the page and shown automatically, while
 * @title is displayed as the page's title in the tab bar.
 **/
void
idol_add_sidebar_page (Idol *idol,
			const char *page_id,
			const char *title,
			GtkWidget *main_widget)
{
	ev_sidebar_add_page (EV_SIDEBAR (idol->sidebar),
			     page_id,
			     title,
			     main_widget);
}

/**
 * idol_remove_sidebar_page:
 * @idol: a #IdolObject
 * @page_id: a string used to identify the page
 *
 * Removes the page identified by @page_id from Idol's sidebar.
 * If @page_id doesn't exist in the sidebar, this function does
 * nothing.
 **/
void
idol_remove_sidebar_page (Idol *idol,
			   const char *page_id)
{
	ev_sidebar_remove_page (EV_SIDEBAR (idol->sidebar),
				page_id);
}

/**
 * idol_file_opened:
 * @idol: a #IdolObject
 * @mrl: the MRL opened
 *
 * Emits the #IdolObject::file-opened signal on @idol, with the
 * specified @mrl.
 **/
void
idol_file_opened (IdolObject *idol,
		   const char *mrl)
{
	g_signal_emit (G_OBJECT (idol),
		       idol_table_signals[FILE_OPENED],
		       0, mrl);
}

/**
 * idol_file_closed:
 * @idol: a #IdolObject
 *
 * Emits the #IdolObject::file-closed signal on @idol.
 **/
void
idol_file_closed (IdolObject *idol)
{
	g_signal_emit (G_OBJECT (idol),
		       idol_table_signals[FILE_CLOSED],
		       0);

}

/**
 * idol_metadata_updated:
 * @idol: a #IdolObject
 * @artist: the stream's artist, or %NULL
 * @title: the stream's title, or %NULL
 * @album: the stream's album, or %NULL
 * @track_num: the track number of the stream
 *
 * Emits the #IdolObject::metadata-updated signal on @idol,
 * with the specified stream data.
 **/
void
idol_metadata_updated (IdolObject *idol,
			const char *artist,
			const char *title,
			const char *album,
			guint track_num)
{
	g_signal_emit (G_OBJECT (idol),
		       idol_table_signals[METADATA_UPDATED],
		       0,
		       artist,
		       title,
		       album,
		       track_num);
}

GQuark
idol_remote_command_quark (void)
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string ("idol_remote_command");

	return quark;
}

/* This should really be standard. */
#define ENUM_ENTRY(NAME, DESC) { NAME, "" #NAME "", DESC }

GType
idol_remote_command_get_type (void)
{
	static GType etype = 0;

	if (etype == 0) {
		static const GEnumValue values[] = {
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_UNKNOWN, "unknown"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_PLAY, "play"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_PAUSE, "pause"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_STOP, "stop"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_PLAYPAUSE, "play-pause"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_NEXT, "next"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_PREVIOUS, "previous"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_SEEK_FORWARD, "seek-forward"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_SEEK_BACKWARD, "seek-backward"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_VOLUME_UP, "volume-up"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_VOLUME_DOWN, "volume-down"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_FULLSCREEN, "fullscreen"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_QUIT, "quit"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_ENQUEUE, "enqueue"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_REPLACE, "replace"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_SHOW, "show"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_TOGGLE_CONTROLS, "toggle-controls"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_UP, "up"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_DOWN, "down"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_LEFT, "left"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_RIGHT, "right"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_SELECT, "select"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_DVD_MENU, "dvd-menu"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_ZOOM_UP, "zoom-up"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_ZOOM_DOWN, "zoom-down"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_EJECT, "eject"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_PLAY_DVD, "play-dvd"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_MUTE, "mute"),
			ENUM_ENTRY (IDOL_REMOTE_COMMAND_TOGGLE_ASPECT, "toggle-aspect-ratio"),
			{ 0, NULL, NULL }
		};

		etype = g_enum_register_static ("IdolRemoteCommand", values);
	}

	return etype;
}

GQuark
idol_remote_setting_quark (void)
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string ("idol_remote_setting");

	return quark;
}

GType
idol_remote_setting_get_type (void)
{
	static GType etype = 0;

	if (etype == 0) {
		static const GEnumValue values[] = {
			ENUM_ENTRY (IDOL_REMOTE_SETTING_SHUFFLE, "shuffle"),
			ENUM_ENTRY (IDOL_REMOTE_SETTING_REPEAT, "repeat"),
			{ 0, NULL, NULL }
		};

		etype = g_enum_register_static ("IdolRemoteSetting", values);
	}

	return etype;
}

static void
reset_seek_status (Idol *idol)
{
	/* Release the lock and reset everything so that we
	 * avoid being "stuck" seeking on errors */

	if (idol->seek_lock != FALSE) {
		idol_statusbar_set_seeking (IDOL_STATUSBAR (idol->statusbar), FALSE);
		idol_time_label_set_seeking (IDOL_TIME_LABEL (idol->fs->time_label), FALSE);
		idol->seek_lock = FALSE;
		bacon_video_widget_seek (idol->bvw, 0, NULL);
		idol_action_stop (idol);
	}
}

/**
 * idol_action_error:
 * @title: the error dialog title
 * @reason: the error dialog text
 * @idol: a #IdolObject
 *
 * Displays a non-blocking error dialog with the
 * given @title and @reason.
 **/
void
idol_action_error (const char *title, const char *reason, Idol *idol)
{
	reset_seek_status (idol);
	idol_interface_error (title, reason,
			GTK_WINDOW (idol->win));
}

G_GNUC_NORETURN void
idol_action_error_and_exit (const char *title,
		const char *reason, Idol *idol)
{
	reset_seek_status (idol);
	idol_interface_error_blocking (title, reason,
			GTK_WINDOW (idol->win));
	idol_action_exit (idol);
}

static void
idol_action_save_size (Idol *idol)
{
	GtkPaned *item;

	if (idol->bvw == NULL)
		return;

	if (idol_is_fullscreen (idol) != FALSE)
		return;

	/* Save the size of the video widget */
	item = GTK_PANED (gtk_builder_get_object (idol->xml, "tmw_main_pane"));
	gtk_window_get_size (GTK_WINDOW (idol->win), &idol->window_w,
			&idol->window_h);
	idol->sidebar_w = idol->window_w
		- gtk_paned_get_position (item);
}

static void
idol_action_save_state (Idol *idol, const char *page_id)
{
	GKeyFile *keyfile;
	char *contents, *filename;

	if (idol->win == NULL)
		return;
	if (idol->window_w == 0
	    || idol->window_h == 0)
		return;

	keyfile = g_key_file_new ();
	g_key_file_set_integer (keyfile, "State",
				"window_w", idol->window_w);
	g_key_file_set_integer (keyfile, "State",
			"window_h", idol->window_h);
	g_key_file_set_boolean (keyfile, "State",
			"show_sidebar", idol_sidebar_is_visible (idol));
	g_key_file_set_boolean (keyfile, "State",
			"maximised", idol->maximised);
	g_key_file_set_integer (keyfile, "State",
			"sidebar_w", idol->sidebar_w);

	g_key_file_set_string (keyfile, "State",
			"sidebar_page", page_id);

	contents = g_key_file_to_data (keyfile, NULL, NULL);
	g_key_file_free (keyfile);
	filename = g_build_filename (idol_dot_dir (), "state.ini", NULL);
	g_file_set_contents (filename, contents, -1, NULL);

	g_free (filename);
	g_free (contents);
}

G_GNUC_NORETURN static void
idol_action_wait_force_exit (gpointer user_data)
{
	g_usleep (10 * G_USEC_PER_SEC);
	exit (1);
}

/**
 * idol_action_exit:
 * @idol: a #IdolObject
 *
 * Closes Idol.
 **/
void
idol_action_exit (Idol *idol)
{
	GdkDisplay *display = NULL;
	char *page_id;

	/* Exit forcefully if we can't do the shutdown in 10 seconds */
	g_thread_create ((GThreadFunc) idol_action_wait_force_exit,
			 NULL, FALSE, NULL);

	if (gtk_main_level () > 0)
		gtk_main_quit ();

	if (idol == NULL)
		exit (0);

	if (idol->win != NULL) {
		gtk_widget_hide (idol->win);
		display = gtk_widget_get_display (idol->win);
	}

	if (idol->prefs != NULL)
		gtk_widget_hide (idol->prefs);

	/* Save the page ID before we close the plugins, otherwise
	 * we'll never save it properly */
	page_id = idol_sidebar_get_current_page (idol);
	idol_object_plugins_shutdown ();

	if (display != NULL)
		gdk_display_sync (display);

	if (idol->bvw) {
		idol_action_save_size (idol);
		idol_save_position (idol);
		bacon_video_widget_close (idol->bvw);
	}

	if (idol->app != NULL)
		g_object_unref (idol->app);
	idol_action_save_state (idol, page_id);
	g_free (page_id);

	idol_sublang_exit (idol);
	idol_destroy_file_filters ();

	if (idol->gc)
		g_object_unref (G_OBJECT (idol->gc));

	if (idol->fs)
		g_object_unref (idol->fs);

	if (idol->win)
		gtk_widget_destroy (GTK_WIDGET (idol->win));

	g_object_unref (idol);

	exit (0);
}

static void
idol_action_menu_popup (Idol *idol, guint button)
{
	GtkWidget *menu;

	menu = gtk_ui_manager_get_widget (idol->ui_manager,
			"/idol-main-popup");
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
			button, gtk_get_current_event_time ());
	gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);
}

G_GNUC_NORETURN gboolean
main_window_destroy_cb (GtkWidget *widget, GdkEvent *event, Idol *idol)
{
	idol_action_exit (idol);
}

static void
play_pause_set_label (Idol *idol, IdolStates state)
{
	GtkAction *action;
	const char *id, *tip;
	GSList *l, *proxies;

	if (state == idol->state)
		return;

	switch (state)
	{
	case STATE_PLAYING:
		idol_statusbar_set_text (IDOL_STATUSBAR (idol->statusbar),
				_("Playing"));
		id = GTK_STOCK_MEDIA_PAUSE;
		tip = N_("Pause");
		idol_playlist_set_playing (idol->playlist, IDOL_PLAYLIST_STATUS_PLAYING);
		break;
	case STATE_PAUSED:
		idol_statusbar_set_text (IDOL_STATUSBAR (idol->statusbar),
				_("Paused"));
		id = GTK_STOCK_MEDIA_PLAY;
		tip = N_("Play");
		idol_playlist_set_playing (idol->playlist, IDOL_PLAYLIST_STATUS_PAUSED);
		break;
	case STATE_STOPPED:
		idol_statusbar_set_text (IDOL_STATUSBAR (idol->statusbar),
				_("Stopped"));
		idol_statusbar_set_time_and_length
			(IDOL_STATUSBAR (idol->statusbar), 0, 0);
		id = GTK_STOCK_MEDIA_PLAY;
		idol_playlist_set_playing (idol->playlist, IDOL_PLAYLIST_STATUS_NONE);
		tip = N_("Play");
		break;
	default:
		g_assert_not_reached ();
		return;
	}

	action = gtk_action_group_get_action (idol->main_action_group, "play");
	g_object_set (G_OBJECT (action),
			"tooltip", _(tip),
			"stock-id", id, NULL);

	proxies = gtk_action_get_proxies (action);
	for (l = proxies; l != NULL; l = l->next) {
		atk_object_set_name (gtk_widget_get_accessible (l->data),
				_(tip));
	}

	idol->state = state;

	g_object_notify (G_OBJECT (idol), "playing");
}

void
idol_action_eject (Idol *idol)
{
	GMount *mount;

	mount = idol_get_mount_for_media (idol->mrl);
	if (mount == NULL)
		return;

	g_free (idol->mrl);
	idol->mrl = NULL;
	bacon_video_widget_close (idol->bvw);
	idol_file_closed (idol);

	/* The volume monitoring will take care of removing the items */
	g_mount_eject_with_operation (mount, G_MOUNT_UNMOUNT_NONE, NULL, NULL, NULL, NULL);
	g_object_unref (mount);
}

void
idol_action_show_properties (Idol *idol)
{
	if (idol_is_fullscreen (idol) == FALSE)
		idol_sidebar_set_current_page (idol, "properties", TRUE);
}

/**
 * idol_action_play:
 * @idol: a #IdolObject
 *
 * Plays the current stream. If Idol is already playing, it continues
 * to play. If the stream cannot be played, and error dialog is displayed.
 **/
void
idol_action_play (Idol *idol)
{
	GError *err = NULL;
	int retval;
	char *msg, *disp;

	if (idol->mrl == NULL)
		return;

	if (bacon_video_widget_is_playing (idol->bvw) != FALSE)
		return;

	retval = bacon_video_widget_play (idol->bvw,  &err);
	play_pause_set_label (idol, retval ? STATE_PLAYING : STATE_STOPPED);

	if (retval != FALSE)
		return;

	disp = idol_uri_escape_for_display (idol->mrl);
	msg = g_strdup_printf(_("Idol could not play '%s'."), disp);
	g_free (disp);

	idol_action_error (msg, err->message, idol);
	idol_action_stop (idol);
	g_free (msg);
	g_error_free (err);
}

static void
idol_action_seek (Idol *idol, double pos)
{
	GError *err = NULL;
	int retval;

	if (idol->mrl == NULL)
		return;
	if (bacon_video_widget_is_seekable (idol->bvw) == FALSE)
		return;

	retval = bacon_video_widget_seek (idol->bvw, pos, &err);

	if (retval == FALSE)
	{
		char *msg, *disp;

		disp = idol_uri_escape_for_display (idol->mrl);
		msg = g_strdup_printf(_("Idol could not play '%s'."), disp);
		g_free (disp);

		reset_seek_status (idol);

		idol_action_error (msg, err->message, idol);
		g_free (msg);
		g_error_free (err);
	}
}

/**
 * idol_action_set_mrl_and_play:
 * @idol: a #IdolObject
 * @mrl: the MRL to play
 * @subtitle: a subtitle file to load, or %NULL
 *
 * Loads the specified @mrl and plays it, if possible.
 * Calls idol_action_set_mrl() then idol_action_play().
 * For more information, see the documentation for idol_action_set_mrl_with_warning().
 **/
void
idol_action_set_mrl_and_play (Idol *idol, const char *mrl, const char *subtitle)
{
	if (idol_action_set_mrl (idol, mrl, subtitle) != FALSE)
		idol_action_play (idol);
}

static gboolean
idol_action_open_dialog (Idol *idol, const char *path, gboolean play)
{
	GSList *filenames;
	gboolean playlist_modified;

	filenames = idol_add_files (GTK_WINDOW (idol->win), path);

	if (filenames == NULL)
		return FALSE;

	playlist_modified = idol_action_open_files_list (idol,
			filenames);

	if (playlist_modified == FALSE) {
		g_slist_foreach (filenames, (GFunc) g_free, NULL);
		g_slist_free (filenames);
		return FALSE;
	}

	g_slist_foreach (filenames, (GFunc) g_free, NULL);
	g_slist_free (filenames);

	if (play != FALSE) {
		char *mrl, *subtitle;

		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
		idol_action_set_mrl_and_play (idol, mrl, subtitle);
		g_free (mrl);
		g_free (subtitle);
	}

	return TRUE;
}

static gboolean
idol_action_load_media (Idol *idol, IdolDiscMediaType type, const char *device)
{
	char **mrls;
	GError *error = NULL;
	const char *link, *link_text, *secondary;
	gboolean retval;

	mrls = bacon_video_widget_get_mrls (idol->bvw, type, device, &error);
	if (mrls == NULL) {
		char *msg;

		/* No errors? Weird */
		if (error == NULL) {
			msg = g_strdup_printf (_("Idol could not play this media (%s) although a plugin is present to handle it."), _(idol_cd_get_human_readable_name (type)));
			idol_action_error (msg, _("You might want to check that a disc is present in the drive and that it is correctly configured."), idol);
			g_free (msg);
			return FALSE;
		}

		/* No plugin for the media type */
		if (g_error_matches (error, BVW_ERROR, BVW_ERROR_NO_PLUGIN_FOR_FILE) != FALSE) {
			link = "http://projects.mate.org/idol/#codecs";
			link_text = _("More information about media plugins");
			secondary = _("Please install the necessary plugins and restart Idol to be able to play this media.");
			if (type == MEDIA_TYPE_DVD || type == MEDIA_TYPE_VCD)
				msg = g_strdup_printf (_("Idol cannot play this type of media (%s) because it does not have the appropriate plugins to be able to read from the disc."), _(idol_cd_get_human_readable_name (type)));
			else
				msg = g_strdup_printf (_("Idol cannot play this type of media (%s) because you do not have the appropriate plugins to handle it."), _(idol_cd_get_human_readable_name (type)));
		/* Unsupported type (ie. CDDA) */
		} else if (g_error_matches (error, BVW_ERROR, BVW_ERROR_INVALID_LOCATION) != FALSE) {
			msg = g_strdup_printf(_("Idol cannot play this type of media (%s) because it is not supported."), _(idol_cd_get_human_readable_name (type)));
			idol_action_error (msg, _("Please insert another disc to play back."), idol);
			g_free (msg);
			return FALSE;
		} else {
			g_assert_not_reached ();
		}

		idol_interface_error_with_link (msg, secondary, link, link_text, GTK_WINDOW (idol->win), idol);
		g_free (msg);
		return FALSE;
	}

	retval = idol_action_open_files (idol, mrls);
	g_strfreev (mrls);

	return retval;
}

static gboolean
idol_action_load_media_device (Idol *idol, const char *device)
{
	IdolDiscMediaType type;
	GError *error = NULL;
	char *device_path, *url;
	gboolean retval;

	if (g_str_has_prefix (device, "file://") != FALSE)
		device_path = g_filename_from_uri (device, NULL, NULL);
	else
		device_path = g_strdup (device);

	type = idol_cd_detect_type_with_url (device_path, &url, &error);

	switch (type) {
		case MEDIA_TYPE_ERROR:
			idol_action_error (_("Idol was not able to play this disc."),
					    error ? error->message : _("No reason."),
					    idol);
			retval = FALSE;
			break;
		case MEDIA_TYPE_DATA:
			/* Set default location to the mountpoint of
			 * this device */
			retval = idol_action_open_dialog (idol, url, FALSE);
			break;
		case MEDIA_TYPE_DVD:
		case MEDIA_TYPE_VCD:
			retval = idol_action_load_media (idol, type, device_path);
			break;
		case MEDIA_TYPE_CDDA:
			idol_action_error (_("Idol does not support playback of Audio CDs"),
					    _("Please consider using a music player or a CD extractor to play this CD"),
					    idol);
			retval = FALSE;
			break;
		default:
			g_assert_not_reached ();
	}

	g_free (url);
	g_free (device_path);

	return retval;
}

/**
 * idol_action_play_media_device:
 * @idol: a #IdolObject
 * @device: the media device's path
 *
 * Attempts to play the media device (for example, a DVD drive or CD drive)
 * with the given @device path by first adding it to the playlist, then
 * playing it.
 *
 * An error dialog will be displayed if Idol cannot read or play what's on
 * the media device.
 **/
void
idol_action_play_media_device (Idol *idol, const char *device)
{
	char *mrl;

	if (idol_action_load_media_device (idol, device) != FALSE) {
		mrl = idol_playlist_get_current_mrl (idol->playlist, NULL);
		idol_action_set_mrl_and_play (idol, mrl, NULL);
		g_free (mrl);
	}
}

/**
 * idol_action_play_media:
 * @idol: a #IdolObject
 * @type: the type of disc media
 * @device: the media's device path
 *
 * Attempts to play the media found on @device (for example, a DVD in a drive or a DVB
 * tuner) by first adding it to the playlist, then playing it.
 *
 * An error dialog will be displayed if Idol cannot support media of @type.
 **/
void
idol_action_play_media (Idol *idol, IdolDiscMediaType type, const char *device)
{
	char *mrl;

	if (idol_action_load_media (idol, type, device) != FALSE) {
		mrl = idol_playlist_get_current_mrl (idol->playlist, NULL);
		idol_action_set_mrl_and_play (idol, mrl, NULL);
		g_free (mrl);
	}
}

/**
 * idol_action_stop:
 * @idol: a #IdolObject
 *
 * Stops the current stream.
 **/
void
idol_action_stop (Idol *idol)
{
	bacon_video_widget_stop (idol->bvw);
	play_pause_set_label (idol, STATE_STOPPED);
}

/**
 * idol_action_play_pause:
 * @idol: a #IdolObject
 *
 * Gets the current MRL from the playlist and attempts to play it.
 * If the stream is already playing, playback is paused.
 **/
void
idol_action_play_pause (Idol *idol)
{
	if (idol->mrl == NULL) {
		char *mrl, *subtitle;

		/* Try to pull an mrl from the playlist */
		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
		if (mrl == NULL) {
			play_pause_set_label (idol, STATE_STOPPED);
			return;
		} else {
			idol_action_set_mrl_and_play (idol, mrl, subtitle);
			g_free (mrl);
			g_free (subtitle);
			return;
		}
	}

	if (bacon_video_widget_is_playing (idol->bvw) == FALSE) {
		bacon_video_widget_play (idol->bvw, NULL);
		play_pause_set_label (idol, STATE_PLAYING);
	} else {
		bacon_video_widget_pause (idol->bvw);
		play_pause_set_label (idol, STATE_PAUSED);

		/* Save the stream position */
		idol_save_position (idol);
	}
}

/**
 * idol_action_pause:
 * @idol: a #IdolObject
 *
 * Pauses the current stream. If Idol is already paused, it continues
 * to be paused.
 **/
void
idol_action_pause (Idol *idol)
{
	if (bacon_video_widget_is_playing (idol->bvw) != FALSE) {
		bacon_video_widget_pause (idol->bvw);
		play_pause_set_label (idol, STATE_PAUSED);

		/* Save the stream position */
		idol_save_position (idol);
	}
}

gboolean
window_state_event_cb (GtkWidget *window, GdkEventWindowState *event,
		       Idol *idol)
{
	if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) {
		idol->maximised = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
                gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (idol->statusbar),
                                                   !idol->maximised);
		idol_action_set_sensitivity ("zoom-1-2", !idol->maximised);
		idol_action_set_sensitivity ("zoom-1-1", !idol->maximised);
		idol_action_set_sensitivity ("zoom-2-1", !idol->maximised);
		return FALSE;
	}

	if ((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) == 0)
		return FALSE;

	if (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) {
		if (idol->controls_visibility != IDOL_CONTROLS_UNDEFINED)
			idol_action_save_size (idol);
		idol_fullscreen_set_fullscreen (idol->fs, TRUE);

		idol->controls_visibility = IDOL_CONTROLS_FULLSCREEN;
		show_controls (idol, FALSE);
		idol_action_set_sensitivity ("fullscreen", FALSE);
	} else {
		GtkAction *action;

		idol_fullscreen_set_fullscreen (idol->fs, FALSE);

		action = gtk_action_group_get_action (idol->main_action_group,
				"show-controls");

		if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)))
			idol->controls_visibility = IDOL_CONTROLS_VISIBLE;
		else
			idol->controls_visibility = IDOL_CONTROLS_HIDDEN;

		show_controls (idol, TRUE);
		idol_action_set_sensitivity ("fullscreen", TRUE);
	}

	g_object_notify (G_OBJECT (idol), "fullscreen");

	return FALSE;
}

/**
 * idol_action_fullscreen_toggle:
 * @idol: a #IdolObject
 *
 * Toggles Idol's fullscreen state; if Idol is fullscreened, calling
 * this makes it unfullscreened and vice-versa.
 **/
void
idol_action_fullscreen_toggle (Idol *idol)
{
	if (idol_is_fullscreen (idol) != FALSE)
		gtk_window_unfullscreen (GTK_WINDOW (idol->win));
	else
		gtk_window_fullscreen (GTK_WINDOW (idol->win));
}

/**
 * idol_action_fullscreen:
 * @idol: a #IdolObject
 * @state: %TRUE if Idol should be fullscreened
 *
 * Sets Idol's fullscreen state according to @state.
 **/
void
idol_action_fullscreen (Idol *idol, gboolean state)
{
	if (idol_is_fullscreen (idol) == state)
		return;

	idol_action_fullscreen_toggle (idol);
}

void
fs_exit1_activate_cb (GtkButton *button, Idol *idol)
{
	idol_action_fullscreen (idol, FALSE);
}

void
idol_action_open (Idol *idol)
{
	idol_action_open_dialog (idol, NULL, TRUE);
}

static void
idol_open_location_response_cb (GtkDialog *dialog, gint response, Idol *idol)
{
	char *uri;

	if (response != GTK_RESPONSE_OK) {
		gtk_widget_destroy (GTK_WIDGET (idol->open_location));
		return;
	}

	gtk_widget_hide (GTK_WIDGET (dialog));

	/* Open the specified URI */
	uri = idol_open_location_get_uri (idol->open_location);

	if (uri != NULL)
	{
		char *mrl, *subtitle;
		const char *filenames[2];

		filenames[0] = uri;
		filenames[1] = NULL;
		idol_action_open_files (idol, (char **) filenames);

		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
		idol_action_set_mrl_and_play (idol, mrl, subtitle);
		g_free (mrl);
		g_free (subtitle);
	}
 	g_free (uri);

	gtk_widget_destroy (GTK_WIDGET (idol->open_location));
}

void
idol_action_open_location (Idol *idol)
{
	if (idol->open_location != NULL) {
		gtk_window_present (GTK_WINDOW (idol->open_location));
		return;
	}

	idol->open_location = IDOL_OPEN_LOCATION (idol_open_location_new ());

	g_signal_connect (G_OBJECT (idol->open_location), "delete-event",
			G_CALLBACK (gtk_widget_destroy), NULL);
	g_signal_connect (G_OBJECT (idol->open_location), "response",
			G_CALLBACK (idol_open_location_response_cb), idol);
	g_object_add_weak_pointer (G_OBJECT (idol->open_location), (gpointer *)&(idol->open_location));

	gtk_window_set_transient_for (GTK_WINDOW (idol->open_location),
			GTK_WINDOW (idol->win));
	gtk_widget_show (GTK_WIDGET (idol->open_location));
}

static char *
idol_get_nice_name_for_stream (Idol *idol)
{
	GValue title_value = { 0, };
	GValue album_value = { 0, };
	GValue artist_value = { 0, };
	GValue value = { 0, };
	char *retval;
	int tracknum;

	bacon_video_widget_get_metadata (idol->bvw, BVW_INFO_TITLE, &title_value);
	bacon_video_widget_get_metadata (idol->bvw, BVW_INFO_ARTIST, &artist_value);
	bacon_video_widget_get_metadata (idol->bvw, BVW_INFO_ALBUM, &album_value);
	bacon_video_widget_get_metadata (idol->bvw,
					 BVW_INFO_TRACK_NUMBER,
					 &value);

	tracknum = g_value_get_int (&value);
	g_value_unset (&value);

	idol_metadata_updated (idol,
				g_value_get_string (&artist_value),
				g_value_get_string (&title_value),
				g_value_get_string (&album_value),
				tracknum);

	if (g_value_get_string (&title_value) == NULL) {
		retval = NULL;
		goto bail;
	}
	if (g_value_get_string (&artist_value) == NULL) {
		retval = g_value_dup_string (&title_value);
		goto bail;
	}

	if (tracknum != 0) {
		retval = g_strdup_printf ("%02d. %s - %s",
					  tracknum,
					  g_value_get_string (&artist_value),
					  g_value_get_string (&title_value));
	} else {
		retval = g_strdup_printf ("%s - %s",
					  g_value_get_string (&artist_value),
					  g_value_get_string (&title_value));
	}

bail:
	g_value_unset (&album_value);
	g_value_unset (&artist_value);
	g_value_unset (&title_value);

	return retval;
}

static void
update_mrl_label (Idol *idol, const char *name)
{
	if (name != NULL)
	{
		/* Update the mrl label */
		idol_fullscreen_set_title (idol->fs, name);

		/* Title */
		gtk_window_set_title (GTK_WINDOW (idol->win), name);
	} else {
		idol_statusbar_set_time_and_length (IDOL_STATUSBAR
				(idol->statusbar), 0, 0);
		idol_statusbar_set_text (IDOL_STATUSBAR (idol->statusbar),
				_("Stopped"));

		g_object_notify (G_OBJECT (idol), "stream-length");

		/* Update the mrl label */
		idol_fullscreen_set_title (idol->fs, NULL);

		/* Title */
		gtk_window_set_title (GTK_WINDOW (idol->win), _("Movie Player"));
	}
}

/**
 * idol_action_set_mrl_with_warning:
 * @idol: a #IdolObject
 * @mrl: the MRL to play
 * @subtitle: a subtitle file to load, or %NULL
 * @warn: %TRUE if error dialogs should be displayed
 *
 * Loads the specified @mrl and optionally the specified subtitle
 * file. If @subtitle is %NULL Idol will attempt to auto-locate
 * any subtitle files for @mrl.
 *
 * If a stream is already playing, it will be stopped and closed.
 *
 * If any errors are encountered, error dialogs will only be displayed
 * if @warn is %TRUE.
 *
 * Return value: %TRUE on success
 **/
gboolean
idol_action_set_mrl_with_warning (Idol *idol,
				   const char *mrl, 
				   const char *subtitle,
				   gboolean warn)
{
	gboolean retval = TRUE;

	if (idol->mrl != NULL) {
		idol_save_position (idol);
		g_free (idol->mrl);
		idol->mrl = NULL;
		bacon_video_widget_close (idol->bvw);
		idol_file_closed (idol);
		play_pause_set_label (idol, STATE_STOPPED);
		update_fill (idol, -1.0);
	}

	if (mrl == NULL) {
		retval = FALSE;

		play_pause_set_label (idol, STATE_STOPPED);

		/* Play/Pause */
		idol_action_set_sensitivity ("play", FALSE);

		/* Volume */
		idol_main_set_sensitivity ("tmw_volume_button", FALSE);
		idol_action_set_sensitivity ("volume-up", FALSE);
		idol_action_set_sensitivity ("volume-down", FALSE);
		idol->volume_sensitive = FALSE;

		/* Control popup */
		idol_fullscreen_set_can_set_volume (idol->fs, FALSE);
		idol_fullscreen_set_seekable (idol->fs, FALSE);
		idol_action_set_sensitivity ("next-chapter", FALSE);
		idol_action_set_sensitivity ("previous-chapter", FALSE);

		/* Clear the playlist */
		idol_action_set_sensitivity ("clear-playlist", FALSE);

		/* Subtitle selection */
		idol_action_set_sensitivity ("select-subtitle", FALSE);

		/* Set the logo */
		bacon_video_widget_set_logo_mode (idol->bvw, TRUE);
		update_mrl_label (idol, NULL);

		/* Unset the drag */
		gtk_drag_source_unset (GTK_WIDGET (idol->bvw));
	} else {
		gboolean caps;
		gdouble volume;
		char *autoload_sub = NULL;
		GError *err = NULL;

		bacon_video_widget_set_logo_mode (idol->bvw, FALSE);

		if (subtitle == NULL && idol->autoload_subs != FALSE)
			autoload_sub = idol_uri_get_subtitle_uri (mrl);

		/* HACK: Bad bad Apple */
		if (g_str_has_prefix (mrl, "http://movies.apple.com")
				|| g_str_has_prefix (mrl, "http://trailers.apple.com"))
			bacon_video_widget_set_user_agent (idol->bvw, "Quicktime/7.2.0");
		else
			bacon_video_widget_set_user_agent (idol->bvw, NULL);

		idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (idol->win));
		idol_try_restore_position (idol, mrl);
		retval = bacon_video_widget_open (idol->bvw, mrl, subtitle ? subtitle : autoload_sub, &err);
		g_free (autoload_sub);
		gdk_window_set_cursor (gtk_widget_get_window (idol->win), NULL);
		idol->mrl = g_strdup (mrl);

		/* Play/Pause */
		idol_action_set_sensitivity ("play", TRUE);

		/* Volume */
		caps = bacon_video_widget_can_set_volume (idol->bvw);
		idol_main_set_sensitivity ("tmw_volume_button", caps);
		idol_fullscreen_set_can_set_volume (idol->fs, caps);
		volume = bacon_video_widget_get_volume (idol->bvw);
		idol_action_set_sensitivity ("volume-up", caps && volume < (1.0 - VOLUME_EPSILON));
		idol_action_set_sensitivity ("volume-down", caps && volume > VOLUME_EPSILON);
		idol->volume_sensitive = caps;

		/* Clear the playlist */
		idol_action_set_sensitivity ("clear-playlist", retval);

		/* Subtitle selection */
		idol_action_set_sensitivity ("select-subtitle", !idol_is_special_mrl (mrl) && retval);
	
		/* Set the playlist */
		play_pause_set_label (idol, retval ? STATE_PAUSED : STATE_STOPPED);

		if (retval == FALSE && warn != FALSE) {
			char *msg, *disp;

			disp = idol_uri_escape_for_display (idol->mrl);
			msg = g_strdup_printf(_("Idol could not play '%s'."), disp);
			g_free (disp);
			if (err && err->message) {
				idol_action_error (msg, err->message, idol);
			}
			else {
				idol_action_error (msg, _("No error message"), idol);
			}
			g_free (msg);
		}

		if (retval == FALSE) {
			if (err)
				g_error_free (err);
			g_free (idol->mrl);
			idol->mrl = NULL;
			bacon_video_widget_set_logo_mode (idol->bvw, TRUE);
		} else {
			char *display_name;
			/* cast is to shut gcc up */
			const GtkTargetEntry source_table[] = {
				{ (gchar*) "text/uri-list", 0, 0 }
			};

			idol_file_opened (idol, idol->mrl);

			/* Set the drag source */
			gtk_drag_source_set (GTK_WIDGET (idol->bvw),
					     GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
					     source_table, G_N_ELEMENTS (source_table),
					     GDK_ACTION_COPY);

			display_name = idol_playlist_get_current_title (idol->playlist, NULL);
			idol_action_add_recent (idol, idol->mrl, display_name);
			g_free (display_name);
		}
	}
	update_buttons (idol);
	update_media_menu_items (idol);

	return retval;
}

/**
 * idol_action_set_mrl:
 * @idol: a #IdolObject
 * @mrl: the MRL to load
 * @subtitle: a subtitle file to load, or %NULL
 *
 * Calls idol_action_set_mrl_with_warning() with warnings enabled.
 * For more information, see the documentation for idol_action_set_mrl_with_warning().
 *
 * Return value: %TRUE on success
 **/
gboolean
idol_action_set_mrl (Idol *idol, const char *mrl, const char *subtitle)
{
	return idol_action_set_mrl_with_warning (idol, mrl, subtitle, TRUE);
}

static gboolean
idol_time_within_seconds (Idol *idol)
{
	gint64 _time;

	_time = bacon_video_widget_get_current_time (idol->bvw);

	return (_time < REWIND_OR_PREVIOUS);
}

static void
idol_action_direction (Idol *idol, IdolPlaylistDirection dir)
{
	if (idol_playing_dvd (idol->mrl) == FALSE &&
		idol_playlist_has_direction (idol->playlist, dir) == FALSE
		&& idol_playlist_get_repeat (idol->playlist) == FALSE)
		return;

	if (idol_playing_dvd (idol->mrl) != FALSE)
	{
		bacon_video_widget_dvd_event (idol->bvw,
				dir == IDOL_PLAYLIST_DIRECTION_NEXT ?
				BVW_DVD_NEXT_CHAPTER :
				BVW_DVD_PREV_CHAPTER);
		return;
	}
	
	if (dir == IDOL_PLAYLIST_DIRECTION_NEXT
			|| bacon_video_widget_is_seekable (idol->bvw) == FALSE
			|| idol_time_within_seconds (idol) != FALSE)
	{
		char *mrl, *subtitle;

		idol_playlist_set_direction (idol->playlist, dir);
		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
		idol_action_set_mrl_and_play (idol, mrl, subtitle);

		g_free (subtitle);
		g_free (mrl);
	} else {
		idol_action_seek (idol, 0);
	}
}

/**
 * idol_action_previous:
 * @idol: a #IdolObject
 *
 * If a DVD is being played, goes to the previous chapter. If a normal stream
 * is being played, goes to the start of the stream if possible. If seeking is
 * not possible, plays the previous entry in the playlist.
 **/
void
idol_action_previous (Idol *idol)
{
	idol_action_direction (idol, IDOL_PLAYLIST_DIRECTION_PREVIOUS);
}

/**
 * idol_action_next:
 * @idol: a #IdolObject
 *
 * If a DVD is being played, goes to the next chapter. If a normal stream
 * is being played, plays the next entry in the playlist.
 **/
void
idol_action_next (Idol *idol)
{
	idol_action_direction (idol, IDOL_PLAYLIST_DIRECTION_NEXT);
}

static void
idol_seek_time_rel (Idol *idol, gint64 _time, gboolean relative, gboolean accurate)
{
	GError *err = NULL;
	gint64 sec;

	if (idol->mrl == NULL)
		return;
	if (bacon_video_widget_is_seekable (idol->bvw) == FALSE)
		return;

	idol_statusbar_set_seeking (IDOL_STATUSBAR (idol->statusbar), TRUE);
	idol_time_label_set_seeking (IDOL_TIME_LABEL (idol->fs->time_label), TRUE);

	if (relative != FALSE) {
		gint64 oldmsec;
		oldmsec = bacon_video_widget_get_current_time (idol->bvw);
		sec = MAX (0, oldmsec + _time);
	} else {
		sec = _time;
	}

	bacon_video_widget_seek_time (idol->bvw, sec, accurate, &err);

	idol_statusbar_set_seeking (IDOL_STATUSBAR (idol->statusbar), FALSE);
	idol_time_label_set_seeking (IDOL_TIME_LABEL (idol->fs->time_label), FALSE);

	if (err != NULL)
	{
		char *msg, *disp;

		disp = idol_uri_escape_for_display (idol->mrl);
		msg = g_strdup_printf(_("Idol could not play '%s'."), disp);
		g_free (disp);

		idol_action_stop (idol);
		idol_action_error (msg, err->message, idol);
		g_free (msg);
		g_error_free (err);
	}
}

/**
 * idol_action_seek_relative:
 * @idol: a #IdolObject
 * @offset: the time offset to seek to
 * @accurate: whether to use accurate seek, an accurate seek might be slower for some formats (see GStreamer docs)
 *
 * Seeks to an @offset from the current position in the stream,
 * or displays an error dialog if that's not possible.
 **/
void
idol_action_seek_relative (Idol *idol, gint64 offset, gboolean accurate)
{
	idol_seek_time_rel (idol, offset, TRUE, accurate);
}

/**
 * idol_action_seek_time:
 * @idol: a #IdolObject
 * @msec: the time to seek to
 * @accurate: whether to use accurate seek, an accurate seek might be slower for some formats (see GStreamer docs)
 *
 * Seeks to an absolute time in the stream, or displays an
 * error dialog if that's not possible.
 **/
void
idol_action_seek_time (Idol *idol, gint64 msec, gboolean accurate)
{
	idol_seek_time_rel (idol, msec, FALSE, accurate);
}

static void
idol_action_zoom (Idol *idol, double zoom)
{
	GtkAction *action;
	gboolean zoom_reset, zoom_in, zoom_out;

	if (zoom == ZOOM_ENABLE)
		zoom = bacon_video_widget_get_zoom (idol->bvw);

	if (zoom == ZOOM_DISABLE) {
		zoom_reset = zoom_in = zoom_out = FALSE;
	} else if (zoom < ZOOM_LOWER || zoom > ZOOM_UPPER) {
		return;
	} else {
		bacon_video_widget_set_zoom (idol->bvw, zoom);
		zoom_reset = (zoom != ZOOM_RESET);
		zoom_out = zoom != ZOOM_LOWER;
		zoom_in = zoom != ZOOM_UPPER;
	}

	action = gtk_action_group_get_action (idol->zoom_action_group,
			"zoom-in");
	gtk_action_set_sensitive (action, zoom_in);

	action = gtk_action_group_get_action (idol->zoom_action_group,
			"zoom-out");
	gtk_action_set_sensitive (action, zoom_out);

	action = gtk_action_group_get_action (idol->zoom_action_group,
			"zoom-reset");
	gtk_action_set_sensitive (action, zoom_reset);
}

void
idol_action_zoom_relative (Idol *idol, double off_pct)
{
	double zoom;

	zoom = bacon_video_widget_get_zoom (idol->bvw);
	idol_action_zoom (idol, zoom + off_pct);
}

void
idol_action_zoom_reset (Idol *idol)
{
	idol_action_zoom (idol, ZOOM_RESET);
}

/**
 * idol_get_volume:
 * @idol: a #IdolObject
 *
 * Gets the current volume level, as a value between %0.0 and %1.0.
 *
 * Return value: the volume level
 **/
double
idol_get_volume (Idol *idol)
{
	return bacon_video_widget_get_volume (idol->bvw);
}

/**
 * idol_action_volume:
 * @idol: a #IdolObject
 * @volume: the new absolute volume value
 *
 * Sets the volume, with %1.0 being the maximum, and %0.0 being the minimum level.
 **/
void
idol_action_volume (Idol *idol, double volume)
{
	if (bacon_video_widget_can_set_volume (idol->bvw) == FALSE)
		return;

	bacon_video_widget_set_volume (idol->bvw, volume);
}

/**
 * idol_action_volume_relative:
 * @idol: a #IdolObject
 * @off_pct: the value by which to increase or decrease the volume
 *
 * Sets the volume relative to its current level, with %1.0 being the
 * maximum, and %0.0 being the minimum level.
 **/
void
idol_action_volume_relative (Idol *idol, double off_pct)
{
	double vol;

	if (bacon_video_widget_can_set_volume (idol->bvw) == FALSE)
		return;
	if (idol->muted != FALSE)
		idol_action_volume_toggle_mute (idol);

	vol = bacon_video_widget_get_volume (idol->bvw);
	bacon_video_widget_set_volume (idol->bvw, vol + off_pct);
}

/**
 * idol_action_volume_toggle_mute:
 * @idol: a #IdolObject
 *
 * Toggles the mute status.
 **/
void
idol_action_volume_toggle_mute (Idol *idol)
{
	if (idol->muted == FALSE) {
		idol->muted = TRUE;
		idol->prev_volume = bacon_video_widget_get_volume (idol->bvw);
		bacon_video_widget_set_volume (idol->bvw, 0.0);
	} else {
		idol->muted = FALSE;
		bacon_video_widget_set_volume (idol->bvw, idol->prev_volume);
	}
}

/**
 * idol_action_toggle_aspect_ratio:
 * @idol: a #IdolObject
 *
 * Toggles the aspect ratio selected in the menu to the
 * next one in the list.
 **/
void
idol_action_toggle_aspect_ratio (Idol *idol)
{
	GtkAction *action;
	int tmp;

	tmp = idol_action_get_aspect_ratio (idol);
	tmp++;
	if (tmp > BVW_RATIO_DVB)
		tmp = BVW_RATIO_AUTO;

	action = gtk_action_group_get_action (idol->main_action_group, "aspect-ratio-auto");
	gtk_radio_action_set_current_value (GTK_RADIO_ACTION (action), tmp);
}

/**
 * idol_action_set_aspect_ratio:
 * @idol: a #IdolObject
 * @ratio: the aspect ratio to use
 *
 * Sets the aspect ratio selected in the menu to @ratio,
 * as defined in #BvwAspectRatio.
 **/
void
idol_action_set_aspect_ratio (Idol *idol, int ratio)
{
	bacon_video_widget_set_aspect_ratio (idol->bvw, ratio);
}

/**
 * idol_action_get_aspect_ratio:
 * @idol: a #IdolObject
 *
 * Gets the current aspect ratio as defined in #BvwAspectRatio.
 *
 * Return value: the current aspect ratio
 **/
int
idol_action_get_aspect_ratio (Idol *idol)
{
	return (bacon_video_widget_get_aspect_ratio (idol->bvw));
}

/**
 * idol_action_set_scale_ratio:
 * @idol: a #IdolObject
 * @ratio: the scale ratio to use
 *
 * Sets the video scale ratio, as a float where, for example,
 * 1.0 is 1:1 and 2.0 is 2:1.
 **/
void
idol_action_set_scale_ratio (Idol *idol, gfloat ratio)
{
	bacon_video_widget_set_scale_ratio (idol->bvw, ratio);
}

void
idol_action_show_help (Idol *idol)
{
	GError *error = NULL;

	if (gtk_show_uri (gtk_widget_get_screen (idol->win), "ghelp:idol", gtk_get_current_event_time (), &error) == FALSE) {
		idol_action_error (_("Idol could not display the help contents."), error->message, idol);
		g_error_free (error);
	}
}

typedef struct {
	gint add_mrl_complete;
	Idol *idol;
} DropFilesData;

/* This is called in the main thread */
static void
idol_action_drop_files_finished (IdolPlaylist *playlist, GAsyncResult *result, DropFilesData *data)
{
	/* When add_mrl_complete reaches 0, this is the last callback to occur and we can safely reconnect the playlist's changed signal (which was
	 * disconnected below in idol_action_drop_files(). We can also free the data struct and generally clean up. */
	if (g_atomic_int_dec_and_test (&(data->add_mrl_complete)) == TRUE) {
		char *mrl, *subtitle;

		/* Reconnect the signal */
		g_signal_connect (G_OBJECT (playlist), "changed", G_CALLBACK (playlist_changed_cb), data->idol);
		mrl = idol_playlist_get_current_mrl (playlist, &subtitle);
		idol_action_set_mrl_and_play (data->idol, mrl, subtitle);
		g_free (mrl);
		g_free (subtitle);

		/* Free the data struct */
		g_object_unref (data->idol);
		g_slice_free (DropFilesData, data);
	}
}

static gboolean
idol_action_drop_files (Idol *idol, GtkSelectionData *data,
		int drop_type, gboolean empty_pl)
{
	char **list;
	guint i, len;
	DropFilesData *drop_files_data = NULL /* shut up gcc */;
	GList *p, *file_list;
	gboolean cleared = FALSE;

	list = g_uri_list_extract_uris ((const char *) gtk_selection_data_get_data (data));
	file_list = NULL;

	for (i = 0; list[i] != NULL; i++) {
		char *filename;

		if (list[i] == NULL)
			continue;

		filename = idol_create_full_path (list[i]);
		file_list = g_list_prepend (file_list,
					    filename ? filename : g_strdup (list[i]));
	}
	g_strfreev (list);

	if (file_list == NULL)
		return FALSE;

	if (drop_type != 1)
		file_list = g_list_sort (file_list, (GCompareFunc) strcmp);
	else
		file_list = g_list_reverse (file_list);

	/* How many files? Check whether those could be subtitles */
	len = g_list_length (file_list);
	if (len == 1 || (len == 2 && drop_type == 1)) {
		if (idol_uri_is_subtitle (file_list->data) != FALSE) {
			idol_playlist_set_current_subtitle (idol->playlist, file_list->data);
			goto bail;
		}
	}

	if (empty_pl != FALSE) {
		/* The function that calls us knows better if we should be doing something with the changed playlist... */
		g_signal_handlers_disconnect_by_func (G_OBJECT (idol->playlist), playlist_changed_cb, idol);
		idol_playlist_clear (idol->playlist);
		cleared = TRUE;

		/* Allocate some shared memory to count how many add_mrl operations have completed (see the comment below).
		 * It's freed in idol_action_drop_files_cb() once all add_mrl operations have finished. */
		drop_files_data = g_slice_new (DropFilesData);
		drop_files_data->add_mrl_complete = len;
		drop_files_data->idol = g_object_ref (idol);
	}

	/* Add each MRL to the playlist asynchronously */
	for (p = file_list; p != NULL; p = p->next) {
		const char *filename;
		char *title;

		filename = p->data;
		title = NULL;

		/* Super _NETSCAPE_URL trick */
		if (drop_type == 1) {
			p = p->next;
			if (p != NULL) {
				if (g_str_has_prefix (p->data, "File:") != FALSE)
					title = (char *)p->data + 5;
				else
					title = p->data;
			}
		}

		/* Add the MRL to the playlist. We need to reconnect playlist's "changed" signal once all of the add_mrl operations have completed,
		 * so we use a piece of allocated memory shared between the async operations to count how many have completed.
		 * If we haven't cleared the playlist, there's no need to do this. */
		if (cleared == TRUE) {
			idol_playlist_add_mrl (idol->playlist, filename, title, TRUE, NULL,
			                        (GAsyncReadyCallback) idol_action_drop_files_finished, drop_files_data);
		} else {
			idol_playlist_add_mrl (idol->playlist, filename, title, TRUE, NULL, NULL, NULL);
		}
	}

bail:
	g_list_foreach (file_list, (GFunc) g_free, NULL);
	g_list_free (file_list);

	return TRUE;
}

static void
drop_video_cb (GtkWidget     *widget,
	 GdkDragContext     *context,
	 gint                x,
	 gint                y,
	 GtkSelectionData   *data,
	 guint               info,
	 guint               _time,
	 Idol              *idol)
{
	GtkWidget *source_widget;
	gboolean empty_pl;
	GdkDragAction action = gdk_drag_context_get_selected_action (context);

	source_widget = gtk_drag_get_source_widget (context);

	/* Drop of video on itself */
	if (source_widget && widget == source_widget && action == GDK_ACTION_MOVE) {
		gtk_drag_finish (context, FALSE, FALSE, _time);
		return;
	}

	if (action == GDK_ACTION_ASK) {
		action = idol_drag_ask (idol_get_playlist_length (idol) > 0);
		gdk_drag_status (context, action, GDK_CURRENT_TIME);
	}

	/* User selected cancel */
	if (action == GDK_ACTION_DEFAULT) {
		gtk_drag_finish (context, FALSE, FALSE, _time);
		return;
	}

	empty_pl = (action == GDK_ACTION_MOVE);
	idol_action_drop_files (idol, data, info, empty_pl);
	gtk_drag_finish (context, TRUE, FALSE, _time);
	return;
}

static void
drag_motion_video_cb (GtkWidget      *widget,
                      GdkDragContext *context,
                      gint            x,
                      gint            y,
                      guint           _time,
                      Idol          *idol)
{
	GdkModifierType mask;

	gdk_window_get_pointer (gtk_widget_get_window (widget), NULL, NULL, &mask);
	if (mask & GDK_CONTROL_MASK) {
		gdk_drag_status (context, GDK_ACTION_COPY, _time);
	} else if (mask & GDK_MOD1_MASK || gdk_drag_context_get_suggested_action (context) == GDK_ACTION_ASK) {
		gdk_drag_status (context, GDK_ACTION_ASK, _time);
	} else {
		gdk_drag_status (context, GDK_ACTION_MOVE, _time);
	}
}

static void
drop_playlist_cb (GtkWidget     *widget,
	       GdkDragContext     *context,
	       gint                x,
	       gint                y,
	       GtkSelectionData   *data,
	       guint               info,
	       guint               _time,
	       Idol              *idol)
{
	gboolean empty_pl;
	GdkDragAction action = gdk_drag_context_get_selected_action (context);

	if (action == GDK_ACTION_ASK) {
		action = idol_drag_ask (idol_get_playlist_length (idol) > 0);
		gdk_drag_status (context, action, GDK_CURRENT_TIME);
	}

	if (action == GDK_ACTION_DEFAULT) {
		gtk_drag_finish (context, FALSE, FALSE, _time);
		return;
	}

	empty_pl = (action == GDK_ACTION_MOVE);

	idol_action_drop_files (idol, data, info, empty_pl);
	gtk_drag_finish (context, TRUE, FALSE, _time);
}

static void
drag_motion_playlist_cb (GtkWidget      *widget,
			 GdkDragContext *context,
			 gint            x,
			 gint            y,
			 guint           _time,
			 Idol          *idol)
{
	GdkModifierType mask;

	gdk_window_get_pointer (gtk_widget_get_window (widget), NULL, NULL, &mask);

	if (mask & GDK_MOD1_MASK || gdk_drag_context_get_suggested_action (context) == GDK_ACTION_ASK)
		gdk_drag_status (context, GDK_ACTION_ASK, _time);
}
static void
drag_video_cb (GtkWidget *widget,
	       GdkDragContext *context,
	       GtkSelectionData *selection_data,
	       guint info,
	       guint32 _time,
	       gpointer callback_data)
{
	Idol *idol = (Idol *) callback_data;
	char *text;
	int len;
	GFile *file;

	g_assert (selection_data != NULL);

	if (idol->mrl == NULL)
		return;

	/* Canonicalise the MRL as a proper URI */
	file = g_file_new_for_commandline_arg (idol->mrl);
	text = g_file_get_uri (file);
	g_object_unref (file);

	g_return_if_fail (text != NULL);

	len = strlen (text);

	gtk_selection_data_set (selection_data, gtk_selection_data_get_target (selection_data),
				8, (guchar *) text, len);

	g_free (text);
}

static void
on_got_redirect (BaconVideoWidget *bvw, const char *mrl, Idol *idol)
{
	char *new_mrl;

	if (strstr (mrl, "://") != NULL) {
		new_mrl = NULL;
	} else {
		GFile *old_file, *parent, *new_file;
		char *old_mrl;

		/* Get the parent for the current MRL, that's our base */
		old_mrl = idol_playlist_get_current_mrl (IDOL_PLAYLIST (idol->playlist), NULL);
		old_file = g_file_new_for_uri (old_mrl);
		g_free (old_mrl);
		parent = g_file_get_parent (old_file);
		g_object_unref (old_file);

		/* Resolve the URL */
		new_file = g_file_get_child (parent, mrl);
		g_object_unref (parent);

		new_mrl = g_file_get_uri (new_file);
		g_object_unref (new_file);
	}

	bacon_video_widget_close (idol->bvw);
	idol_file_closed (idol);
	idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (idol->win));
	bacon_video_widget_open (idol->bvw, new_mrl ? new_mrl : mrl, NULL, NULL);
	idol_file_opened (idol, new_mrl ? new_mrl : mrl);
	gdk_window_set_cursor (gtk_widget_get_window (idol->win), NULL);
	bacon_video_widget_play (bvw, NULL);
	g_free (new_mrl);
}

static void
on_channels_change_event (BaconVideoWidget *bvw, Idol *idol)
{
	gchar *name;

	idol_sublang_update (idol);

	/* updated stream info (new song) */
	name = idol_get_nice_name_for_stream (idol);

	if (name != NULL) {
		update_mrl_label (idol, name);
		idol_playlist_set_title
			(IDOL_PLAYLIST (idol->playlist), name, TRUE);
		g_free (name);
	}
}

static void
on_playlist_change_name (IdolPlaylist *playlist, Idol *idol)
{
	char *name;

	name = idol_playlist_get_current_title (playlist, NULL);
	if (name != NULL) {
		update_mrl_label (idol, name);
		g_free (name);
	}
}

static void
on_got_metadata_event (BaconVideoWidget *bvw, Idol *idol)
{
        char *name = NULL;
	
	name = idol_get_nice_name_for_stream (idol);

	if (name != NULL) {
		idol_playlist_set_title
			(IDOL_PLAYLIST (idol->playlist), name, FALSE);
		g_free (name);
	}
	
	on_playlist_change_name (IDOL_PLAYLIST (idol->playlist), idol);
}

static void
on_error_event (BaconVideoWidget *bvw, char *message,
                gboolean playback_stopped, gboolean fatal, Idol *idol)
{
	/* Clear the seek if it's there, we only want to try and seek
	 * the first file, even if it's not there */
	idol->seek_to = 0;
	idol->seek_to_start = 0;

	if (playback_stopped)
		play_pause_set_label (idol, STATE_STOPPED);

	if (fatal == FALSE) {
		idol_action_error (_("An error occurred"), message, idol);
	} else {
		idol_action_error_and_exit (_("An error occurred"),
				message, idol);
	}
}

static void
on_buffering_event (BaconVideoWidget *bvw, int percentage, Idol *idol)
{
	idol_statusbar_push (IDOL_STATUSBAR (idol->statusbar), percentage);
}

static void
on_download_buffering_event (BaconVideoWidget *bvw, gdouble level, Idol *idol)
{
	update_fill (idol, level);
}

static void
update_fill (Idol *idol, gdouble level)
{
	if (level < 0.0) {
		gtk_range_set_show_fill_level (GTK_RANGE (idol->seek), FALSE);
		gtk_range_set_show_fill_level (GTK_RANGE (idol->fs->seek), FALSE);
	} else {
		gtk_range_set_fill_level (GTK_RANGE (idol->seek), level * 65535.0f);
		gtk_range_set_show_fill_level (GTK_RANGE (idol->seek), TRUE);

		gtk_range_set_fill_level (GTK_RANGE (idol->fs->seek), level * 65535.0f);
		gtk_range_set_show_fill_level (GTK_RANGE (idol->fs->seek), TRUE);
	}
}

static void
update_seekable (Idol *idol)
{
	GtkAction *action;
	GtkActionGroup *action_group;
	gboolean seekable;

	seekable = bacon_video_widget_is_seekable (idol->bvw);
	if (idol->seekable == seekable)
		return;
	idol->seekable = seekable;

	/* Check if the stream is seekable */
	gtk_widget_set_sensitive (idol->seek, seekable);

	idol_main_set_sensitivity ("tmw_seek_hbox", seekable);

	idol_fullscreen_set_seekable (idol->fs, seekable);

	/* FIXME: We can use this code again once bug #457631 is fixed and
	 * skip-* are back in the main action group. */
	/*idol_action_set_sensitivity ("skip-forward", seekable);
	idol_action_set_sensitivity ("skip-backwards", seekable);*/
	action_group = GTK_ACTION_GROUP (gtk_builder_get_object (idol->xml, "skip-action-group"));

	action = gtk_action_group_get_action (action_group, "skip-forward");
	gtk_action_set_sensitive (action, seekable);

	action = gtk_action_group_get_action (action_group, "skip-backwards");
	gtk_action_set_sensitive (action, seekable);

	/* This is for the session restore and the position saving
	 * to seek to the saved time */
	if (seekable != FALSE) {
		if (idol->seek_to_start != 0) {
			bacon_video_widget_seek_time (idol->bvw,
						      idol->seek_to_start, FALSE, NULL);
			idol_action_pause (idol);
		} else if (idol->seek_to != 0) {
			bacon_video_widget_seek_time (idol->bvw,
						      idol->seek_to, FALSE, NULL);
		}
	}
	idol->seek_to = 0;
	idol->seek_to_start = 0;

	g_object_notify (G_OBJECT (idol), "seekable");
}

static void
update_current_time (BaconVideoWidget *bvw,
		gint64 current_time,
		gint64 stream_length,
		double current_position,
		gboolean seekable, Idol *idol)
{
	if (idol->seek_lock == FALSE)
	{
		gtk_adjustment_set_value (idol->seekadj,
				current_position * 65535);

		if (stream_length == 0 && idol->mrl != NULL)
		{
			idol_statusbar_set_time_and_length
				(IDOL_STATUSBAR (idol->statusbar),
				(int) (current_time / 1000), -1);
		} else {
			idol_statusbar_set_time_and_length
				(IDOL_STATUSBAR (idol->statusbar),
				(int) (current_time / 1000),
				(int) (stream_length / 1000));
		}

		idol_time_label_set_time
			(IDOL_TIME_LABEL (idol->fs->time_label),
			 current_time, stream_length);
	}

	if (idol->stream_length != stream_length) {
		g_object_notify (G_OBJECT (idol), "stream-length");
		idol->stream_length = stream_length;
	}
}

void
volume_button_value_changed_cb (GtkScaleButton *button, gdouble value, Idol *idol)
{
	idol->muted = FALSE;
	bacon_video_widget_set_volume (idol->bvw, value);
}

static void
update_volume_sliders (Idol *idol)
{
	double volume;
	GtkAction *action;

	volume = bacon_video_widget_get_volume (idol->bvw);

	g_signal_handlers_block_by_func (idol->volume, volume_button_value_changed_cb, idol);
	gtk_scale_button_set_value (GTK_SCALE_BUTTON (idol->volume), volume);
	g_signal_handlers_unblock_by_func (idol->volume, volume_button_value_changed_cb, idol);
  
	action = gtk_action_group_get_action (idol->main_action_group, "volume-down");
	gtk_action_set_sensitive (action, volume > VOLUME_EPSILON && idol->volume_sensitive);

	action = gtk_action_group_get_action (idol->main_action_group, "volume-up");
	gtk_action_set_sensitive (action, volume < (1.0 - VOLUME_EPSILON) && idol->volume_sensitive);
}

static void
property_notify_cb_volume (BaconVideoWidget *bvw, GParamSpec *spec, Idol *idol)
{
	update_volume_sliders (idol);
}

static void
property_notify_cb_logo_mode (BaconVideoWidget *bvw, GParamSpec *spec, Idol *idol)
{
	gboolean enabled;
	enabled = bacon_video_widget_get_logo_mode (idol->bvw);
	idol_action_zoom (idol, enabled ? ZOOM_DISABLE : ZOOM_ENABLE);
}

static void
property_notify_cb_seekable (BaconVideoWidget *bvw, GParamSpec *spec, Idol *idol)
{
	update_seekable (idol);
}

gboolean
seek_slider_pressed_cb (GtkWidget *widget, GdkEventButton *event, Idol *idol)
{
	/* HACK: we want the behaviour you get with the middle button, so we
	 * mangle the event.  clicking with other buttons moves the slider in
	 * step increments, clicking with the middle button moves the slider to
	 * the location of the click.
	 */
	event->button = 2;

	idol->seek_lock = TRUE;
	if (bacon_video_widget_can_direct_seek (idol->bvw) == FALSE) {
		idol_statusbar_set_seeking (IDOL_STATUSBAR (idol->statusbar), TRUE);
		idol_time_label_set_seeking (IDOL_TIME_LABEL (idol->fs->time_label), TRUE);
	}

	return FALSE;
}

void
seek_slider_changed_cb (GtkAdjustment *adj, Idol *idol)
{
	double pos;
	gint _time;

	if (idol->seek_lock == FALSE)
		return;

	pos = gtk_adjustment_get_value (adj) / 65535;
	_time = bacon_video_widget_get_stream_length (idol->bvw);
	idol_statusbar_set_time_and_length (IDOL_STATUSBAR (idol->statusbar),
			(int) (pos * _time / 1000), _time / 1000);
	idol_time_label_set_time
			(IDOL_TIME_LABEL (idol->fs->time_label),
			 (int) (pos * _time), _time);

	if (bacon_video_widget_can_direct_seek (idol->bvw) != FALSE)
		idol_action_seek (idol, pos);
}

gboolean
seek_slider_released_cb (GtkWidget *widget, GdkEventButton *event, Idol *idol)
{
	GtkAdjustment *adj;
	gdouble val;

	/* HACK: see seek_slider_pressed_cb */
	event->button = 2;

	/* set to FALSE here to avoid triggering a final seek when
	 * syncing the adjustments while being in direct seek mode */
	idol->seek_lock = FALSE;

	/* sync both adjustments */
	adj = gtk_range_get_adjustment (GTK_RANGE (widget));
	val = gtk_adjustment_get_value (adj);

	if (bacon_video_widget_can_direct_seek (idol->bvw) == FALSE)
		idol_action_seek (idol, val / 65535.0);

	idol_statusbar_set_seeking (IDOL_STATUSBAR (idol->statusbar), FALSE);
	idol_time_label_set_seeking (IDOL_TIME_LABEL (idol->fs->time_label),
			FALSE);
	return FALSE;
}

gboolean
idol_action_open_files (Idol *idol, char **list)
{
	GSList *slist = NULL;
	int i, retval;

	for (i = 0 ; list[i] != NULL; i++)
		slist = g_slist_prepend (slist, list[i]);

	slist = g_slist_reverse (slist);
	retval = idol_action_open_files_list (idol, slist);
	g_slist_free (slist);

	return retval;
}

static gboolean
idol_action_open_files_list (Idol *idol, GSList *list)
{
	GSList *l;
	gboolean changed;
	gboolean cleared;

	changed = FALSE;
	cleared = FALSE;

	if (list == NULL)
		return changed;

	idol_gdk_window_set_waiting_cursor (gtk_widget_get_window (idol->win));

	for (l = list ; l != NULL; l = l->next)
	{
		char *filename;
		char *data = l->data;

		if (data == NULL)
			continue;

		/* Ignore relatives paths that start with "--", tough luck */
		if (data[0] == '-' && data[1] == '-')
			continue;

		/* Get the subtitle part out for our tests */
		filename = idol_create_full_path (data);
		if (filename == NULL)
			filename = g_strdup (data);

		if (g_file_test (filename, G_FILE_TEST_IS_REGULAR)
				|| strstr (filename, "#") != NULL
				|| strstr (filename, "://") != NULL
				|| g_str_has_prefix (filename, "dvd:") != FALSE
				|| g_str_has_prefix (filename, "vcd:") != FALSE
				|| g_str_has_prefix (filename, "dvb:") != FALSE)
		{
			if (cleared == FALSE)
			{
				/* The function that calls us knows better
				 * if we should be doing something with the 
				 * changed playlist ... */
				g_signal_handlers_disconnect_by_func
					(G_OBJECT (idol->playlist),
					 playlist_changed_cb, idol);
				changed = idol_playlist_clear (idol->playlist);
				bacon_video_widget_close (idol->bvw);
				idol_file_closed (idol);
				cleared = TRUE;
			}

			if (idol_is_block_device (filename) != FALSE) {
				idol_action_load_media_device (idol, data);
				changed = TRUE;
			} else if (g_str_has_prefix (filename, "dvb:/") != FALSE) {
				idol_playlist_add_mrl (idol->playlist, data, NULL, FALSE, NULL, NULL, NULL);
				changed = TRUE;
			} else {
				idol_playlist_add_mrl (idol->playlist, filename, NULL, FALSE, NULL, NULL, NULL);
				changed = TRUE;
			}
		}

		g_free (filename);
	}

	gdk_window_set_cursor (gtk_widget_get_window (idol->win), NULL);

	/* ... and reconnect because we're nice people */
	if (cleared != FALSE)
	{
		g_signal_connect (G_OBJECT (idol->playlist),
				"changed", G_CALLBACK (playlist_changed_cb),
				idol);
	}

	return changed;
}

void
show_controls (Idol *idol, gboolean was_fullscreen)
{
	GtkAction *action;
	GtkWidget *menubar, *controlbar, *statusbar, *bvw_box, *widget;
	GtkAllocation allocation;
	int width = 0, height = 0;

	if (idol->bvw == NULL)
		return;

	menubar = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_menubar_box"));
	controlbar = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_controls_vbox"));
	statusbar = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_statusbar"));
	bvw_box = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_bvw_box"));
	widget = GTK_WIDGET (idol->bvw);

	action = gtk_action_group_get_action (idol->main_action_group, "show-controls");
	gtk_action_set_sensitive (action, !idol_is_fullscreen (idol));
	gtk_widget_get_allocation (widget, &allocation);

	if (idol->controls_visibility == IDOL_CONTROLS_VISIBLE) {
		if (was_fullscreen == FALSE) {
			height = allocation.height;
			width = allocation.width;
		}

		gtk_widget_set_sensitive (menubar, TRUE);
		gtk_widget_show (menubar);
		gtk_widget_show (controlbar);
		gtk_widget_show (statusbar);
		if (idol_sidebar_is_visible (idol) != FALSE) {
			/* This is uglier then you might expect because of the
			   resize handle between the video and sidebar. There
			   is no convenience method to get the handle's width.
			   */
			GValue value = { 0, };
			GtkWidget *pane;
			GtkAllocation allocation_sidebar;
			int handle_size;

			g_value_init (&value, G_TYPE_INT);
			pane = GTK_WIDGET (gtk_builder_get_object (idol->xml,
					"tmw_main_pane"));
			gtk_widget_style_get_property (pane, "handle-size",
					&value);
			handle_size = g_value_get_int (&value);
			g_value_unset (&value);

			gtk_widget_show (idol->sidebar);
			gtk_widget_get_allocation (idol->sidebar, &allocation_sidebar);
			width += allocation_sidebar.width + handle_size;
		} else {
			gtk_widget_hide (idol->sidebar);
		}

		if (was_fullscreen == FALSE) {
			GtkAllocation allocation_menubar;
			GtkAllocation allocation_controlbar;
			GtkAllocation allocation_statusbar;

			gtk_widget_get_allocation (menubar, &allocation_menubar);
			gtk_widget_get_allocation (controlbar, &allocation_controlbar);
			gtk_widget_get_allocation (statusbar, &allocation_statusbar);
			height += allocation_menubar.height
				+ allocation_controlbar.height
				+ allocation_statusbar.height;
			gtk_window_resize (GTK_WINDOW(idol->win),
					width, height);
		}
	} else {
		if (idol->controls_visibility == IDOL_CONTROLS_HIDDEN) {
			width = allocation.width;
			height = allocation.height;
		}

		/* Hide and make the menubar unsensitive */
		gtk_widget_set_sensitive (menubar, FALSE);
		gtk_widget_hide (menubar);

		gtk_widget_hide (controlbar);
		gtk_widget_hide (statusbar);
		gtk_widget_hide (idol->sidebar);

		 /* We won't show controls in fullscreen */
		gtk_container_set_border_width (GTK_CONTAINER (bvw_box), 0);

		if (idol->controls_visibility == IDOL_CONTROLS_HIDDEN) {
			gtk_window_resize (GTK_WINDOW(idol->win),
					width, height);
		}
	}
}

/**
 * idol_action_toggle_controls:
 * @idol: a #IdolObject
 *
 * If Idol's not fullscreened, this toggles the state of the "Show Controls"
 * menu entry, and consequently shows or hides the controls in the UI.
 **/
void
idol_action_toggle_controls (Idol *idol)
{
	GtkAction *action;
	gboolean state;

	if (idol_is_fullscreen (idol) != FALSE)
		return;

 	action = gtk_action_group_get_action (idol->main_action_group,
 		"show-controls");
 	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
 	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), !state);
}

/**
 * idol_action_next_angle:
 * @idol: a #IdolObject
 *
 * Switches to the next angle, if watching a DVD. If not watching a DVD, this is a
 * no-op.
 **/
void
idol_action_next_angle (Idol *idol)
{
	if (idol_playing_dvd (idol->mrl) != FALSE)
		bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_NEXT_ANGLE);
}

/**
 * idol_action_set_playlist_index:
 * @idol: a #IdolObject
 * @index: the new playlist index
 *
 * Sets the %0-based playlist index to @index, causing Idol to load and
 * start playing that playlist entry.
 *
 * If @index is higher than the current length of the playlist, this
 * has the effect of restarting the current playlist entry.
 **/
void
idol_action_set_playlist_index (Idol *idol, guint playlist_index)
{
	char *mrl, *subtitle;

	idol_playlist_set_current (idol->playlist, playlist_index);
	mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
	idol_action_set_mrl_and_play (idol, mrl, subtitle);
	g_free (mrl);
	g_free (subtitle);
}

/**
 * idol_action_remote:
 * @idol: a #IdolObject
 * @cmd: a #IdolRemoteCommand
 * @url: an MRL to play, or %NULL
 *
 * Executes the specified @cmd on this instance of Idol. If @cmd
 * is an operation requiring an MRL, @url is required; it can be %NULL
 * otherwise.
 *
 * If Idol's fullscreened and the operation is executed correctly,
 * the controls will appear as if the user had moved the mouse.
 **/
void
idol_action_remote (Idol *idol, IdolRemoteCommand cmd, const char *url)
{
	const char *icon_name;
	gboolean handled;

	icon_name = NULL;
	handled = TRUE;

	switch (cmd) {
	case IDOL_REMOTE_COMMAND_PLAY:
		idol_action_play (idol);
		icon_name = "gtk-media-play";
		break;
	case IDOL_REMOTE_COMMAND_PLAYPAUSE:
		if (bacon_video_widget_is_playing (idol->bvw) == FALSE)
			icon_name = "gtk-media-play";
		else
			icon_name = "gtk-media-pause";
		idol_action_play_pause (idol);
		break;
	case IDOL_REMOTE_COMMAND_PAUSE:
		idol_action_pause (idol);
		icon_name = "gtk-media-pause";
		break;
	case IDOL_REMOTE_COMMAND_STOP: {
		char *mrl, *subtitle;

		idol_playlist_set_at_start (idol->playlist);
		update_buttons (idol);
		idol_action_stop (idol);
		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
		if (mrl != NULL) {
			idol_action_set_mrl_with_warning (idol, mrl, subtitle, FALSE);
			bacon_video_widget_pause (idol->bvw);
			g_free (mrl);
			g_free (subtitle);
		}
		icon_name = "gtk-media-stop";
		break;
	};
	case IDOL_REMOTE_COMMAND_SEEK_FORWARD: {
		double offset = 0;

		if (url != NULL)
			offset = g_ascii_strtod (url, NULL);
		if (offset == 0) {
			idol_action_seek_relative (idol, SEEK_FORWARD_OFFSET * 1000, FALSE);
		} else {
			idol_action_seek_relative (idol, offset * 1000, FALSE);
		}
		icon_name = "gtk-media-forward";
		break;
	}
	case IDOL_REMOTE_COMMAND_SEEK_BACKWARD: {
		double offset = 0;

		if (url != NULL)
			offset = g_ascii_strtod (url, NULL);
		if (offset == 0)
			idol_action_seek_relative (idol, SEEK_BACKWARD_OFFSET * 1000, FALSE);
		else
			idol_action_seek_relative (idol,  - (offset * 1000), FALSE);
		icon_name = "gtk-media-rewind";
		break;
	}
	case IDOL_REMOTE_COMMAND_VOLUME_UP:
		idol_action_volume_relative (idol, VOLUME_UP_OFFSET);
		break;
	case IDOL_REMOTE_COMMAND_VOLUME_DOWN:
		idol_action_volume_relative (idol, VOLUME_DOWN_OFFSET);
		break;
	case IDOL_REMOTE_COMMAND_NEXT:
		idol_action_next (idol);
		icon_name = "gtk-media-next";
		break;
	case IDOL_REMOTE_COMMAND_PREVIOUS:
		idol_action_previous (idol);
		icon_name = "gtk-media-previous";
		break;
	case IDOL_REMOTE_COMMAND_FULLSCREEN:
		idol_action_fullscreen_toggle (idol);
		break;
	case IDOL_REMOTE_COMMAND_QUIT:
		idol_action_exit (idol);
		break;
	case IDOL_REMOTE_COMMAND_ENQUEUE:
		g_assert (url != NULL);
		idol_playlist_add_mrl (idol->playlist, url, NULL, TRUE, NULL, NULL, NULL);
		break;
	case IDOL_REMOTE_COMMAND_REPLACE:
		idol_playlist_clear (idol->playlist);
		if (url == NULL) {
			bacon_video_widget_close (idol->bvw);
			idol_file_closed (idol);
			idol_action_set_mrl (idol, NULL, NULL);
			break;
		}
		if (strcmp (url, "dvd:") == 0) {
			/* FIXME b0rked */
			idol_action_play_media (idol, MEDIA_TYPE_DVD, NULL);
		} else if (strcmp (url, "vcd:") == 0) {
			/* FIXME b0rked */
			idol_action_play_media (idol, MEDIA_TYPE_VCD, NULL);
		} else {
			idol_playlist_add_mrl (idol->playlist, url, NULL, TRUE, NULL, NULL, NULL);
		}
		break;
	case IDOL_REMOTE_COMMAND_SHOW:
		gtk_window_present (GTK_WINDOW (idol->win));
		break;
	case IDOL_REMOTE_COMMAND_TOGGLE_CONTROLS:
		if (idol->controls_visibility != IDOL_CONTROLS_FULLSCREEN)
		{
			GtkToggleAction *action;
			gboolean state;

			action = GTK_TOGGLE_ACTION (gtk_action_group_get_action
					(idol->main_action_group,
					 "show-controls"));
			state = gtk_toggle_action_get_active (action);
			gtk_toggle_action_set_active (action, !state);
		}
		break;
	case IDOL_REMOTE_COMMAND_UP:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_ROOT_MENU_UP);
		break;
	case IDOL_REMOTE_COMMAND_DOWN:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_ROOT_MENU_DOWN);
		break;
	case IDOL_REMOTE_COMMAND_LEFT:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_ROOT_MENU_LEFT);
		break;
	case IDOL_REMOTE_COMMAND_RIGHT:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_ROOT_MENU_RIGHT);
		break;
	case IDOL_REMOTE_COMMAND_SELECT:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_ROOT_MENU_SELECT);
		break;
	case IDOL_REMOTE_COMMAND_DVD_MENU:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_ROOT_MENU);
		break;
	case IDOL_REMOTE_COMMAND_ZOOM_UP:
		idol_action_zoom_relative (idol, ZOOM_IN_OFFSET);
		break;
	case IDOL_REMOTE_COMMAND_ZOOM_DOWN:
		idol_action_zoom_relative (idol, ZOOM_OUT_OFFSET);
		break;
	case IDOL_REMOTE_COMMAND_EJECT:
		idol_action_eject (idol);
		icon_name = "media-eject";
		break;
	case IDOL_REMOTE_COMMAND_PLAY_DVD:
		/* TODO - how to see if can, and play the DVD (like the menu item) */
		break;
	case IDOL_REMOTE_COMMAND_MUTE:
		idol_action_volume_toggle_mute (idol);
		break;
	case IDOL_REMOTE_COMMAND_TOGGLE_ASPECT:
		idol_action_toggle_aspect_ratio (idol);
		break;
	case IDOL_REMOTE_COMMAND_UNKNOWN:
	default:
		handled = FALSE;
		break;
	}

	if (handled != FALSE
			&& gtk_window_is_active (GTK_WINDOW (idol->win))
			&& idol_fullscreen_is_fullscreen (idol->fs) != FALSE) {
		idol_fullscreen_show_popups_or_osd (idol->fs, icon_name, TRUE);
	}
}

/**
 * idol_action_remote_set_setting:
 * @idol: a #IdolObject
 * @setting: a #IdolRemoteSetting
 * @value: the new value for the setting
 *
 * Sets @setting to @value on this instance of Idol.
 **/
void idol_action_remote_set_setting (Idol *idol,
				      IdolRemoteSetting setting,
				      gboolean value)
{
	GtkAction *action;

	action = NULL;

	switch (setting) {
	case IDOL_REMOTE_SETTING_SHUFFLE:
		action = gtk_action_group_get_action (idol->main_action_group, "shuffle-mode");
		break;
	case IDOL_REMOTE_SETTING_REPEAT:
		action = gtk_action_group_get_action (idol->main_action_group, "repeat-mode");
		break;
	default:
		g_assert_not_reached ();
	}

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), value);
}

/**
 * idol_action_remote_get_setting:
 * @idol: a #IdolObject
 * @setting: a #IdolRemoteSetting
 *
 * Returns the value of @setting for this instance of Idol.
 *
 * Return value: %TRUE if the setting is enabled, %FALSE otherwise
 **/
gboolean idol_action_remote_get_setting (Idol *idol,
					  IdolRemoteSetting setting)
{
	GtkAction *action;

	action = NULL;

	switch (setting) {
	case IDOL_REMOTE_SETTING_SHUFFLE:
		action = gtk_action_group_get_action (idol->main_action_group, "shuffle-mode");
		break;
	case IDOL_REMOTE_SETTING_REPEAT:
		action = gtk_action_group_get_action (idol->main_action_group, "repeat-mode");
		break;
	default:
		g_assert_not_reached ();
	}

	return gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
}

static void
playlist_changed_cb (GtkWidget *playlist, Idol *idol)
{
	char *mrl, *subtitle;

	update_buttons (idol);
	mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);

	if (mrl == NULL)
		return;

	if (idol_playlist_get_playing (idol->playlist) == IDOL_PLAYLIST_STATUS_NONE)
		idol_action_set_mrl_and_play (idol, mrl, subtitle);

	g_free (mrl);
	g_free (subtitle);
}

static void
item_activated_cb (GtkWidget *playlist, Idol *idol)
{
	idol_action_seek (idol, 0);
}

static void
current_removed_cb (GtkWidget *playlist, Idol *idol)
{
	char *mrl, *subtitle;

	/* Set play button status */
	play_pause_set_label (idol, STATE_STOPPED);
	mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);

	if (mrl == NULL) {
		g_free (subtitle);
		subtitle = NULL;
		idol_playlist_set_at_start (idol->playlist);
		update_buttons (idol);
		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
	} else {
		update_buttons (idol);
	}

	idol_action_set_mrl_and_play (idol, mrl, subtitle);
	g_free (mrl);
	g_free (subtitle);
}

static void
subtitle_changed_cb (GtkWidget *playlist, Idol *idol)
{
	char *mrl, *subtitle;

	idol_action_stop (idol);
	mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
	idol_action_set_mrl_and_play (idol, mrl, subtitle);

	g_free (mrl);
	g_free (subtitle);
}

static void
playlist_repeat_toggle_cb (IdolPlaylist *playlist, gboolean repeat, Idol *idol)
{
	GtkAction *action;

	action = gtk_action_group_get_action (idol->main_action_group, "repeat-mode");

	g_signal_handlers_block_matched (G_OBJECT (action), G_SIGNAL_MATCH_DATA, 0, 0,
			NULL, NULL, idol);

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), repeat);

	g_signal_handlers_unblock_matched (G_OBJECT (action), G_SIGNAL_MATCH_DATA, 0, 0,
			NULL, NULL, idol);
}

static void
playlist_shuffle_toggle_cb (IdolPlaylist *playlist, gboolean shuffle, Idol *idol)
{
	GtkAction *action;

	action = gtk_action_group_get_action (idol->main_action_group, "shuffle-mode");

	g_signal_handlers_block_matched (G_OBJECT (action), G_SIGNAL_MATCH_DATA, 0, 0,
			NULL, NULL, idol);

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), shuffle);

	g_signal_handlers_unblock_matched (G_OBJECT (action), G_SIGNAL_MATCH_DATA, 0, 0,
			NULL, NULL, idol);
}

/**
 * idol_is_fullscreen:
 * @idol: a #IdolObject
 *
 * Returns %TRUE if Idol is fullscreened.
 *
 * Return value: %TRUE if Idol is fullscreened
 **/
gboolean
idol_is_fullscreen (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), FALSE);

	return (idol->controls_visibility == IDOL_CONTROLS_FULLSCREEN);
}

/**
 * idol_is_playing:
 * @idol: a #IdolObject
 *
 * Returns %TRUE if Idol is playing a stream.
 *
 * Return value: %TRUE if Idol is playing a stream
 **/
gboolean
idol_is_playing (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), FALSE);

	if (idol->bvw == NULL)
		return FALSE;

	return bacon_video_widget_is_playing (idol->bvw) != FALSE;
}

/**
 * idol_is_paused:
 * @idol: a #IdolObject
 *
 * Returns %TRUE if playback is paused.
 *
 * Return value: %TRUE if playback is paused, %FALSE otherwise
 **/
gboolean
idol_is_paused (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), FALSE);

	return idol->state == STATE_PAUSED;
}

/**
 * idol_is_seekable:
 * @idol: a #IdolObject
 *
 * Returns %TRUE if the current stream is seekable.
 *
 * Return value: %TRUE if the current stream is seekable
 **/
gboolean
idol_is_seekable (Idol *idol)
{
	g_return_val_if_fail (IDOL_IS_OBJECT (idol), FALSE);

	if (idol->bvw == NULL)
		return FALSE;

	return bacon_video_widget_is_seekable (idol->bvw) != FALSE;
}

static void
on_mouse_click_fullscreen (GtkWidget *widget, Idol *idol)
{
	if (idol_fullscreen_is_fullscreen (idol->fs) != FALSE)
		idol_fullscreen_show_popups (idol->fs, TRUE);
}

static gboolean
on_video_button_press_event (BaconVideoWidget *bvw, GdkEventButton *event,
		Idol *idol)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
		gtk_widget_grab_focus (GTK_WIDGET (bvw));
		return TRUE;
	} else if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
		idol_action_fullscreen_toggle(idol);
		return TRUE;
	} else if (event->type == GDK_BUTTON_PRESS && event->button == 2) {
		if (idol_is_fullscreen (idol) != FALSE) {
			const char *icon_name;
			if (bacon_video_widget_is_playing (idol->bvw) == FALSE)
				icon_name = "gtk-media-play";
			else
				icon_name = "gtk-media-pause";
			idol_fullscreen_show_popups_or_osd (idol->fs, icon_name, FALSE);
		}
		idol_action_play_pause (idol);
		return TRUE;
	} else if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		idol_action_menu_popup (idol, event->button);
		return TRUE;
	}

	return FALSE;
}

static gboolean
on_eos_event (GtkWidget *widget, Idol *idol)
{
	reset_seek_status (idol);

	if (bacon_video_widget_get_logo_mode (idol->bvw) != FALSE)
		return FALSE;

	if (idol_playlist_has_next_mrl (idol->playlist) == FALSE &&
	    idol_playlist_get_repeat (idol->playlist) == FALSE &&
	    (idol_playlist_get_last (idol->playlist) != 0 ||
	     idol_is_seekable (idol) == FALSE)) {
		char *mrl, *subtitle;

		/* Set play button status */
		idol_playlist_set_at_start (idol->playlist);
		update_buttons (idol);
		idol_action_stop (idol);
		mrl = idol_playlist_get_current_mrl (idol->playlist, &subtitle);
		idol_action_set_mrl_with_warning (idol, mrl, subtitle, FALSE);
		bacon_video_widget_pause (idol->bvw);
		g_free (mrl);
		g_free (subtitle);
	} else {
		if (idol_playlist_get_last (idol->playlist) == 0 &&
		    idol_is_seekable (idol)) {
			if (idol_playlist_get_repeat (idol->playlist) != FALSE) {
				idol_action_seek_time (idol, 0, FALSE);
				idol_action_play (idol);
			} else {
				idol_action_pause (idol);
				idol_action_seek_time (idol, 0, FALSE);
			}
		} else {
			idol_action_next (idol);
		}
	}

	return FALSE;
}

static gboolean
idol_action_handle_key_release (Idol *idol, GdkEventKey *event)
{
	gboolean retval = TRUE;

	switch (event->keyval) {
	case GDK_Left:
	case GDK_Right:
		idol_statusbar_set_seeking (IDOL_STATUSBAR (idol->statusbar), FALSE);
		idol_time_label_set_seeking (IDOL_TIME_LABEL (idol->fs->time_label), FALSE);
		break;
	default:
		retval = FALSE;
	}

	return retval;
}

static void
idol_action_handle_seek (Idol *idol, GdkEventKey *event, gboolean is_forward)
{
	if (is_forward != FALSE) {
		if (event->state & GDK_SHIFT_MASK)
			idol_action_seek_relative (idol, SEEK_FORWARD_SHORT_OFFSET * 1000, FALSE);
		else if (event->state & GDK_CONTROL_MASK)
			idol_action_seek_relative (idol, SEEK_FORWARD_LONG_OFFSET * 1000, FALSE);
		else
			idol_action_seek_relative (idol, SEEK_FORWARD_OFFSET * 1000, FALSE);
	} else {
		if (event->state & GDK_SHIFT_MASK)
			idol_action_seek_relative (idol, SEEK_BACKWARD_SHORT_OFFSET * 1000, FALSE);
		else if (event->state & GDK_CONTROL_MASK)
			idol_action_seek_relative (idol, SEEK_BACKWARD_LONG_OFFSET * 1000, FALSE);
		else
			idol_action_seek_relative (idol, SEEK_BACKWARD_OFFSET * 1000, FALSE);
	}
}

static gboolean
idol_action_handle_key_press (Idol *idol, GdkEventKey *event)
{
	gboolean retval;
	const char *icon_name;

	retval = TRUE;
	icon_name = NULL;

	switch (event->keyval) {
	case GDK_A:
	case GDK_a:
		idol_action_toggle_aspect_ratio (idol);
		break;
	case GDK_AudioPrev:
	case GDK_Back:
	case GDK_B:
	case GDK_b:
		idol_action_previous (idol);
		icon_name = "gtk-media-previous";
		break;
	case GDK_C:
	case GDK_c:
		bacon_video_widget_dvd_event (idol->bvw,
				BVW_DVD_CHAPTER_MENU);
		break;
	case GDK_F11:
	case GDK_f:
	case GDK_F:
		idol_action_fullscreen_toggle (idol);
		break;
	case GDK_g:
	case GDK_G:
		idol_action_next_angle (idol);
		break;
	case GDK_h:
	case GDK_H:
		idol_action_toggle_controls (idol);
		break;
	case GDK_M:
	case GDK_m:
		bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU);
		break;
	case GDK_AudioNext:
	case GDK_Forward:
	case GDK_N:
	case GDK_n:
	case GDK_End:
		idol_action_next (idol);
		icon_name = "gtk-media-next";
		break;
	case GDK_OpenURL:
		idol_action_fullscreen (idol, FALSE);
		idol_action_open_location (idol);
		break;
	case GDK_O:
	case GDK_o:
	case GDK_Open:
		idol_action_fullscreen (idol, FALSE);
		idol_action_open (idol);
		break;
	case GDK_AudioPlay:
	case GDK_p:
	case GDK_P:
		if (event->state & GDK_CONTROL_MASK) {
			idol_action_show_properties (idol);
		} else {
			if (bacon_video_widget_is_playing (idol->bvw) == FALSE)
				icon_name = "gtk-media-play";
			else
				icon_name = "gtk-media-pause";
			idol_action_play_pause (idol);
		}
		break;
	case GDK_comma:
		idol_action_pause (idol);
		bacon_video_widget_step (idol->bvw, FALSE, NULL);
		break;
	case GDK_period:
		idol_action_pause (idol);
		bacon_video_widget_step (idol->bvw, TRUE, NULL);
		break;
	case GDK_AudioPause:
	case GDK_AudioStop:
		idol_action_pause (idol);
		icon_name = "gtk-media-pause";
		break;
	case GDK_q:
	case GDK_Q:
		idol_action_exit (idol);
		break;
	case GDK_r:
	case GDK_R:
	case GDK_ZoomIn:
		idol_action_zoom_relative (idol, ZOOM_IN_OFFSET);
		break;
	case GDK_t:
	case GDK_T:
	case GDK_ZoomOut:
		idol_action_zoom_relative (idol, ZOOM_OUT_OFFSET);
		break;
	case GDK_Eject:
		idol_action_eject (idol);
		icon_name = "media-eject";
		break;
	case GDK_Escape:
		if (event->state & GDK_SUPER_MASK)
			bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU);
		else
			idol_action_fullscreen (idol, FALSE);
		break;
	case GDK_space:
	case GDK_Return:
		{
			GtkWidget *focus = gtk_window_get_focus (GTK_WINDOW (idol->win));
			if (idol_is_fullscreen (idol) != FALSE || focus == NULL ||
			    focus == GTK_WIDGET (idol->bvw) || focus == idol->seek) {
				if (event->keyval == GDK_space) {
					if (bacon_video_widget_is_playing (idol->bvw) == FALSE)
						icon_name = "gtk-media-play";
					else
						icon_name = "gtk-media-pause";
					idol_action_play_pause (idol);
				} else if (bacon_video_widget_has_menus (idol->bvw) != FALSE) {
					bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU_SELECT);
				}
			} else
				retval = FALSE;
		}
		break;
	case GDK_Left:
	case GDK_Right:
		if (bacon_video_widget_has_menus (idol->bvw) == FALSE) {
			gboolean is_forward;

			is_forward = (event->keyval == GDK_Right);
			/* Switch direction in RTL environment */
			if (gtk_widget_get_direction (idol->win) == GTK_TEXT_DIR_RTL)
				is_forward = !is_forward;
			icon_name = is_forward ? "gtk-media-forward" : "gtk-media-rewind";

			idol_action_handle_seek (idol, event, is_forward);
		} else {
			if (event->keyval == GDK_Left)
				bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU_LEFT);
			else
				bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU_RIGHT);
		}
		break;
	case GDK_Home:
		idol_action_seek (idol, 0);
		icon_name = "gtk-media-rewind";
		break;
	case GDK_Up:
		if (bacon_video_widget_has_menus (idol->bvw) != FALSE)
			bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU_UP);
		else
			idol_action_volume_relative (idol, VOLUME_UP_OFFSET);
		break;
	case GDK_Down:
		if (bacon_video_widget_has_menus (idol->bvw) != FALSE)
			bacon_video_widget_dvd_event (idol->bvw, BVW_DVD_ROOT_MENU_DOWN);
		else
			idol_action_volume_relative (idol, VOLUME_DOWN_OFFSET);
		break;
	case GDK_0:
		if (event->state & GDK_CONTROL_MASK)
			idol_action_zoom_reset (idol);
		else
			idol_action_set_scale_ratio (idol, 0.5);
		break;
	case GDK_onehalf:
		idol_action_set_scale_ratio (idol, 0.5);
		break;
	case GDK_1:
		idol_action_set_scale_ratio (idol, 1);
		break;
	case GDK_2:
		idol_action_set_scale_ratio (idol, 2);
		break;
	case GDK_Menu:
		idol_action_menu_popup (idol, 0);
		break;
	case GDK_F10:
		if (!(event->state & GDK_SHIFT_MASK))
			return FALSE;

		idol_action_menu_popup (idol, 0);
		break;
	case GDK_plus:
	case GDK_KP_Add:
		if (!(event->state & GDK_CONTROL_MASK)) {
			idol_action_next (idol);
		} else {
			idol_action_zoom_relative (idol, ZOOM_IN_OFFSET);
		}
		break;
	case GDK_minus:
	case GDK_KP_Subtract:
		if (!(event->state & GDK_CONTROL_MASK)) {
			idol_action_previous (idol);
		} else {
			idol_action_zoom_relative (idol, ZOOM_OUT_OFFSET);
		}
		break;
	case GDK_KP_Up:
	case GDK_KP_8:
		bacon_video_widget_dvd_event (idol->bvw, 
					      BVW_DVD_ROOT_MENU_UP);
		break;
	case GDK_KP_Down:
	case GDK_KP_2:
		bacon_video_widget_dvd_event (idol->bvw, 
					      BVW_DVD_ROOT_MENU_DOWN);
		break;
	case GDK_KP_Right:
	case GDK_KP_6:
		bacon_video_widget_dvd_event (idol->bvw, 
					      BVW_DVD_ROOT_MENU_RIGHT);
		break;
	case GDK_KP_Left:
	case GDK_KP_4:
		bacon_video_widget_dvd_event (idol->bvw, 
					      BVW_DVD_ROOT_MENU_LEFT);
		break;
	case GDK_KP_Begin:
	case GDK_KP_5:
		bacon_video_widget_dvd_event (idol->bvw,
					      BVW_DVD_ROOT_MENU_SELECT);
	default:
		retval = FALSE;
	}

	if (idol_is_fullscreen (idol) != FALSE && icon_name != NULL)
		idol_fullscreen_show_popups_or_osd (idol->fs,
						     icon_name,
						     FALSE);

	return retval;
}

static gboolean
idol_action_handle_scroll (Idol *idol, GdkScrollDirection direction)
{
	gboolean retval = TRUE;

	if (idol_fullscreen_is_fullscreen (idol->fs) != FALSE)
		idol_fullscreen_show_popups (idol->fs, TRUE);

	switch (direction) {
	case GDK_SCROLL_UP:
		idol_action_seek_relative (idol, SEEK_FORWARD_SHORT_OFFSET * 1000, FALSE);
		break;
	case GDK_SCROLL_DOWN:
		idol_action_seek_relative (idol, SEEK_BACKWARD_SHORT_OFFSET * 1000, FALSE);
		break;
	default:
		retval = FALSE;
	}

	return retval;
}

gboolean
window_key_press_event_cb (GtkWidget *win, GdkEventKey *event, Idol *idol)
{
	gboolean sidebar_handles_kbd;

	/* Shortcuts disabled? */
	if (idol->disable_kbd_shortcuts != FALSE)
		return FALSE;

	/* Check whether the sidebar needs the key events */
	if (event->type == GDK_KEY_PRESS) {
		if (idol_sidebar_is_focused (idol, &sidebar_handles_kbd) != FALSE) {
			/* Make Escape pass the focus to the video widget */
			if (sidebar_handles_kbd == FALSE &&
			    event->keyval == GDK_Escape)
				gtk_widget_grab_focus (GTK_WIDGET (idol->bvw));
			return FALSE;
		}
	} else {
		if (idol_sidebar_is_focused (idol, NULL) != FALSE)
			return FALSE;
	}

	/* Special case Eject, Open, Open URI and
	 * seeking keyboard shortcuts */
	if (event->state != 0
			&& (event->state & GDK_CONTROL_MASK))
	{
		switch (event->keyval) {
		case GDK_E:
		case GDK_e:
		case GDK_O:
		case GDK_o:
		case GDK_L:
		case GDK_l:
		case GDK_q:
		case GDK_Q:
		case GDK_S:
		case GDK_s:
		case GDK_Right:
		case GDK_Left:
		case GDK_plus:
		case GDK_KP_Add:
		case GDK_minus:
		case GDK_KP_Subtract:
		case GDK_0:
			if (event->type == GDK_KEY_PRESS)
				return idol_action_handle_key_press (idol, event);
			else
				return idol_action_handle_key_release (idol, event);
		default:
			break;
		}
	}

	if (event->state != 0 && (event->state & GDK_SUPER_MASK)) {
		switch (event->keyval) {
		case GDK_Escape:
			if (event->type == GDK_KEY_PRESS)
				return idol_action_handle_key_press (idol, event);
			else
				return idol_action_handle_key_release (idol, event);
		default:
			break;
		}
	}


	/* If we have modifiers, and either Ctrl, Mod1 (Alt), or any
	 * of Mod3 to Mod5 (Mod2 is num-lock...) are pressed, we
	 * let Gtk+ handle the key */
	if (event->state != 0
			&& ((event->state & GDK_CONTROL_MASK)
			|| (event->state & GDK_MOD1_MASK)
			|| (event->state & GDK_MOD3_MASK)
			|| (event->state & GDK_MOD4_MASK)
			|| (event->state & GDK_MOD5_MASK)))
		return FALSE;

	if (event->type == GDK_KEY_PRESS) {
		return idol_action_handle_key_press (idol, event);
	} else {
		return idol_action_handle_key_release (idol, event);
	}
}

gboolean
window_scroll_event_cb (GtkWidget *win, GdkEventScroll *event, Idol *idol)
{
	return idol_action_handle_scroll (idol, event->direction);
}

static void
update_media_menu_items (Idol *idol)
{
	GMount *mount;
	gboolean playing;

	playing = idol_playing_dvd (idol->mrl);

	idol_action_set_sensitivity ("dvd-root-menu", playing);
	idol_action_set_sensitivity ("dvd-title-menu", playing);
	idol_action_set_sensitivity ("dvd-audio-menu", playing);
	idol_action_set_sensitivity ("dvd-angle-menu", playing);
	idol_action_set_sensitivity ("dvd-chapter-menu", playing);
	/* FIXME we should only show that if we have multiple angles */
	idol_action_set_sensitivity ("next-angle", playing);

	mount = idol_get_mount_for_media (idol->mrl);
	idol_action_set_sensitivity ("eject", mount != NULL);
	if (mount != NULL)
		g_object_unref (mount);
}

static void
update_buttons (Idol *idol)
{
	gboolean has_item;

	/* Previous */
	if (idol_playing_dvd (idol->mrl) != FALSE)
		has_item = bacon_video_widget_has_previous_track (idol->bvw);
	else
		has_item = idol_playlist_has_previous_mrl (idol->playlist);

	idol_action_set_sensitivity ("previous-chapter", has_item);

	/* Next */
	if (idol_playing_dvd (idol->mrl) != FALSE)
		has_item = bacon_video_widget_has_next_track (idol->bvw);
	else
		has_item = idol_playlist_has_next_mrl (idol->playlist);

	idol_action_set_sensitivity ("next-chapter", has_item);
}

void
main_pane_size_allocated (GtkWidget *main_pane, GtkAllocation *allocation, Idol *idol)
{
	gulong handler_id;

	if (!idol->maximised || gtk_widget_get_mapped (idol->win)) {
		handler_id = g_signal_handler_find (main_pane, 
				G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
				0, 0, NULL,
				main_pane_size_allocated, idol);
		g_signal_handler_disconnect (main_pane, handler_id);

		gtk_paned_set_position (GTK_PANED (main_pane), allocation->width - idol->sidebar_w);
	}
}

char *
idol_setup_window (Idol *idol)
{
	GKeyFile *keyfile;
	int w, h, i;
	gboolean show_sidebar;
	char *filename, *page_id;
	GError *err = NULL;
	GtkWidget *vbox;
	GdkColor black;

	filename = g_build_filename (idol_dot_dir (), "state.ini", NULL);
	keyfile = g_key_file_new ();
	if (g_key_file_load_from_file (keyfile, filename,
			G_KEY_FILE_NONE, NULL) == FALSE) {
		idol->sidebar_w = 0;
		w = DEFAULT_WINDOW_W;
		h = DEFAULT_WINDOW_H;
		show_sidebar = TRUE;
		page_id = NULL;
		g_free (filename);
	} else {
		g_free (filename);

		w = g_key_file_get_integer (keyfile, "State", "window_w", &err);
		if (err != NULL) {
			w = 0;
			g_error_free (err);
			err = NULL;
		}

		h = g_key_file_get_integer (keyfile, "State", "window_h", &err);
		if (err != NULL) {
			h = 0;
			g_error_free (err);
			err = NULL;
		}

		show_sidebar = g_key_file_get_boolean (keyfile, "State",
				"show_sidebar", &err);
		if (err != NULL) {
			show_sidebar = TRUE;
			g_error_free (err);
			err = NULL;
		}

		idol->maximised = g_key_file_get_boolean (keyfile, "State",
				"maximised", &err);
		if (err != NULL) {
			g_error_free (err);
			err = NULL;
		}

		page_id = g_key_file_get_string (keyfile, "State",
				"sidebar_page", &err);
		if (err != NULL) {
			g_error_free (err);
			page_id = NULL;
			err = NULL;
		}

		idol->sidebar_w = g_key_file_get_integer (keyfile, "State",
				"sidebar_w", &err);
		if (err != NULL) {
			g_error_free (err);
			idol->sidebar_w = 0;
		}
		g_key_file_free (keyfile);
	}

	if (w > 0 && h > 0 && idol->maximised == FALSE) {
		gtk_window_set_default_size (GTK_WINDOW (idol->win),
				w, h);
		idol->window_w = w;
		idol->window_h = h;
	} else if (idol->maximised != FALSE) {
		gtk_window_maximize (GTK_WINDOW (idol->win));
	}

	/* Set the vbox to be completely black */
	vbox = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_bvw_box"));
	gdk_color_parse ("Black", &black);
	for (i = 0; i <= GTK_STATE_INSENSITIVE; i++)
		gtk_widget_modify_bg (vbox, i, &black);

	idol_sidebar_setup (idol, show_sidebar, page_id);
	return page_id;
}

void
idol_callback_connect (Idol *idol)
{
	GtkWidget *item, *arrow;
	GtkAction *action;
	GtkActionGroup *action_group;
	GtkBox *box;

	/* Menu items */
	action = gtk_action_group_get_action (idol->main_action_group, "repeat-mode");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
		idol_playlist_get_repeat (idol->playlist));
	action = gtk_action_group_get_action (idol->main_action_group, "shuffle-mode");
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action),
		idol_playlist_get_shuffle (idol->playlist));

	/* Controls */
	box = GTK_BOX (gtk_builder_get_object (idol->xml, "tmw_buttons_hbox"));

	/* Previous */
	action = gtk_action_group_get_action (idol->main_action_group,
			"previous-chapter");
	item = gtk_action_create_tool_item (action);
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item), 
					_("Previous Chapter/Movie"));
	atk_object_set_name (gtk_widget_get_accessible (item),
			_("Previous Chapter/Movie"));
	gtk_box_pack_start (box, item, FALSE, FALSE, 0);

	/* Play/Pause */
	action = gtk_action_group_get_action (idol->main_action_group, "play");
	item = gtk_action_create_tool_item (action);
	atk_object_set_name (gtk_widget_get_accessible (item),
			_("Play / Pause"));
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
 					_("Play / Pause"));
	gtk_box_pack_start (box, item, FALSE, FALSE, 0);

	/* Next */
	action = gtk_action_group_get_action (idol->main_action_group,
			"next-chapter");
	item = gtk_action_create_tool_item (action);
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item), 
					_("Next Chapter/Movie"));
	atk_object_set_name (gtk_widget_get_accessible (item),
			_("Next Chapter/Movie"));
	gtk_box_pack_start (box, item, FALSE, FALSE, 0);

	/* Separator */
	item = GTK_WIDGET(gtk_separator_tool_item_new ());
	gtk_box_pack_start (box, item, FALSE, FALSE, 0);

	/* Fullscreen button */
	action = gtk_action_group_get_action (idol->main_action_group,
			"fullscreen");
	item = gtk_action_create_tool_item (action);
	gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
					_("Fullscreen"));
	atk_object_set_name (gtk_widget_get_accessible (item),
			_("Fullscreen"));
	gtk_box_pack_start (box, item, FALSE, FALSE, 0);

	/* Sidebar button (Drag'n'Drop) */
	box = GTK_BOX (gtk_builder_get_object (idol->xml, "tmw_sidebar_button_hbox"));
	action = gtk_action_group_get_action (idol->main_action_group, "sidebar");
	item = gtk_toggle_button_new ();
	gtk_activatable_set_related_action (GTK_ACTIVATABLE (item), action);
	arrow = gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_NONE);
	g_object_set_data (G_OBJECT (box), "arrow", arrow);
	gtk_button_set_image_position (GTK_BUTTON (item), GTK_POS_RIGHT);
	gtk_button_set_image (GTK_BUTTON (item), arrow);
	gtk_box_pack_start (box, item, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (item), "drag_data_received",
			G_CALLBACK (drop_playlist_cb), idol);
	g_signal_connect (G_OBJECT (item), "drag_motion",
			G_CALLBACK (drag_motion_playlist_cb), idol);
	gtk_drag_dest_set (item, GTK_DEST_DEFAULT_ALL,
			target_table, G_N_ELEMENTS (target_table),
			GDK_ACTION_COPY | GDK_ACTION_MOVE);

	/* Fullscreen window buttons */
	g_signal_connect (G_OBJECT (idol->fs->exit_button), "clicked",
			  G_CALLBACK (fs_exit1_activate_cb), idol);

	action = gtk_action_group_get_action (idol->main_action_group, "play");
	item = gtk_action_create_tool_item (action);
	gtk_box_pack_start (GTK_BOX (idol->fs->buttons_box), item, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (item), "clicked",
			G_CALLBACK (on_mouse_click_fullscreen), idol);

	action = gtk_action_group_get_action (idol->main_action_group, "previous-chapter");
	item = gtk_action_create_tool_item (action);
	gtk_box_pack_start (GTK_BOX (idol->fs->buttons_box), item, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (item), "clicked",
			G_CALLBACK (on_mouse_click_fullscreen), idol);

	action = gtk_action_group_get_action (idol->main_action_group, "next-chapter");
	item = gtk_action_create_tool_item (action);
	gtk_box_pack_start (GTK_BOX (idol->fs->buttons_box), item, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (item), "clicked",
			G_CALLBACK (on_mouse_click_fullscreen), idol);

	/* Connect the keys */
	gtk_widget_add_events (idol->win, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	/* Connect the mouse wheel */
	gtk_widget_add_events (idol->win, GDK_SCROLL_MASK);
	gtk_widget_add_events (idol->seek, GDK_SCROLL_MASK);
	gtk_widget_add_events (idol->fs->seek, GDK_SCROLL_MASK);

	/* FIXME Hack to fix bug #462286 and #563894 */
	g_signal_connect (G_OBJECT (idol->fs->seek), "button-press-event",
			G_CALLBACK (seek_slider_pressed_cb), idol);
	g_signal_connect (G_OBJECT (idol->fs->seek), "button-release-event",
			G_CALLBACK (seek_slider_released_cb), idol);
	g_signal_connect (G_OBJECT (idol->fs->seek), "scroll-event",
			  G_CALLBACK (window_scroll_event_cb), idol);


	/* Set sensitivity of the toolbar buttons */
	idol_action_set_sensitivity ("play", FALSE);
	idol_action_set_sensitivity ("next-chapter", FALSE);
	idol_action_set_sensitivity ("previous-chapter", FALSE);
	/* FIXME: We can use this code again once bug #457631 is fixed
	 * and skip-* are back in the main action group. */
	/*idol_action_set_sensitivity ("skip-forward", FALSE);
	idol_action_set_sensitivity ("skip-backwards", FALSE);*/

	action_group = GTK_ACTION_GROUP (gtk_builder_get_object (idol->xml, "skip-action-group"));

	action = gtk_action_group_get_action (action_group, "skip-forward");
	gtk_action_set_sensitive (action, FALSE);

	action = gtk_action_group_get_action (action_group, "skip-backwards");
	gtk_action_set_sensitive (action, FALSE);
}

void
playlist_widget_setup (Idol *idol)
{
	idol->playlist = IDOL_PLAYLIST (idol_playlist_new ());

	if (idol->playlist == NULL)
		idol_action_exit (idol);

	gtk_widget_show_all (GTK_WIDGET (idol->playlist));

	g_signal_connect (G_OBJECT (idol->playlist), "active-name-changed",
			  G_CALLBACK (on_playlist_change_name), idol);
	g_signal_connect (G_OBJECT (idol->playlist), "item-activated",
			  G_CALLBACK (item_activated_cb), idol);
	g_signal_connect (G_OBJECT (idol->playlist),
			  "changed", G_CALLBACK (playlist_changed_cb),
			  idol);
	g_signal_connect (G_OBJECT (idol->playlist),
			  "current-removed", G_CALLBACK (current_removed_cb),
			  idol);
	g_signal_connect (G_OBJECT (idol->playlist),
			  "repeat-toggled",
			  G_CALLBACK (playlist_repeat_toggle_cb),
			  idol);
	g_signal_connect (G_OBJECT (idol->playlist),
			  "shuffle-toggled",
			  G_CALLBACK (playlist_shuffle_toggle_cb),
			  idol);
	g_signal_connect (G_OBJECT (idol->playlist),
			  "subtitle-changed",
			  G_CALLBACK (subtitle_changed_cb),
			  idol);
}

void
video_widget_create (Idol *idol)
{
	GError *err = NULL;
	GtkContainer *container;
	BaconVideoWidget **bvw;

	idol->bvw = BACON_VIDEO_WIDGET
		(bacon_video_widget_new (-1, -1, BVW_USE_TYPE_VIDEO, &err));

	if (idol->bvw == NULL) {
		idol_action_error_and_exit (_("Idol could not startup."), err != NULL ? err->message : _("No reason."), idol);
		if (err != NULL)
			g_error_free (err);
	}

	idol_action_zoom (idol, ZOOM_RESET);

	g_signal_connect_after (G_OBJECT (idol->bvw),
			"button-press-event",
			G_CALLBACK (on_video_button_press_event),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"eos",
			G_CALLBACK (on_eos_event),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"got-redirect",
			G_CALLBACK (on_got_redirect),
			idol);
	g_signal_connect (G_OBJECT(idol->bvw),
			"channels-change",
			G_CALLBACK (on_channels_change_event),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"tick",
			G_CALLBACK (update_current_time),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"got-metadata",
			G_CALLBACK (on_got_metadata_event),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"buffering",
			G_CALLBACK (on_buffering_event),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"download-buffering",
			G_CALLBACK (on_download_buffering_event),
			idol);
	g_signal_connect (G_OBJECT (idol->bvw),
			"error",
			G_CALLBACK (on_error_event),
			idol);

	container = GTK_CONTAINER (gtk_builder_get_object (idol->xml, "tmw_bvw_box"));
	gtk_container_add (container,
			GTK_WIDGET (idol->bvw));

	/* Events for the widget video window as well */
	gtk_widget_add_events (GTK_WIDGET (idol->bvw),
			GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	g_signal_connect (G_OBJECT(idol->bvw), "key_press_event",
			G_CALLBACK (window_key_press_event_cb), idol);
	g_signal_connect (G_OBJECT(idol->bvw), "key_release_event",
			G_CALLBACK (window_key_press_event_cb), idol);

	g_signal_connect (G_OBJECT (idol->bvw), "drag_data_received",
			G_CALLBACK (drop_video_cb), idol);
	g_signal_connect (G_OBJECT (idol->bvw), "drag_motion",
			G_CALLBACK (drag_motion_video_cb), idol);
	gtk_drag_dest_set (GTK_WIDGET (idol->bvw), GTK_DEST_DEFAULT_ALL,
			target_table, G_N_ELEMENTS (target_table),
			GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect (G_OBJECT (idol->bvw), "drag_data_get",
			G_CALLBACK (drag_video_cb), idol);

	bvw = &(idol->bvw);
	g_object_add_weak_pointer (G_OBJECT (idol->bvw),
				   (gpointer *) bvw);

	gtk_widget_realize (GTK_WIDGET (idol->bvw));
	gtk_widget_show (GTK_WIDGET (idol->bvw));

	idol_preferences_visuals_setup (idol);

	g_signal_connect (G_OBJECT (idol->bvw), "notify::volume",
			G_CALLBACK (property_notify_cb_volume), idol);
	g_signal_connect (G_OBJECT (idol->bvw), "notify::logo-mode",
			G_CALLBACK (property_notify_cb_logo_mode), idol);
	g_signal_connect (G_OBJECT (idol->bvw), "notify::seekable",
			G_CALLBACK (property_notify_cb_seekable), idol);
	update_volume_sliders (idol);
}
