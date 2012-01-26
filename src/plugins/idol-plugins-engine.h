/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Plugin engine for Idol, heavily based on the code from Rhythmbox,
 * which is based heavily on the code from pluma.
 *
 * Copyright (C) 2002-2005 Paolo Maggi
 *               2006 James Livingston  <jrl@ids.org.au>
 *               2007 Bastien Nocera <hadess@hadess.net>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301  USA.
 *
 * Sunday 13th May 2007: Bastien Nocera: Add exception clause.
 * See license_change file for details.
 *
 */

#ifndef __IDOL_PLUGINS_ENGINE_H__
#define __IDOL_PLUGINS_ENGINE_H__

#include <glib.h>
#include <idol.h>

typedef struct _IdolPluginInfo IdolPluginInfo;

gboolean	 idol_plugins_engine_init 		(IdolObject *idol);
void		 idol_plugins_engine_shutdown 		(void);

void		 idol_plugins_engine_garbage_collect	(void);

GList*		idol_plugins_engine_get_plugins_list 	(void);

gboolean 	 idol_plugins_engine_activate_plugin 	(IdolPluginInfo *info);
gboolean 	 idol_plugins_engine_deactivate_plugin	(IdolPluginInfo *info);
gboolean 	 idol_plugins_engine_plugin_is_active 	(IdolPluginInfo *info);
gboolean 	 idol_plugins_engine_plugin_is_visible (IdolPluginInfo *info);

gboolean	 idol_plugins_engine_plugin_is_configurable
							(IdolPluginInfo *info);
void	 	 idol_plugins_engine_configure_plugin	(IdolPluginInfo *info,
							 GtkWindow *parent);

const gchar*	idol_plugins_engine_get_plugin_name	(IdolPluginInfo *info);
const gchar*	idol_plugins_engine_get_plugin_description
							(IdolPluginInfo *info);

const gchar**	idol_plugins_engine_get_plugin_authors	(IdolPluginInfo *info);
const gchar*	idol_plugins_engine_get_plugin_website	(IdolPluginInfo *info);
const gchar*	idol_plugins_engine_get_plugin_copyright
							(IdolPluginInfo *info);
GdkPixbuf *	idol_plugins_engine_get_plugin_icon	(IdolPluginInfo *info);

#endif  /* __IDOL_PLUGINS_ENGINE_H__ */
