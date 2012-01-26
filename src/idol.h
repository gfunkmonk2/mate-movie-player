/* 
 * Copyright (C) 2001,2002,2003,2004,2005 Bastien Nocera <hadess@hadess.net>
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

#ifndef __IDOL_H__
#define __IDOL_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <idol-disc.h>
#include "idol-playlist.h"

/**
 * IDOL_MATECONF_PREFIX:
 *
 * The MateConf prefix under which all Idol MateConf keys are stored.
 **/
#define IDOL_MATECONF_PREFIX "/apps/idol"

G_BEGIN_DECLS

/**
 * IdolRemoteCommand:
 * @IDOL_REMOTE_COMMAND_UNKNOWN: unknown command
 * @IDOL_REMOTE_COMMAND_PLAY: play the current stream
 * @IDOL_REMOTE_COMMAND_PAUSE: pause the current stream
 * @IDOL_REMOTE_COMMAND_STOP: stop playing the current stream
 * @IDOL_REMOTE_COMMAND_PLAYPAUSE: toggle play/pause on the current stream
 * @IDOL_REMOTE_COMMAND_NEXT: play the next playlist item
 * @IDOL_REMOTE_COMMAND_PREVIOUS: play the previous playlist item
 * @IDOL_REMOTE_COMMAND_SEEK_FORWARD: seek forwards in the current stream
 * @IDOL_REMOTE_COMMAND_SEEK_BACKWARD: seek backwards in the current stream
 * @IDOL_REMOTE_COMMAND_VOLUME_UP: increase the volume
 * @IDOL_REMOTE_COMMAND_VOLUME_DOWN: decrease the volume
 * @IDOL_REMOTE_COMMAND_FULLSCREEN: toggle fullscreen mode
 * @IDOL_REMOTE_COMMAND_QUIT: quit the instance of Idol
 * @IDOL_REMOTE_COMMAND_ENQUEUE: enqueue a new playlist item
 * @IDOL_REMOTE_COMMAND_REPLACE: replace an item in the playlist
 * @IDOL_REMOTE_COMMAND_SHOW: show the Idol instance
 * @IDOL_REMOTE_COMMAND_TOGGLE_CONTROLS: toggle the control visibility
 * @IDOL_REMOTE_COMMAND_UP: go up (DVD controls)
 * @IDOL_REMOTE_COMMAND_DOWN: go down (DVD controls)
 * @IDOL_REMOTE_COMMAND_LEFT: go left (DVD controls)
 * @IDOL_REMOTE_COMMAND_RIGHT: go right (DVD controls)
 * @IDOL_REMOTE_COMMAND_SELECT: select the current item (DVD controls)
 * @IDOL_REMOTE_COMMAND_DVD_MENU: go to the DVD menu
 * @IDOL_REMOTE_COMMAND_ZOOM_UP: increase the zoom level
 * @IDOL_REMOTE_COMMAND_ZOOM_DOWN: decrease the zoom level
 * @IDOL_REMOTE_COMMAND_EJECT: eject the current disc
 * @IDOL_REMOTE_COMMAND_PLAY_DVD: play a DVD in a drive
 * @IDOL_REMOTE_COMMAND_MUTE: toggle mute
 * @IDOL_REMOTE_COMMAND_TOGGLE_ASPECT: toggle the aspect ratio
 *
 * Represents a command which can be sent to a running Idol instance remotely.
 **/
