/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Sunil Mohan Adapa <sunilmohan@gnu.org.in>
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
 */

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "bacon-video-widget.h"

#define IDOL_TYPE_FULLSCREEN            (idol_fullscreen_get_type ())
#define IDOL_FULLSCREEN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                          IDOL_TYPE_FULLSCREEN, \
                                          IdolFullscreen))
#define IDOL_FULLSCREEN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                          IDOL_TYPE_FULLSCREEN, \
                                          IdolFullscreenClass))
#define IDOL_IS_FULLSCREEN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                          IDOL_TYPE_FULLSCREEN))
#define IDOL_IS_FULLSCREEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                          IDOL_TYPE_FULLSCREEN))

typedef struct IdolFullscreen IdolFullscreen;
typedef struct IdolFullscreenClass IdolFullscreenClass;
typedef struct _IdolFullscreenPrivate IdolFullscreenPrivate;

struct IdolFullscreen {
	GObject                parent;
	
	/* Public Widgets from popups */
	GtkWidget              *time_label;
	GtkWidget              *seek;
	GtkWidget              *volume;
	GtkWidget              *buttons_box;
	GtkWidget              *exit_button;

	/* Read only */
	gboolean                is_fullscreen;

	/* Private */
	IdolFullscreenPrivate *priv;
};

struct IdolFullscreenClass {
	GObjectClass parent_class;
};

GType    idol_fullscreen_get_type           (void);
IdolFullscreen * idol_fullscreen_new       (GtkWindow *toplevel_window);
void     idol_fullscreen_set_video_widget   (IdolFullscreen *fs,
					      BaconVideoWidget *bvw);
void     idol_fullscreen_set_parent_window  (IdolFullscreen *fs,
					      GtkWindow *parent_window);
void     idol_fullscreen_show_popups        (IdolFullscreen *fs,
					      gboolean show_cursor);
void idol_fullscreen_show_popups_or_osd (IdolFullscreen *fs,
					  const char *icon_name,
					  gboolean show_cursor);
gboolean idol_fullscreen_is_fullscreen      (IdolFullscreen *fs);
void     idol_fullscreen_set_fullscreen     (IdolFullscreen *fs,
					      gboolean fullscreen);
void     idol_fullscreen_set_title          (IdolFullscreen *fs,
					      const char *title);
void     idol_fullscreen_set_seekable       (IdolFullscreen *fs,
					      gboolean seekable);
void     idol_fullscreen_set_can_set_volume (IdolFullscreen *fs,
					      gboolean can_set_volume);
