/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2001-2007 Bastien Nocera <hadess@hadess.net>
 * Copyright (C) 2007 Sunil Mohan Adapa <sunilmohan@gnu.org.in>
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
 */

#include <config.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "idol-fullscreen.h"
#include "idol-interface.h"
#include "idol-time-label.h"
#include "bacon-video-widget.h"
#include "gsd-media-keys-window.h"

#define FULLSCREEN_POPUP_TIMEOUT 5

static void idol_fullscreen_dispose (GObject *object);
static void idol_fullscreen_finalize (GObject *object);
static gboolean idol_fullscreen_popup_hide (IdolFullscreen *fs);

/* Callback functions for GtkBuilder */
G_MODULE_EXPORT gboolean idol_fullscreen_vol_slider_pressed_cb (GtkWidget *widget, GdkEventButton *event, IdolFullscreen *fs);
G_MODULE_EXPORT gboolean idol_fullscreen_vol_slider_released_cb (GtkWidget *widget, GdkEventButton *event, IdolFullscreen *fs);
G_MODULE_EXPORT gboolean idol_fullscreen_seek_slider_pressed_cb (GtkWidget *widget, GdkEventButton *event, IdolFullscreen *fs);
G_MODULE_EXPORT gboolean idol_fullscreen_seek_slider_released_cb (GtkWidget *widget, GdkEventButton *event, IdolFullscreen *fs);
G_MODULE_EXPORT gboolean idol_fullscreen_motion_notify (GtkWidget *widget, GdkEventMotion *event, IdolFullscreen *fs);
G_MODULE_EXPORT gboolean idol_fullscreen_control_enter_notify (GtkWidget *widget, GdkEventCrossing *event, IdolFullscreen *fs);
G_MODULE_EXPORT gboolean idol_fullscreen_control_leave_notify (GtkWidget *widget, GdkEventCrossing *event, IdolFullscreen *fs);


struct _IdolFullscreenPrivate {
	BaconVideoWidget *bvw;
	GtkWidget        *parent_window;
	GtkWidget        *osd;

	/* Fullscreen Popups */
	GtkWidget        *exit_popup;
	GtkWidget        *control_popup;

	/* Locks for keeping the popups during adjustments */
	gboolean          seek_lock;

	guint             popup_timeout;
	gboolean          popup_in_progress;
	gboolean          pointer_on_control;
	guint             motion_handler_id;

	GtkBuilder       *xml;
};

#define IDOL_FULLSCREEN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), IDOL_TYPE_FULLSCREEN, IdolFullscreenPrivate))

G_DEFINE_TYPE (IdolFullscreen, idol_fullscreen, G_TYPE_OBJECT)

gboolean
idol_fullscreen_is_fullscreen (IdolFullscreen *fs)
{
	g_return_val_if_fail (IDOL_IS_FULLSCREEN (fs), FALSE);

	return (fs->is_fullscreen != FALSE);
}

static void
idol_fullscreen_move_popups (IdolFullscreen *fs)
{
	int exit_width,    exit_height;
	int control_width, control_height;
	
	GdkScreen              *screen;
	GdkRectangle            fullscreen_rect;
	GdkWindow              *window;
	IdolFullscreenPrivate *priv = fs->priv;

	g_return_if_fail (priv->parent_window != NULL);

	/* Obtain the screen rectangle */
	screen = gtk_window_get_screen (GTK_WINDOW (priv->parent_window));
	window = gtk_widget_get_window (priv->parent_window);
	gdk_screen_get_monitor_geometry (screen,
					 gdk_screen_get_monitor_at_window (screen, window),
					 &fullscreen_rect);

	/* Get the popup window sizes */
	gtk_window_get_size (GTK_WINDOW (priv->exit_popup),
			     &exit_width, &exit_height);
	gtk_window_get_size (GTK_WINDOW (priv->control_popup),
			     &control_width, &control_height);

	/* We take the full width of the screen */
	gtk_window_resize (GTK_WINDOW (priv->control_popup),
			   fullscreen_rect.width, control_height);

	if (gtk_widget_get_direction (priv->exit_popup) == GTK_TEXT_DIR_RTL) {
		gtk_window_move (GTK_WINDOW (priv->exit_popup),
				 fullscreen_rect.x,
				 fullscreen_rect.y);
		gtk_window_move (GTK_WINDOW (priv->control_popup),
				 fullscreen_rect.width - control_width,
				 fullscreen_rect.height + fullscreen_rect.y -
				 control_height);
	} else {
		gtk_window_move (GTK_WINDOW (priv->exit_popup),
				 fullscreen_rect.width + fullscreen_rect.x -
				 exit_width,
				 fullscreen_rect.y);
		gtk_window_move (GTK_WINDOW (priv->control_popup),
				 fullscreen_rect.x,
				 fullscreen_rect.height + fullscreen_rect.y -
				 control_height);
	}
}

