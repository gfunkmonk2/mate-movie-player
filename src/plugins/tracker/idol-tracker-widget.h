/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-
 *
 * Copyright (C) 2007 Javier Goday <jgoday@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Author : Javier Goday <jgoday@gmail.com>
 */
#ifndef __IDOL_TRACKER_WIDGET_H__
#define __IDOL_TRACKER_WIDGET_H__

#include "idol.h"

#include <gtk/gtk.h>
#include <libtracker-client/tracker-client.h>

#define IDOL_TYPE_TRACKER_WIDGET               (idol_tracker_widget_get_type ())
#define IDOL_TRACKER_WIDGET(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_TRACKER_WIDGET, IdolTrackerWidget))
#define IDOL_TRACKER_WIDGET_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_TRACKER_WIDGET, IdolTrackerWidgetClass))
#define IDOL_IS_TRACKER_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_TRACKER_WIDGET))
#define IDOL_IS_TRACKER_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_TRACKER_WIDGET))
#define IDOL_TRACKER_WIDGET_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), IDOL_TYPE_TRACKER_WIDGET, IdolTrackerWidgetClass))

typedef struct IdolTrackerWidgetPrivate  IdolTrackerWidgetPrivate;

typedef struct IdolTrackerWidget {
	GtkEventBox parent;
	
	IdolObject *idol;
	IdolTrackerWidgetPrivate *priv;
} IdolTrackerWidget;

typedef struct {
	GtkEventBoxClass parent_class;
} IdolTrackerWidgetClass;

GType       idol_tracker_widget_get_type   (void);
GtkWidget*  idol_tracker_widget_new        (IdolObject *idol); 

#endif /* __IDOL_TRACKER_WIDGET_H__ */
