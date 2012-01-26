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

#ifndef IDOL_SCREENSHOT_PLUGIN_H
#define IDOL_SCREENSHOT_PLUGIN_H

#include <glib.h>

#include "idol.h"
#include "idol-plugin.h"

G_BEGIN_DECLS

#define IDOL_TYPE_SCREENSHOT_PLUGIN		(idol_screenshot_plugin_get_type ())
#define IDOL_SCREENSHOT_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_SCREENSHOT_PLUGIN, IdolScreenshotPlugin))
#define IDOL_SCREENSHOT_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_SCREENSHOT_PLUGIN, IdolScreenshotPluginClass))
#define IDOL_IS_SCREENSHOT_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_SCREENSHOT_PLUGIN))
#define IDOL_IS_SCREENSHOT_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_SCREENSHOT_PLUGIN))
#define IDOL_SCREENSHOT_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_SCREENSHOT_PLUGIN, IdolScreenshotPluginClass))

typedef struct IdolScreenshotPluginPrivate	IdolScreenshotPluginPrivate;

typedef struct {
	IdolPlugin parent;
	IdolScreenshotPluginPrivate *priv;
} IdolScreenshotPlugin;

typedef struct {
	IdolPluginClass parent_class;
} IdolScreenshotPluginClass;

GType idol_screenshot_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT GType register_idol_plugin (GTypeModule *module);

gchar *idol_screenshot_plugin_setup_file_chooser (const char *filename_format, const char *movie_name) G_GNUC_WARN_UNUSED_RESULT;
void idol_screenshot_plugin_update_file_chooser (const char *filename);

G_END_DECLS

#endif /* !IDOL_SCREENSHOT_PLUGIN_H */