static void
idol_fullscreen_size_changed_cb (GdkScreen *screen, IdolFullscreen *fs)
{
	idol_fullscreen_move_popups (fs);
}

static void
idol_fullscreen_theme_changed_cb (GtkIconTheme *icon_theme, IdolFullscreen *fs)
{
	idol_fullscreen_move_popups (fs);
}

static void
idol_fullscreen_composited_changed_cb (GdkScreen *screen, IdolFullscreen *fs)
{
	if (gdk_screen_is_composited (screen)) {
		if (fs->priv->osd == NULL)
			fs->priv->osd = gsd_media_keys_window_new ();
	} else {
		if (fs->priv->osd != NULL) {
			gtk_widget_destroy (fs->priv->osd);
			fs->priv->osd = NULL;
		}
	}
}

static void
idol_fullscreen_window_realize_cb (GtkWidget *widget, IdolFullscreen *fs)
{
	GdkScreen *screen;
	
	screen = gtk_widget_get_screen (widget);
	g_signal_connect (G_OBJECT (screen), "size-changed",
			  G_CALLBACK (idol_fullscreen_size_changed_cb), fs);
	g_signal_connect (G_OBJECT (screen), "composited-changed",
			  G_CALLBACK (idol_fullscreen_composited_changed_cb), fs);
	g_signal_connect (G_OBJECT (gtk_icon_theme_get_for_screen (screen)),
			  "changed",
			  G_CALLBACK (idol_fullscreen_theme_changed_cb), fs);

	idol_fullscreen_composited_changed_cb (screen, fs);
}

static void
idol_fullscreen_window_unrealize_cb (GtkWidget *widget, IdolFullscreen *fs)
{
	GdkScreen *screen;

	screen = gtk_widget_get_screen (widget);
	g_signal_handlers_disconnect_by_func (screen,
					      G_CALLBACK (idol_fullscreen_size_changed_cb), fs);
	g_signal_handlers_disconnect_by_func (gtk_icon_theme_get_for_screen (screen),
					      G_CALLBACK (idol_fullscreen_theme_changed_cb), fs);
}

static gboolean
idol_fullscreen_exit_popup_expose_cb (GtkWidget *widget,
				       GdkEventExpose *event,
				       IdolFullscreen *fs)
{
	GdkScreen *screen;
	cairo_t *cr;

	screen = gtk_widget_get_screen (widget);
	if (gdk_screen_is_composited (screen) == FALSE)
		return FALSE;

	gtk_widget_set_app_paintable (widget, TRUE);

	cr = gdk_cairo_create (gtk_widget_get_window (widget));
	cairo_set_source_rgba (cr, 1., 1., 1., 0.);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	cairo_destroy (cr);

	return FALSE;
}

gboolean
idol_fullscreen_seek_slider_pressed_cb (GtkWidget *widget,
					 GdkEventButton *event,
					 IdolFullscreen *fs)
{
	fs->priv->seek_lock = TRUE;
	return FALSE;
}

gboolean
idol_fullscreen_seek_slider_released_cb (GtkWidget *widget,
					  GdkEventButton *event,
					  IdolFullscreen *fs)
{
	fs->priv->seek_lock = FALSE;
	return FALSE;
}

static void
idol_fullscreen_popup_timeout_add (IdolFullscreen *fs)
{
	fs->priv->popup_timeout = g_timeout_add_seconds (FULLSCREEN_POPUP_TIMEOUT,
							 (GSourceFunc) idol_fullscreen_popup_hide, fs);
}

static void
idol_fullscreen_popup_timeout_remove (IdolFullscreen *fs)
{
	if (fs->priv->popup_timeout != 0) {
		g_source_remove (fs->priv->popup_timeout);
		fs->priv->popup_timeout = 0;
	}
}

static void
idol_fullscreen_set_cursor (IdolFullscreen *fs, gboolean state)
{
	if (fs->priv->bvw != NULL)
		bacon_video_widget_set_show_cursor (fs->priv->bvw, state);
}

