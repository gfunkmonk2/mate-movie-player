/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2004 Bastien Nocera
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

#ifndef IDOL_SCREENSHOT_H
#define IDOL_SCREENSHOT_H

#include <gtk/gtk.h>
#include "idol-plugin.h"

G_BEGIN_DECLS

#define IDOL_TYPE_SCREENSHOT			(idol_screenshot_get_type ())
#define IDOL_SCREENSHOT(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_SCREENSHOT, IdolScreenshot))
#define IDOL_SCREENSHOT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_SCREENSHOT, IdolScreenshotClass))
#define IDOL_IS_SCREENSHOT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_SCREENSHOT))
#define IDOL_IS_SCREENSHOT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_SCREENSHOT))

typedef struct IdolScreenshot			IdolScreenshot;
typedef struct IdolScreenshotClass		IdolScreenshotClass;
typedef struct IdolScreenshotPrivate		IdolScreenshotPrivate;

struct IdolScreenshot {
	GtkDialog parent;
	IdolScreenshotPrivate *priv;
};

struct IdolScreenshotClass {
	GtkDialogClass parent_class;
};

GType idol_screenshot_get_type (void) G_GNUC_CONST;
GtkWidget *idol_screenshot_new (Idol *idol, IdolPlugin *screenshot_plugin, GdkPixbuf *screen_image) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !IDOL_SCREENSHOT_H */
