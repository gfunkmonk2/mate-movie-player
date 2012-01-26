/*
 * This is a based on rb-module.c from Rhythmbox, which is based on 
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

#include "config.h"

#include "idol-module.h"

#include <gmodule.h>

typedef struct _IdolModuleClass IdolModuleClass;

struct _IdolModuleClass
{
	GTypeModuleClass parent_class;
};

struct _IdolModule
{
	GTypeModule parent_instance;

	GModule *library;

	gchar *path;
	gchar *name;
	GType type;
};

typedef GType (*IdolModuleRegisterFunc) (GTypeModule *);

static GObjectClass *parent_class = NULL;

G_DEFINE_TYPE (IdolModule, idol_module, G_TYPE_TYPE_MODULE)

static gboolean
idol_module_load (GTypeModule *gmodule)
{
	IdolModule *module = IDOL_MODULE (gmodule);
	IdolModuleRegisterFunc register_func;

	module->library = g_module_open (module->path, 0);

	if (module->library == NULL) {
		g_warning ("%s", g_module_error());
		return FALSE;
	}

	/* extract symbols from the lib */
	if (!g_module_symbol (module->library, "register_idol_plugin", (void *)&register_func)) {
		g_warning ("%s", g_module_error ());
		g_module_close (module->library);
		return FALSE;
	}

	g_assert (register_func);

	module->type = register_func (gmodule);
	if (module->type == 0) {
		g_warning ("Invalid idol plugin contained by module %s", module->path);
		return FALSE;
	}

	return TRUE;
}

static void
idol_module_unload (GTypeModule *gmodule)
{
	IdolModule *module = IDOL_MODULE (gmodule);

	g_module_close (module->library);

	module->library = NULL;
	module->type = 0;
}

const gchar *
idol_module_get_path (IdolModule *module)
{
	g_return_val_if_fail (IDOL_IS_MODULE (module), NULL);

	return module->path;
}

GObject *
idol_module_new_object (IdolModule *module)
{
	GObject *obj;

	if (module->type == 0) {
		return NULL;
	}

	obj = g_object_new (module->type,
			    "name", module->name,
			    NULL);
	return obj;
}

static void
idol_module_init (IdolModule *module)
{

}

static void
idol_module_finalize (GObject *object)
{
	IdolModule *module = IDOL_MODULE (object);

	g_free (module->path);
	g_free (module->name);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
idol_module_class_init (IdolModuleClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (class);

	parent_class = (GObjectClass *) g_type_class_peek_parent (class);

	object_class->finalize = idol_module_finalize;

	module_class->load = idol_module_load;
	module_class->unload = idol_module_unload;
}

IdolModule *
idol_module_new (const gchar *path, const char *module)
{
	IdolModule *result;

	if (path == NULL || path[0] == '\0') {
		return NULL;
	}

	result = g_object_new (IDOL_TYPE_MODULE, NULL);

	g_type_module_set_name (G_TYPE_MODULE (result), path);
	result->path = g_strdup (path);
	result->name = g_strdup (module);

	return result;
}
