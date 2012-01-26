/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Javier Goday <jgoday@gmail.com>
 * Based on the sidebar-test idol plugin example 
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
 * See license_change file for details.
 *
 */

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <string.h>

#include "idol-plugin.h"
#include "idol.h"
#include "idol-tracker-widget.h"

#define IDOL_TYPE_TRACKER_PLUGIN		(idol_tracker_plugin_get_type ())
#define IDOL_TRACKER_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_TRACKER_PLUGIN, IdolTrackerPlugin))
#define IDOL_TRACKER_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_TRACKER_PLUGIN, IdolTrackerPluginClass))
#define IDOL_IS_TRACKER_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_TRACKER_PLUGIN))
#define IDOL_IS_TRACKER_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_TRACKER_PLUGIN))
#define IDOL_TRACKER_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_TRACKER_PLUGIN, IdolTrackerPluginClass))

typedef struct
{
	IdolPlugin   parent;
} IdolTrackerPlugin;

typedef struct
{
	IdolPluginClass parent_class;
} IdolTrackerPluginClass;


G_MODULE_EXPORT GType register_idol_plugin		(GTypeModule *module);
GType	idol_tracker_plugin_get_type			(void) G_GNUC_CONST;

static gboolean impl_activate				(IdolPlugin *plugin, IdolObject *idol, GError **error);
static void impl_deactivate				(IdolPlugin *plugin, IdolObject *idol);

IDOL_PLUGIN_REGISTER (IdolTrackerPlugin, idol_tracker_plugin)

static void
idol_tracker_plugin_class_init (IdolTrackerPluginClass *klass)
{
	IdolPluginClass *plugin_class = IDOL_PLUGIN_CLASS (klass);

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
}

static void
idol_tracker_plugin_init (IdolTrackerPlugin *plugin)
{
}

static gboolean
impl_activate (IdolPlugin *plugin,
	       IdolObject *idol,
	       GError **error)
{
	GtkWidget *widget;

	widget = idol_tracker_widget_new (idol);
	gtk_widget_show (widget);
	idol_add_sidebar_page (idol, "tracker", _("Local Search"), widget);

	return TRUE;
}

static void
impl_deactivate	(IdolPlugin *plugin,
		 IdolObject *idol)
{
	idol_remove_sidebar_page (idol, "tracker");
}
