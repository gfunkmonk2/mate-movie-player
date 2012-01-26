/*
 * Copyright (C) 2003  Andrew Sobala <aes@mate.org>
 * Copyright (C) 2005  Bastien Nocera <hadess@hadess.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * The Idol project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Idol. This
 * permission are above and beyond the permissions granted by the GPL license
 * Idol is covered by.
 *
 * Monday 7th February 2005: Christian Schaller: Add exception clause.
 * See license_change file for details.
 *
 */

#ifndef IDOL_PROPERTIES_VIEW_H
#define IDOL_PROPERTIES_VIEW_H

#include <gtk/gtk.h>

#define IDOL_TYPE_PROPERTIES_VIEW	    (idol_properties_view_get_type ())
#define IDOL_PROPERTIES_VIEW(obj)	    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_PROPERTIES_VIEW, IdolPropertiesView))
#define IDOL_PROPERTIES_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_PROPERTIES_VIEW, IdolPropertiesViewClass))
#define IDOL_IS_PROPERTIES_VIEW(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_PROPERTIES_VIEW))
#define IDOL_IS_PROPERTIES_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_PROPERTIES_VIEW))

typedef struct IdolPropertiesViewPriv IdolPropertiesViewPriv;

typedef struct {
	GtkTable parent;
	IdolPropertiesViewPriv *priv;
} IdolPropertiesView;

typedef struct {
	GtkTableClass parent;
} IdolPropertiesViewClass;

GType      idol_properties_view_get_type      (void);
void       idol_properties_view_register_type (GTypeModule *module);

GtkWidget *idol_properties_view_new           (const char *location,
						GtkWidget  *label);
void       idol_properties_view_set_location  (IdolPropertiesView *view,
						 const char         *location);

#endif /* IDOL_PROPERTIES_VIEW_H */
