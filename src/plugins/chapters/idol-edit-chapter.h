/*
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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
 *
 * The Idol project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Idol. This
 * permission are above and beyond the permissions granted by the GPL license
 * Idol is covered by.
 */

#ifndef IDOL_EDIT_CHAPTER_H
#define IDOL_EDIT_CHAPTER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IDOL_TYPE_EDIT_CHAPTER			(idol_edit_chapter_get_type ())
#define IDOL_EDIT_CHAPTER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_EDIT_CHAPTER, IdolEditChapter))
#define IDOL_EDIT_CHAPTER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_EDIT_CHAPTER, IdolEditChapterClass))
#define IDOL_IS_EDIT_CHAPTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_EDIT_CHAPTER))
#define IDOL_IS_EDIT_CHAPTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_EDIT_CHAPTER))

typedef struct IdolEditChapter			IdolEditChapter;
typedef struct IdolEditChapterClass		IdolEditChapterClass;
typedef struct IdolEditChapterPrivate		IdolEditChapterPrivate;

struct IdolEditChapter {
	GtkDialog parent;
	IdolEditChapterPrivate *priv;
};

struct IdolEditChapterClass {
	GtkDialogClass parent_class;
};

GType idol_edit_chapter_get_type (void);
GtkWidget * idol_edit_chapter_new (void);
void idol_edit_chapter_set_title (IdolEditChapter *edit_chapter, const gchar *title);
gchar * idol_edit_chapter_get_title (IdolEditChapter *edit_chapter);

G_END_DECLS

#endif /* IDOL_EDIT_CHAPTER_H */