static gboolean
idol_fullscreen_is_volume_popup_visible (IdolFullscreen *fs)
{
	return gtk_widget_get_visible (gtk_scale_button_get_popup (GTK_SCALE_BUTTON (fs->volume)));
}

static void
idol_fullscreen_force_popup_hide (IdolFullscreen *fs)
{
	/* Popdown the volume button if it's visible */
	if (idol_fullscreen_is_volume_popup_visible (fs))
		gtk_bindings_activate (GTK_OBJECT (fs->volume), GDK_Escape, 0);

	gtk_widget_hide (fs->priv->exit_popup);
	gtk_widget_hide (fs->priv->control_popup);

	idol_fullscreen_popup_timeout_remove (fs);

	idol_fullscreen_set_cursor (fs, FALSE);
}

static gboolean
idol_fullscreen_popup_hide (IdolFullscreen *fs)
{
	if (fs->priv->bvw == NULL || idol_fullscreen_is_fullscreen (fs) == FALSE)
		return TRUE;

	if (fs->priv->seek_lock != FALSE || idol_fullscreen_is_volume_popup_visible (fs) != FALSE)
		return TRUE;

	idol_fullscreen_force_popup_hide (fs);

	return FALSE;
}

G_MODULE_EXPORT gboolean
idol_fullscreen_motion_notify (GtkWidget *widget, GdkEventMotion *event,
				IdolFullscreen *fs)
{
	if (!fs->priv->pointer_on_control)
		idol_fullscreen_show_popups (fs, TRUE);
	return FALSE;
}

void
idol_fullscreen_show_popups (IdolFullscreen *fs, gboolean show_cursor)
{
	GtkWidget *item;

	g_assert (fs->is_fullscreen != FALSE);

	if (fs->priv->popup_in_progress != FALSE)
		return;

	if (gtk_window_is_active (GTK_WINDOW (fs->priv->parent_window)) == FALSE)
		return;

	fs->priv->popup_in_progress = TRUE;

	idol_fullscreen_popup_timeout_remove (fs);

	/* FIXME: is this really required while we are anyway going 
	   to do a show_all on its parent control_popup? */
	item = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml, "tcw_hbox"));
	gtk_widget_show_all (item);
	gdk_flush ();

	/* Show the popup widgets */
	idol_fullscreen_move_popups (fs);
	gtk_widget_show_all (fs->priv->exit_popup);
	gtk_widget_show_all (fs->priv->control_popup);

	if (show_cursor != FALSE) {
		/* Show the mouse cursor */
		idol_fullscreen_set_cursor (fs, TRUE);
	}

	/* Reset the popup timeout */
	idol_fullscreen_popup_timeout_add (fs);

	fs->priv->popup_in_progress = FALSE;
}

void
idol_fullscreen_show_popups_or_osd (IdolFullscreen *fs,
				     const char *icon_name,
				     gboolean show_cursor)
{
	GtkAllocation allocation;
	GdkScreen *screen;
	GdkWindow *window;
	GdkRectangle rect;
	int monitor;

	if (fs->priv->osd == NULL || icon_name == NULL) {
		idol_fullscreen_show_popups (fs, show_cursor);
		return;
	}

	gtk_widget_get_allocation (GTK_WIDGET (fs->priv->bvw), &allocation);
	gtk_window_resize (GTK_WINDOW (fs->priv->osd),
			   allocation.height / 8,
			   allocation.height / 8);

	window = gtk_widget_get_window (GTK_WIDGET (fs->priv->bvw));
	screen = gtk_widget_get_screen (GTK_WIDGET (fs->priv->bvw));
	monitor = gdk_screen_get_monitor_at_window (screen, window);
	gdk_screen_get_monitor_geometry (screen, monitor, &rect);

	if (gtk_widget_get_direction (GTK_WIDGET (fs->priv->bvw)) == GTK_TEXT_DIR_RTL)
		gtk_window_move (GTK_WINDOW (fs->priv->osd),
				 rect.width - 8 - allocation.height / 8,
				 rect.y + 8);
	else
		gtk_window_move (GTK_WINDOW (fs->priv->osd), rect.x + 8, rect.y + 8);

	gsd_media_keys_window_set_action_custom (GSD_MEDIA_KEYS_WINDOW (fs->priv->osd),
						 icon_name, FALSE);
	gtk_widget_show (fs->priv->osd);
}

