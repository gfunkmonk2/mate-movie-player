/* idol-playlist.h: Simple playlist dialog

   Copyright (C) 2002, 2003, 2004, 2005 Bastien Nocera <hadess@hadess.net>

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301  USA.

   Author: Bastien Nocera <hadess@hadess.net>
 */

#ifndef IDOL_PLAYLIST_H
#define IDOL_PLAYLIST_H

#include <gtk/gtk.h>
#include <idol-pl-parser.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define IDOL_TYPE_PLAYLIST            (idol_playlist_get_type ())
#define IDOL_PLAYLIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_PLAYLIST, IdolPlaylist))
#define IDOL_PLAYLIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_PLAYLIST, IdolPlaylistClass))
#define IDOL_IS_PLAYLIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_PLAYLIST))
#define IDOL_IS_PLAYLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_PLAYLIST))

typedef enum {
	IDOL_PLAYLIST_STATUS_NONE,
	IDOL_PLAYLIST_STATUS_PLAYING,
	IDOL_PLAYLIST_STATUS_PAUSED
} IdolPlaylistStatus;

typedef enum {
	IDOL_PLAYLIST_DIRECTION_NEXT,
	IDOL_PLAYLIST_DIRECTION_PREVIOUS
} IdolPlaylistDirection;

typedef enum {
	IDOL_PLAYLIST_DIALOG_SELECTED,
	IDOL_PLAYLIST_DIALOG_PLAYING
} IdolPlaylistSelectDialog;


typedef struct IdolPlaylist	       IdolPlaylist;
typedef struct IdolPlaylistClass      IdolPlaylistClass;
typedef struct IdolPlaylistPrivate    IdolPlaylistPrivate;

typedef void (*IdolPlaylistForeachFunc) (IdolPlaylist *playlist,
					  const gchar   *filename,
					  const gchar   *uri,
					  gpointer       user_data);

struct IdolPlaylist {
	GtkVBox parent;
	IdolPlaylistPrivate *priv;
};

struct IdolPlaylistClass {
	GtkVBoxClass parent_class;

	void (*changed) (IdolPlaylist *playlist);
	void (*item_activated) (IdolPlaylist *playlist);
	void (*active_name_changed) (IdolPlaylist *playlist);
	void (*current_removed) (IdolPlaylist *playlist);
	void (*repeat_toggled) (IdolPlaylist *playlist, gboolean repeat);
	void (*shuffle_toggled) (IdolPlaylist *playlist, gboolean toggled);
	void (*subtitle_changed) (IdolPlaylist *playlist);
	void (*item_added) (IdolPlaylist *playlist, const gchar *filename, const gchar *uri);
	void (*item_removed) (IdolPlaylist *playlist, const gchar *filename, const gchar *uri);
};

GType    idol_playlist_get_type (void);
GtkWidget *idol_playlist_new      (void);

/* The application is responsible for checking that the mrl is correct
 * @display_name is if you have a preferred display string for the mrl,
 * NULL otherwise
 */
void idol_playlist_add_mrl (IdolPlaylist *playlist,
                             const char *mrl,
                             const char *display_name,
                             gboolean cursor,
                             GCancellable *cancellable,
                             GAsyncReadyCallback callback,
                             gpointer user_data);
gboolean idol_playlist_add_mrl_finish (IdolPlaylist *playlist,
                                        GAsyncResult *result);
gboolean idol_playlist_add_mrl_sync (IdolPlaylist *playlist,
                                      const char *mrl,
                                      const char *display_name);

void idol_playlist_save_current_playlist (IdolPlaylist *playlist,
					   const char *output);
void idol_playlist_save_current_playlist_ext (IdolPlaylist *playlist,
					   const char *output, IdolPlParserType type);
void idol_playlist_select_subtitle_dialog (IdolPlaylist *playlist,
					    IdolPlaylistSelectDialog mode);

/* idol_playlist_clear doesn't emit the current_removed signal, even if it does
 * because the caller should know what to do after it's done with clearing */
gboolean   idol_playlist_clear (IdolPlaylist *playlist);
void       idol_playlist_clear_with_g_mount (IdolPlaylist *playlist,
					      GMount *mount);
char      *idol_playlist_get_current_mrl (IdolPlaylist *playlist,
					   char **subtitle);
char      *idol_playlist_get_current_title (IdolPlaylist *playlist,
					     gboolean *custom);
char      *idol_playlist_get_title (IdolPlaylist *playlist,
				     guint title_index);

gboolean   idol_playlist_set_title (IdolPlaylist *playlist,
				     const char *title,
				     gboolean force);
void       idol_playlist_set_current_subtitle (IdolPlaylist *playlist,
						const char *subtitle_uri);

#define    idol_playlist_has_direction(playlist, direction) (direction == IDOL_PLAYLIST_DIRECTION_NEXT ? idol_playlist_has_next_mrl (playlist) : idol_playlist_has_previous_mrl (playlist))
gboolean   idol_playlist_has_previous_mrl (IdolPlaylist *playlist);
gboolean   idol_playlist_has_next_mrl (IdolPlaylist *playlist);

#define    idol_playlist_set_direction(playlist, direction) (direction == IDOL_PLAYLIST_DIRECTION_NEXT ? idol_playlist_set_next (playlist) : idol_playlist_set_previous (playlist))
void       idol_playlist_set_previous (IdolPlaylist *playlist);
void       idol_playlist_set_next (IdolPlaylist *playlist);

gboolean   idol_playlist_get_repeat (IdolPlaylist *playlist);
void       idol_playlist_set_repeat (IdolPlaylist *playlist, gboolean repeat);

gboolean   idol_playlist_get_shuffle (IdolPlaylist *playlist);
void       idol_playlist_set_shuffle (IdolPlaylist *playlist,
				       gboolean shuffle);

gboolean   idol_playlist_set_playing (IdolPlaylist *playlist, IdolPlaylistStatus state);
IdolPlaylistStatus idol_playlist_get_playing (IdolPlaylist *playlist);

void       idol_playlist_set_at_start (IdolPlaylist *playlist);
void       idol_playlist_set_at_end (IdolPlaylist *playlist);

int        idol_playlist_get_current (IdolPlaylist *playlist);
int        idol_playlist_get_last (IdolPlaylist *playlist);
void       idol_playlist_set_current (IdolPlaylist *playlist, guint current_index);

void       idol_playlist_foreach (IdolPlaylist *playlist,
				   IdolPlaylistForeachFunc callback,
				   gpointer user_data);

G_END_DECLS

#endif /* IDOL_PLAYLIST_H */
