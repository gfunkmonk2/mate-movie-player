/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
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
 * Monday 7th February 2005: Christian Schaller: Add excemption clause.
 * See license_change file for details.
 *
 * Author: Bastien Nocera <hadess@hadess.net>, Philip Withnall <philip@tecnocode.co.uk>
 */

#ifndef IDOL_SKIPTO_H
#define IDOL_SKIPTO_H

#include <gtk/gtk.h>

#include "idol.h"
#include "idol-skipto-plugin.h"

G_BEGIN_DECLS

#define IDOL_TYPE_SKIPTO		(idol_skipto_get_type ())
#define IDOL_SKIPTO(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_SKIPTO, IdolSkipto))
#define IDOL_SKIPTO_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_SKIPTO, IdolSkiptoClass))
#define IDOL_IS_SKIPTO(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_SKIPTO))
#define IDOL_IS_SKIPTO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_SKIPTO))

GType idol_skipto_register_type	(GTypeModule *module);

typedef struct IdolSkipto		IdolSkipto;
typedef struct IdolSkiptoClass		IdolSkiptoClass;
typedef struct IdolSkiptoPrivate	IdolSkiptoPrivate;

struct IdolSkipto {
	GtkDialog parent;
	IdolSkiptoPrivate *priv;
};

struct IdolSkiptoClass {
	GtkDialogClass parent_class;
};

GType idol_skipto_get_type	(void);
GtkWidget *idol_skipto_new	(IdolSkiptoPlugin *plugin);
gint64 idol_skipto_get_range	(IdolSkipto *skipto);
void idol_skipto_update_range	(IdolSkipto *skipto, gint64 _time);
void idol_skipto_set_seekable	(IdolSkipto *skipto, gboolean seekable);
void idol_skipto_set_current	(IdolSkipto *skipto, gint64 _time);

G_END_DECLS

#endif /* IDOL_SKIPTO_H */