G_MODULE_EXPORT gboolean
idol_fullscreen_control_enter_notify (GtkWidget *widget,
			       GdkEventCrossing *event,
			       IdolFullscreen *fs)
{
	fs->priv->pointer_on_control = TRUE;
	idol_fullscreen_popup_timeout_remove (fs);
	return TRUE;
}

G_MODULE_EXPORT gboolean
idol_fullscreen_control_leave_notify (GtkWidget *widget,
			       GdkEventCrossing *event,
			       IdolFullscreen *fs)
{
	fs->priv->pointer_on_control = FALSE;
	return TRUE;
}

void
idol_fullscreen_set_fullscreen (IdolFullscreen *fs,
				 gboolean fullscreen)
{
	g_return_if_fail (IDOL_IS_FULLSCREEN (fs));

	idol_fullscreen_force_popup_hide (fs);

	bacon_video_widget_set_fullscreen (fs->priv->bvw, fullscreen);
	idol_fullscreen_set_cursor (fs, !fullscreen);

	fs->is_fullscreen = fullscreen;

	if (fullscreen == FALSE && fs->priv->motion_handler_id != 0) {
		g_signal_handler_disconnect (G_OBJECT (fs->priv->bvw),
					     fs->priv->motion_handler_id);
		fs->priv->motion_handler_id = 0;
	} else if (fullscreen != FALSE && fs->priv->motion_handler_id == 0 && fs->priv->bvw != NULL) {
		fs->priv->motion_handler_id = g_signal_connect (G_OBJECT (fs->priv->bvw), "motion-notify-event",
								G_CALLBACK (idol_fullscreen_motion_notify), fs);
	}
}

static void
idol_fullscreen_parent_window_notify (GtkWidget *parent_window,
				       GParamSpec *property,
				       IdolFullscreen *fs)
{
	GtkWidget *popup;

	if (idol_fullscreen_is_fullscreen (fs) == FALSE)
		return;

	popup = gtk_scale_button_get_popup (GTK_SCALE_BUTTON (fs->volume));
	if (parent_window == fs->priv->parent_window &&
	    gtk_window_is_active (GTK_WINDOW (parent_window)) == FALSE &&
	    gtk_widget_get_visible (popup) == FALSE) {
		idol_fullscreen_force_popup_hide (fs);
		idol_fullscreen_set_cursor (fs, TRUE);
	} else {
		idol_fullscreen_set_cursor (fs, FALSE);
	}
}

IdolFullscreen *
idol_fullscreen_new (GtkWindow *toplevel_window)
{
        IdolFullscreen *fs = IDOL_FULLSCREEN (g_object_new 
						(IDOL_TYPE_FULLSCREEN, NULL));

	if (fs->priv->xml == NULL) {
		g_object_unref (fs);
		return NULL;
	}

	idol_fullscreen_set_parent_window (fs, toplevel_window);

	fs->time_label = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml,
				"tcw_time_display_label"));
	fs->buttons_box = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml,
				"tcw_buttons_hbox"));
	fs->exit_button = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml,
				"tefw_fs_exit_button"));

	/* Volume */
	fs->volume = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml, "tcw_volume_button"));
	
	/* Seek */
	fs->seek = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml, "tcw_seek_hscale"));

	/* Motion notify */
	gtk_widget_add_events (fs->seek, GDK_POINTER_MOTION_MASK);
	gtk_widget_add_events (fs->exit_button, GDK_POINTER_MOTION_MASK);

	return fs;
}

void
idol_fullscreen_set_video_widget (IdolFullscreen *fs,
				   BaconVideoWidget *bvw)
{
	g_return_if_fail (IDOL_IS_FULLSCREEN (fs));
	g_return_if_fail (BACON_IS_VIDEO_WIDGET (bvw));
	g_return_if_fail (fs->priv->bvw == NULL);

	fs->priv->bvw = bvw;

	if (fs->is_fullscreen != FALSE && fs->priv->motion_handler_id == 0) {
		fs->priv->motion_handler_id = g_signal_connect (G_OBJECT (fs->priv->bvw), "motion-notify-event",
								G_CALLBACK (idol_fullscreen_motion_notify), fs);
	}
}

