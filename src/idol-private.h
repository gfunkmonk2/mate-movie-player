/* 
 * Copyright (C) 2001-2002 Bastien Nocera <hadess@hadess.net>
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

#ifndef __IDOL_PRIVATE_H__
#define __IDOL_PRIVATE_H__

#include <mateconf/mateconf-client.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <unique/uniqueapp.h>

#include "idol-playlist.h"
#include "bacon-video-widget.h"
#include "idol-open-location.h"
#include "idol-fullscreen.h"

#define idol_signal_block_by_data(obj, data) (g_signal_handlers_block_matched (obj, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, data))
#define idol_signal_unblock_by_data(obj, data) (g_signal_handlers_unblock_matched (obj, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, data))

#define idol_set_sensitivity(xml, name, state)					\
	{									\
		GtkWidget *widget;						\
		widget = GTK_WIDGET (gtk_builder_get_object (xml, name));	\
		gtk_widget_set_sensitive (widget, state);			\
	}
#define idol_main_set_sensitivity(name, state) idol_set_sensitivity (idol->xml, name, state)

#define idol_action_set_sensitivity(name, state)					\
	{										\
		GtkAction *__action;							\
		__action = gtk_action_group_get_action (idol->main_action_group, name);\
		gtk_action_set_sensitive (__action, state);				\
	}

typedef enum {
	IDOL_CONTROLS_UNDEFINED,
	IDOL_CONTROLS_VISIBLE,
	IDOL_CONTROLS_HIDDEN,
	IDOL_CONTROLS_FULLSCREEN
} ControlsVisibility;

typedef enum {
	STATE_PLAYING,
	STATE_PAUSED,
	STATE_STOPPED
} IdolStates;

struct _IdolObject {
	GObject parent;

	/* Control window */
	GtkBuilder *xml;
	GtkWidget *win;
	BaconVideoWidget *bvw;
	GtkWidget *prefs;
	GtkWidget *statusbar;

	/* UI manager */
	GtkActionGroup *main_action_group;
	GtkActionGroup *zoom_action_group;
	GtkUIManager *ui_manager;

	GtkActionGroup *devices_action_group;
	guint devices_ui_id;

	GtkActionGroup *languages_action_group;
	guint languages_ui_id;

	GtkActionGroup *subtitles_action_group;
	guint subtitles_ui_id;

	/* Plugins */
	GtkWidget *plugins;

	/* Sidebar */
	GtkWidget *sidebar;
	gboolean sidebar_shown;
	int sidebar_w;

	/* Play/Pause */
	GtkWidget *pp_button;
	/* fullscreen Play/Pause */
	GtkWidget *fs_pp_button;

	/* Seek */
	GtkWidget *seek;
	GtkAdjustment *seekadj;
	gboolean seek_lock;
	gboolean seekable;

	/* Volume */
	GtkWidget *volume;
	gboolean volume_sensitive;
	gboolean muted;
	double prev_volume;

	/* Subtitles/Languages menus */
	GtkWidget *subtitles;
	GtkWidget *languages;
	GList *subtitles_list;
	GList *language_list;
	gboolean autoload_subs;

	/* Fullscreen */
	IdolFullscreen *fs;

	/* controls management */
	ControlsVisibility controls_visibility;

	/* Stream info */
	gint64 stream_length;

	/* recent file stuff */
	GtkRecentManager *recent_manager;
	GtkActionGroup *recent_action_group;
	guint recent_ui_id;

	/* Monitor for playlist unmounts and drives/volumes monitoring */
	GVolumeMonitor *monitor;
	gboolean drives_changed;

	/* session */
	const char *argv0;
	gint64 seek_to_start;
	guint index;
	gboolean session_restored;

	/* Window State */
	int window_w, window_h;
	gboolean maximised;

	/* other */
	char *mrl;
	gint64 seek_to;
	IdolPlaylist *playlist;
	MateConfClient *gc;
	UniqueApp *app;
	IdolStates state;
	IdolOpenLocation *open_location;
	gboolean remember_position;
	gboolean disable_kbd_shortcuts;
};

GtkWidget *idol_volume_create (void);

#define SEEK_FORWARD_OFFSET 60
#define SEEK_BACKWARD_OFFSET -15

#define VOLUME_DOWN_OFFSET (-0.08)
#define VOLUME_UP_OFFSET (0.08)

#define ZOOM_IN_OFFSET 0.01
#define ZOOM_OUT_OFFSET -0.01

void	idol_action_open			(Idol *idol);
void	idol_action_open_location		(Idol *idol);
void	idol_action_eject			(Idol *idol);
void	idol_action_zoom_relative		(Idol *idol, double off_pct);
void	idol_action_zoom_reset			(Idol *idol);
void	idol_action_show_help			(Idol *idol);
void	idol_action_show_properties		(Idol *idol);
gboolean idol_action_open_files		(Idol *idol, char **list);
G_GNUC_NORETURN void idol_action_error_and_exit (const char *title, const char *reason, Idol *idol);

void	show_controls				(Idol *idol, gboolean was_fullscreen);

char	*idol_setup_window			(Idol *idol);
void	idol_callback_connect			(Idol *idol);
void	playlist_widget_setup			(Idol *idol);
void	video_widget_create			(Idol *idol);

#endif /* __IDOL_PRIVATE_H__ */
