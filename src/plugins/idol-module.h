/*
 * This is a based on rb-module.h from Rhythmbox, which is based on 
 * pluma-module.h from pluma, which is based on Epiphany source code.
 *
 * Copyright (C) 2003 Marco Pesenti Gritti
 * Copyright (C) 2003, 2004 Christian Persch
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2007 - Bastien Nocera <hadess@hadess.net>
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

#ifndef IDOL_MODULE_H
#define IDOL_MODULE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define IDOL_TYPE_MODULE		(idol_module_get_type ())
#define IDOL_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_MODULE, IdolModule))
#define IDOL_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_MODULE, IdolModuleClass))
#define IDOL_IS_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_MODULE))
#define IDOL_IS_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), IDOL_TYPE_MODULE))
#define IDOL_MODULE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), IDOL_TYPE_MODULE, IdolModuleClass))

typedef struct _IdolModule	IdolModule;

GType		 idol_module_get_type		(void) G_GNUC_CONST;;

IdolModule	*idol_module_new		(const gchar *path, const char *module);

const gchar	*idol_module_get_path		(IdolModule *module);

GObject		*idol_module_new_object	(IdolModule *module);

G_END_DECLS

#endif