typedef enum {
	IDOL_REMOTE_COMMAND_UNKNOWN = 0,
	IDOL_REMOTE_COMMAND_PLAY,
	IDOL_REMOTE_COMMAND_PAUSE,
	IDOL_REMOTE_COMMAND_STOP,
	IDOL_REMOTE_COMMAND_PLAYPAUSE,
	IDOL_REMOTE_COMMAND_NEXT,
	IDOL_REMOTE_COMMAND_PREVIOUS,
	IDOL_REMOTE_COMMAND_SEEK_FORWARD,
	IDOL_REMOTE_COMMAND_SEEK_BACKWARD,
	IDOL_REMOTE_COMMAND_VOLUME_UP,
	IDOL_REMOTE_COMMAND_VOLUME_DOWN,
	IDOL_REMOTE_COMMAND_FULLSCREEN,
	IDOL_REMOTE_COMMAND_QUIT,
	IDOL_REMOTE_COMMAND_ENQUEUE,
	IDOL_REMOTE_COMMAND_REPLACE,
	IDOL_REMOTE_COMMAND_SHOW,
	IDOL_REMOTE_COMMAND_TOGGLE_CONTROLS,
	IDOL_REMOTE_COMMAND_UP,
	IDOL_REMOTE_COMMAND_DOWN,
	IDOL_REMOTE_COMMAND_LEFT,
	IDOL_REMOTE_COMMAND_RIGHT,
	IDOL_REMOTE_COMMAND_SELECT,
	IDOL_REMOTE_COMMAND_DVD_MENU,
	IDOL_REMOTE_COMMAND_ZOOM_UP,
	IDOL_REMOTE_COMMAND_ZOOM_DOWN,
	IDOL_REMOTE_COMMAND_EJECT,
	IDOL_REMOTE_COMMAND_PLAY_DVD,
	IDOL_REMOTE_COMMAND_MUTE,
	IDOL_REMOTE_COMMAND_TOGGLE_ASPECT
} IdolRemoteCommand;

/**
 * IdolRemoteSetting:
 * @IDOL_REMOTE_SETTING_SHUFFLE: whether shuffle is enabled
 * @IDOL_REMOTE_SETTING_REPEAT: whether repeat is enabled
 *
 * Represents a boolean setting or preference on a remote Idol instance.
 **/
typedef enum {
	IDOL_REMOTE_SETTING_SHUFFLE,
	IDOL_REMOTE_SETTING_REPEAT
} IdolRemoteSetting;

GType idol_remote_command_get_type	(void);
GQuark idol_remote_command_quark	(void);
#define IDOL_TYPE_REMOTE_COMMAND	(idol_remote_command_get_type())
#define IDOL_REMOTE_COMMAND		idol_remote_command_quark ()

GType idol_remote_setting_get_type	(void);
GQuark idol_remote_setting_quark	(void);
#define IDOL_TYPE_REMOTE_SETTING	(idol_remote_setting_get_type())
#define IDOL_REMOTE_SETTING		idol_remote_setting_quark ()

#define IDOL_TYPE_OBJECT              (idol_object_get_type ())
#define IDOL_OBJECT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), idol_object_get_type (), IdolObject))
#define IDOL_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), idol_object_get_type (), IdolObjectClass))
#define IDOL_IS_OBJECT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE (obj, idol_object_get_type ()))
#define IDOL_IS_OBJECT_CLASS(klass)   (G_CHECK_INSTANCE_GET_CLASS ((klass), idol_object_get_type ()))

/**
 * Idol:
 *
 * The #Idol object is a handy synonym for #IdolObject, and the two can be used interchangably.
 **/

/**
 * IdolObject:
 *
 * All the fields in the #IdolObject structure are private and should never be accessed directly.
 **/
typedef struct _IdolObject Idol;
typedef struct _IdolObject IdolObject;

/**
 * IdolObjectClass:
 * @parent_class: the parent class
 * @file_opened: the generic signal handler for the #IdolObject::file-opened signal,
 * which can be overridden by inheriting classes
 * @file_closed: the generic signal handler for the #IdolObject::file-closed signal,
 * which can be overridden by inheriting classes
 * @metadata_updated: the generic signal handler for the #IdolObject::metadata-updated signal,
 * which can be overridden by inheriting classes
 *
 * The class structure for the #IdolPlParser type.
 **/
typedef struct {
	GObjectClass parent_class;

	void (*file_opened)			(Idol *idol, const char *mrl);
	void (*file_closed)			(Idol *idol);
	void (*metadata_updated)		(Idol *idol,
						 const char *artist,
						 const char *title,
						 const char *album,
						 guint track_num);
} IdolObjectClass;

GType	idol_object_get_type			(void);
void    idol_object_plugins_init		(IdolObject *idol);
void    idol_object_plugins_shutdown		(void);
void	idol_file_opened			(IdolObject *idol,
						 const char *mrl);
