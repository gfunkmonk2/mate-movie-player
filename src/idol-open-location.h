/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2007 Philip Withnall <philip@tecnocode.co.uk>
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

#ifndef IDOL_OPEN_LOCATION_H
#define IDOL_OPEN_LOCATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IDOL_TYPE_OPEN_LOCATION		(idol_open_location_get_type ())
#define IDOL_OPEN_LOCATION(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_OPEN_LOCATION, IdolOpenLocation))
#define IDOL_OPEN_LOCATION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_OPEN_LOCATION, IdolOpenLocationClass))
#define IDOL_IS_OPEN_LOCATION(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_OPEN_LOCATION))
#define IDOL_IS_OPEN_LOCATION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_OPEN_LOCATION))

typedef struct IdolOpenLocation		IdolOpenLocation;
typedef struct IdolOpenLocationClass		IdolOpenLocationClass;
typedef struct IdolOpenLocationPrivate		IdolOpenLocationPrivate;

struct IdolOpenLocation {
	GtkDialog parent;
	IdolOpenLocationPrivate *priv;
};

struct IdolOpenLocationClass {
	GtkDialogClass parent_class;
};

GType idol_open_location_get_type		(void);
GtkWidget *idol_open_location_new		(void);
char *idol_open_location_get_uri		(IdolOpenLocation *open_location);

G_END_DECLS

#endif /* IDOL_OPEN_LOCATION_H */