void
idol_fullscreen_set_parent_window (IdolFullscreen *fs, GtkWindow *parent_window)
{
	g_return_if_fail (IDOL_IS_FULLSCREEN (fs));
	g_return_if_fail (GTK_IS_WINDOW (parent_window));
	g_return_if_fail (fs->priv->parent_window == NULL);

	fs->priv->parent_window = GTK_WIDGET (parent_window);

	/* Screen size and Theme changes */
	g_signal_connect (fs->priv->parent_window, "realize",
			  G_CALLBACK (idol_fullscreen_window_realize_cb), fs);
	g_signal_connect (fs->priv->parent_window, "unrealize",
			  G_CALLBACK (idol_fullscreen_window_unrealize_cb), fs);
	g_signal_connect (G_OBJECT (fs->priv->parent_window), "notify::is-active",
			  G_CALLBACK (idol_fullscreen_parent_window_notify), fs);
}

static void
idol_fullscreen_init (IdolFullscreen *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, IDOL_TYPE_FULLSCREEN, IdolFullscreenPrivate);

        self->priv->seek_lock = FALSE;
	self->priv->xml = idol_interface_load ("fullscreen.ui", TRUE, NULL, self);

	if (self->priv->xml == NULL)
		return;

	self->priv->pointer_on_control = FALSE;

	self->priv->exit_popup = GTK_WIDGET (gtk_builder_get_object (self->priv->xml,
				"idol_exit_fullscreen_window"));
	g_signal_connect (G_OBJECT (self->priv->exit_popup), "expose-event",
			  G_CALLBACK (idol_fullscreen_exit_popup_expose_cb), self);
	self->priv->control_popup = GTK_WIDGET (gtk_builder_get_object (self->priv->xml,
				"idol_controls_window"));

	/* Motion notify */
	gtk_widget_add_events (self->priv->exit_popup, GDK_POINTER_MOTION_MASK);
	gtk_widget_add_events (self->priv->control_popup, GDK_POINTER_MOTION_MASK);
}

static void
idol_fullscreen_dispose (GObject *object)
{
        IdolFullscreenPrivate *priv = IDOL_FULLSCREEN_GET_PRIVATE (object);

	if (priv->xml != NULL) {
		g_object_unref (priv->xml);
		priv->xml = NULL;
		gtk_widget_destroy (priv->exit_popup);
		gtk_widget_destroy (priv->control_popup);
	}

	G_OBJECT_CLASS (idol_fullscreen_parent_class)->dispose (object);
}

static void
idol_fullscreen_finalize (GObject *object)
{
        IdolFullscreen *fs = IDOL_FULLSCREEN (object);

	idol_fullscreen_popup_timeout_remove (fs);
	if (fs->priv->motion_handler_id != 0) {
		g_signal_handler_disconnect (G_OBJECT (fs->priv->bvw),
					     fs->priv->motion_handler_id);
		fs->priv->motion_handler_id = 0;
	}

	G_OBJECT_CLASS (idol_fullscreen_parent_class)->finalize (object);
}

static void
idol_fullscreen_class_init (IdolFullscreenClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (IdolFullscreenPrivate));

	object_class->dispose = idol_fullscreen_dispose;
        object_class->finalize = idol_fullscreen_finalize;
}

void
idol_fullscreen_set_title (IdolFullscreen *fs, const char *title)
{
	GtkLabel *widget;
	char *text;

	g_return_if_fail (IDOL_IS_FULLSCREEN (fs));

	widget = GTK_LABEL (gtk_builder_get_object (fs->priv->xml, "tcw_title_label"));

	if (title != NULL) {
		char *escaped;

		escaped = g_markup_escape_text (title, -1);
		text = g_strdup_printf
			("<span size=\"medium\"><b>%s</b></span>", escaped);
		g_free (escaped);
	} else {
		text = g_strdup_printf
			("<span size=\"medium\"><b>%s</b></span>",
			 _("No File"));
	}

	gtk_label_set_markup (widget, text);
	g_free (text);
}

void
idol_fullscreen_set_seekable (IdolFullscreen *fs, gboolean seekable)
{
	GtkWidget *item;

	g_return_if_fail (IDOL_IS_FULLSCREEN (fs));

	item = GTK_WIDGET (gtk_builder_get_object (fs->priv->xml, "tcw_time_hbox"));
	gtk_widget_set_sensitive (item, seekable);

	gtk_widget_set_sensitive (fs->seek, seekable);
}

void
idol_fullscreen_set_can_set_volume (IdolFullscreen *fs, gboolean can_set_volume)
{
	g_return_if_fail (IDOL_IS_FULLSCREEN (fs));

	gtk_widget_set_sensitive (fs->volume, can_set_volume);
}