void	idol_file_closed			(IdolObject *idol);
void	idol_metadata_updated			(IdolObject *idol,
						 const char *artist,
						 const char *title,
						 const char *album,
						 guint track_num);

void	idol_action_exit			(Idol *idol) G_GNUC_NORETURN;
void	idol_action_play			(Idol *idol);
void	idol_action_stop			(Idol *idol);
void	idol_action_play_pause			(Idol *idol);
void	idol_action_pause			(Idol *idol);
void	idol_action_fullscreen_toggle		(Idol *idol);
void	idol_action_fullscreen			(Idol *idol, gboolean state);
void	idol_action_next			(Idol *idol);
void	idol_action_previous			(Idol *idol);
void	idol_action_seek_time			(Idol *idol, gint64 msec, gboolean accurate);
void	idol_action_seek_relative		(Idol *idol, gint64 offset, gboolean accurate);
double	idol_get_volume			(Idol *idol);
void	idol_action_volume			(Idol *idol, double volume);
void	idol_action_volume_relative		(Idol *idol, double off_pct);
void	idol_action_volume_toggle_mute		(Idol *idol);
gboolean idol_action_set_mrl			(Idol *idol,
						 const char *mrl,
						 const char *subtitle);
void	idol_action_set_mrl_and_play		(Idol *idol,
						 const char *mrl, 
						 const char *subtitle);

gboolean idol_action_set_mrl_with_warning	(Idol *idol,
						 const char *mrl,
						 const char *subtitle,
						 gboolean warn);

void	idol_action_play_media			(Idol *idol,
						 IdolDiscMediaType type,
						 const char *device);

void	idol_action_toggle_aspect_ratio	(Idol *idol);
void	idol_action_set_aspect_ratio		(Idol *idol, int ratio);
int	idol_action_get_aspect_ratio		(Idol *idol);
void	idol_action_toggle_controls		(Idol *idol);
void	idol_action_next_angle			(Idol *idol);

void	idol_action_set_scale_ratio		(Idol *idol, gfloat ratio);
void    idol_action_error                      (const char *title,
						 const char *reason,
						 Idol *idol);
void    idol_action_play_media_device		(Idol *idol,
						 const char *device);

gboolean idol_is_fullscreen			(Idol *idol);
gboolean idol_is_playing			(Idol *idol);
gboolean idol_is_paused			(Idol *idol);
gboolean idol_is_seekable			(Idol *idol);
GtkWindow *idol_get_main_window		(Idol *idol);
GtkUIManager *idol_get_ui_manager		(Idol *idol);
GtkWidget *idol_get_video_widget		(Idol *idol);
char *idol_get_video_widget_backend_name	(Idol *idol);
char *idol_get_version				(void);

/* Current media information */
char *	idol_get_short_title			(Idol *idol);
gint64	idol_get_current_time			(Idol *idol);

/* Playlist handling */
guint	idol_get_playlist_length		(Idol *idol);
void	idol_action_set_playlist_index		(Idol *idol,
						 guint index);
int	idol_get_playlist_pos			(Idol *idol);
char *	idol_get_title_at_playlist_pos		(Idol *idol,
						 guint playlist_index);
void idol_add_to_playlist_and_play		(Idol *idol,
						 const char *uri,
						 const char *display_name,
						 gboolean add_to_recent);
char *  idol_get_current_mrl			(Idol *idol);
void	idol_set_current_subtitle		(Idol *idol,
						 const char *subtitle_uri);
/* Sidebar handling */
void    idol_add_sidebar_page			(Idol *idol,
						 const char *page_id,
						 const char *title,
						 GtkWidget *main_widget);
void    idol_remove_sidebar_page		(Idol *idol,
						 const char *page_id);

/* Remote actions */
void    idol_action_remote			(Idol *idol,
						 IdolRemoteCommand cmd,
						 const char *url);
void	idol_action_remote_set_setting		(Idol *idol,
						 IdolRemoteSetting setting,
						 gboolean value);
gboolean idol_action_remote_get_setting	(Idol *idol,
						 IdolRemoteSetting setting);

#endif /* __IDOL_H__ */
