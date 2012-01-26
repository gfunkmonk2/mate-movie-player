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
 * Monday 7th February 2005: Christian Schaller: Add excemption clause.
 * See license_change file for details.
 *
 * Author: Philip Withnall <philip@tecnocode.co.uk>
 */

#ifndef IDOL_TIME_ENTRY_H
#define IDOL_TIME_ENTRY_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IDOL_TYPE_TIME_ENTRY		(idol_time_entry_get_type ())
#define IDOL_TIME_ENTRY(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), IDOL_TYPE_TIME_ENTRY, IdolTimeEntry))
#define IDOL_TIME_ENTRY_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), IDOL_TYPE_TIME_ENTRY, IdolTimeEntryClass))
#define IDOL_IS_TIME_ENTRY(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), IDOL_TYPE_TIME_ENTRY))
#define IDOL_IS_TIME_ENTRY_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), IDOL_TYPE_TIME_ENTRY))
#define IDOL_TIME_ENTRY_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), IDOL_TYPE_TIME_ENTRY, IdolTimeEntryClass))

typedef struct {
	GtkSpinButton parent;
} IdolTimeEntry;

typedef struct {
	GtkSpinButtonClass parent;
} IdolTimeEntryClass;

GType idol_time_entry_get_type (void);
GtkWidget *idol_time_entry_new (GtkAdjustment *adjustment, gdouble climb_rate);

G_END_DECLS

#endif /* !IDOL_TIME_ENTRY_H */
