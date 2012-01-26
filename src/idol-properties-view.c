/*
 * Copyright (C) 2003  Andrew Sobala <aes@mate.org>
 * Copyright (C) 2004  Bastien Nocera <hadess@hadess.net>
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

#include <config.h>

#include "idol-properties-view.h"

#include "bacon-video-widget-properties.h"
#include "bacon-video-widget.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

struct IdolPropertiesViewPriv {
	GtkWidget *label;
	GtkWidget *vbox;
	BaconVideoWidgetProperties *props;
	BaconVideoWidget *bvw;
};

static GObjectClass *parent_class = NULL;
static void idol_properties_view_finalize (GObject *object);

G_DEFINE_TYPE (IdolPropertiesView, idol_properties_view, GTK_TYPE_TABLE)

void
idol_properties_view_register_type (GTypeModule *module)
{
	idol_properties_view_get_type ();
}

static void
idol_properties_view_class_init (IdolPropertiesViewClass *class)
{
	parent_class = g_type_class_peek_parent (class);
	G_OBJECT_CLASS (class)->finalize = idol_properties_view_finalize;
}

static void
on_got_metadata_event (BaconVideoWidget *bvw, IdolPropertiesView *props)
{
	GValue value = { 0, };
	gboolean has_audio, has_video;
	const char *label = NULL;

	bacon_video_widget_get_metadata (BACON_VIDEO_WIDGET (bvw),
			BVW_INFO_HAS_VIDEO, &value);
	has_video = g_value_get_boolean (&value);
	g_value_unset (&value);

	bacon_video_widget_get_metadata (BACON_VIDEO_WIDGET (bvw),
			BVW_INFO_HAS_AUDIO, &value);
	has_audio = g_value_get_boolean (&value);
	g_value_unset (&value);

	if (has_audio == FALSE) {
		if (has_video == FALSE) {
			//FIXME this should be setting an error?
			label = N_("Audio/Video");
		} else {
			label = N_("Video");
		}
	} else {
		if (has_video == FALSE) {
			label = N_("Audio");
		} else {
			label = N_("Audio/Video");
		}
	}

	gtk_label_set_text (GTK_LABEL (props->priv->label), _(label));

	bacon_video_widget_properties_update
		(props->priv->props, GTK_WIDGET (props->priv->bvw));
}

static void
idol_properties_view_init (IdolPropertiesView *props)
{
	GError *err = NULL;

	props->priv = g_new0 (IdolPropertiesViewPriv, 1);

	props->priv->bvw = BACON_VIDEO_WIDGET (bacon_video_widget_new
			(-1, -1, BVW_USE_TYPE_METADATA, &err));

	if (props->priv->bvw != NULL)
	{
		/* Reference it, so that it's not floating */
		g_object_ref (props->priv->bvw);

		g_signal_connect (G_OBJECT (props->priv->bvw),
				"got-metadata",
				G_CALLBACK (on_got_metadata_event),
				props);
	} else {
		g_warning ("Error: %s", err ? err->message : "bla");
	}

	props->priv->vbox = bacon_video_widget_properties_new ();
	gtk_table_resize (GTK_TABLE (props), 1, 1);
	gtk_container_add (GTK_CONTAINER (props), props->priv->vbox);
	gtk_widget_show (GTK_WIDGET (props));

	props->priv->props = BACON_VIDEO_WIDGET_PROPERTIES (props->priv->vbox);
}

static void
idol_properties_view_finalize (GObject *object)
{
	IdolPropertiesView *props;

	props = IDOL_PROPERTIES_VIEW (object);

	if (props->priv != NULL)
	{
		if (props->priv->bvw != NULL)
			g_object_unref (G_OBJECT (props->priv->bvw));
		if (props->priv->label != NULL)
		g_object_unref (G_OBJECT (props->priv->label));
		props->priv->bvw = NULL;
		props->priv->label = NULL;
		g_free (props->priv);
	}
	props->priv = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
idol_properties_view_new (const char *location, GtkWidget *label)
{
	IdolPropertiesView *self;

	self = g_object_new (IDOL_TYPE_PROPERTIES_VIEW, NULL);
	g_object_ref (label);
	self->priv->label = label;
	idol_properties_view_set_location (self, location);

	return GTK_WIDGET (self);
}

void
idol_properties_view_set_location (IdolPropertiesView *props,
				     const char *location)
{
	g_assert (IDOL_IS_PROPERTIES_VIEW (props));

	if (location != NULL && props->priv->bvw != NULL) {
		GError *error = NULL;

		bacon_video_widget_close (props->priv->bvw);
		bacon_video_widget_properties_reset (props->priv->props);

		if (bacon_video_widget_open (props->priv->bvw, location, NULL, &error) == FALSE) {
			g_warning ("Couldn't open %s: %s", location, error->message);
			g_error_free (error);
			return;
		}

		bacon_video_widget_close (props->priv->bvw);
	} else {
		if (props->priv->bvw != NULL)
			bacon_video_widget_close (props->priv->bvw);
		bacon_video_widget_properties_reset (props->priv->props);
	}
}
