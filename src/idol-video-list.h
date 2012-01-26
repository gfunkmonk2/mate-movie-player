/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2001-2007 Philip Withnall <philip@tecnocode.co.uk>
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

#include <gtk/gtk.h>
#include <glib.h>

#ifndef IDOL_VIDEOLIST_H
#define IDOL_VIDEOLIST_H

G_BEGIN_DECLS

#define IDOL_TYPE_VIDEO_LIST		(idol_video_list_get_type ())
#define IDOL_VIDEO_LIST(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_VIDEO_LIST, IdolVideoList))
#define IDOL_VIDEO_LIST_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_VIDEO_LIST, IdolVideoListClass))
#define IDOL_IS_VIDEO_LIST(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_VIDEO_LIST))
#define IDOL_IS_VIDEO_LIST_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_VIDEO_LIST))
#define IDOL_VIDEO_LIST_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_VIDEO_LIST, IdolVideoListClass))

typedef struct _IdolVideoListPrivate	IdolVideoListPrivate;

/**
 * IdolVideoList:
 *
 * All the fields in the #IdolVideoList structure are private and should never be accessed directly.
 **/
typedef struct {
	GtkTreeView parent;
	IdolVideoListPrivate *priv;
} IdolVideoList;

/**
 * IdolVideoListClass:
 * @parent: the parent class
 * @starting_video: the generic signal handler for the #IdolVideoList::starting-video signal,
 * which can be overridden by inheriting classes
 *
 * The class structure for the #IdolVideoList type.
 **/
typedef struct {
	GtkTreeViewClass parent;
	gboolean (*starting_video) (IdolVideoList *video_list, GtkTreePath *path);
} IdolVideoListClass;

GType idol_video_list_get_type (void);
IdolVideoList *idol_video_list_new (void);
GtkUIManager *idol_video_list_get_ui_manager (IdolVideoList *self);

G_END_DECLS

#endif /* IDOL_VIDEOLIST_H */
