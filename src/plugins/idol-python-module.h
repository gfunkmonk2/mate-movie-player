/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Heavily based on code from Rhythmbox and Pluma.
 *
 * Copyright (C) 2005 Raphael Slinckx
 * Copyright (C) 2007 Philip Withnall
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
 * Saturday 19th May 2007: Philip Withnall: Add exception clause.
 * See license_change file for details.
 */

#ifndef IDOL_PYTHON_MODULE_H
#define IDOL_PYTHON_MODULE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define IDOL_TYPE_PYTHON_MODULE		(idol_python_module_get_type ())
#define IDOL_PYTHON_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_PYTHON_MODULE, IdolPythonModule))
#define IDOL_PYTHON_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_PYTHON_MODULE, IdolPythonModuleClass))
#define IDOL_IS_PYTHON_MODULE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_PYTHON_MODULE))
#define IDOL_IS_PYTHON_MODULE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), IDOL_TYPE_PYTHON_MODULE))
#define IDOL_PYTHON_MODULE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), IDOL_TYPE_PYTHON_MODULE, IdolPythonModuleClass))

typedef struct
{
	GTypeModuleClass parent_class;
} IdolPythonModuleClass;

typedef struct
{
	GTypeModule parent_instance;
} IdolPythonModule;

GType			idol_python_module_get_type		(void);
IdolPythonModule	*idol_python_module_new		(const gchar* path, const gchar *module);
GObject			*idol_python_module_new_object		(IdolPythonModule *module);

/* --- python utils --- */
void			idol_python_garbage_collect		(void);
void			idol_python_shutdown			(void);

G_END_DECLS

#endif
