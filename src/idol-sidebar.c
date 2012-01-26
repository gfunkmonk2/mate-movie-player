/* idol-sidebar.c

   Copyright (C) 2004-2005 Bastien Nocera

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301  USA.

   Author: Bastien Nocera <hadess@hadess.net>
 */

#include "config.h"

#include <glib/gi18n.h>

#include "idol.h"
#include "idol-sidebar.h"
#include "idol-private.h"
#include "ev-sidebar.h"

static void
cb_resize (Idol * idol)
{
	GValue gvalue_size = { 0, };
	gint handle_size;
	GtkWidget *pane;
	GtkAllocation allocation;
	int w, h;

	gtk_widget_get_allocation (idol->win, &allocation);
	w = allocation.width;
	h = allocation.height;

	g_value_init (&gvalue_size, G_TYPE_INT);
	pane = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_main_pane"));
	gtk_widget_style_get_property (pane, "handle-size", &gvalue_size);
	handle_size = g_value_get_int (&gvalue_size);
	g_value_unset (&gvalue_size);
	
	gtk_widget_get_allocation (idol->sidebar, &allocation);
	if (idol->sidebar_shown) {
		w += allocation.width + handle_size;
	} else {
		w -= allocation.width + handle_size;
	}

	if (w > 0 && h > 0)
		gtk_window_resize (GTK_WINDOW (idol->win), w, h);
}

void
idol_sidebar_toggle (Idol *idol, gboolean state)
{
	GtkAction *action;
	GtkWidget *box, *arrow;

	if (gtk_widget_get_visible (GTK_WIDGET (idol->sidebar)) == state)
		return;

	if (state != FALSE)
		gtk_widget_show (GTK_WIDGET (idol->sidebar));
	else
		gtk_widget_hide (GTK_WIDGET (idol->sidebar));

	action = gtk_action_group_get_action (idol->main_action_group, "sidebar");
	idol_signal_block_by_data (G_OBJECT (action), idol);
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), state);
	idol_signal_unblock_by_data (G_OBJECT (action), idol);

	box = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_sidebar_button_hbox"));
	arrow = g_object_get_data (G_OBJECT (box), "arrow");
	gtk_arrow_set (GTK_ARROW (arrow), state ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT, GTK_SHADOW_NONE);

	idol->sidebar_shown = state;
	cb_resize(idol);
}

static void
toggle_sidebar_from_sidebar (GtkWidget *playlist, Idol *idol)
{
	idol_sidebar_toggle (idol, FALSE);
}

gboolean
idol_sidebar_is_visible (Idol *idol)
{
	return idol->sidebar_shown;
}

static gboolean
has_popup (void)
{
	GList *list, *l;
	gboolean retval = FALSE;

	list = gtk_window_list_toplevels ();
	for (l = list; l != NULL; l = l->next) {
		GtkWindow *window = GTK_WINDOW (l->data);
		if (gtk_widget_get_visible (GTK_WIDGET (window)) && gtk_window_get_window_type (window) == GTK_WINDOW_POPUP) {
			retval = TRUE;
			break;
		}
	}
	g_list_free (list);
	return retval;
}

gboolean
idol_sidebar_is_focused (Idol *idol, gboolean *handles_kbd)
{
	GtkWidget *focused;

	if (handles_kbd != NULL)
		*handles_kbd = has_popup ();

	focused = gtk_window_get_focus (GTK_WINDOW (idol->win));
	if (focused != NULL && gtk_widget_is_ancestor
			(focused, GTK_WIDGET (idol->sidebar)) != FALSE) {
		return TRUE;
	}

	return FALSE;
}

void
idol_sidebar_setup (Idol *idol, gboolean visible, const char *page_id)
{
	GtkPaned *item;
	GtkAction *action;

	item = GTK_PANED (gtk_builder_get_object (idol->xml, "tmw_main_pane"));
	idol->sidebar = ev_sidebar_new ();
	ev_sidebar_add_page (EV_SIDEBAR (idol->sidebar),
			"playlist", _("Playlist"),
			GTK_WIDGET (idol->playlist));
	if (page_id != NULL) {
		ev_sidebar_set_current_page (EV_SIDEBAR (idol->sidebar),
				page_id);
	} else {
		ev_sidebar_set_current_page (EV_SIDEBAR (idol->sidebar),
				"playlist");
	}
	gtk_paned_pack2 (item, idol->sidebar, FALSE, FALSE);

	idol->sidebar_shown = visible;

	action = gtk_action_group_get_action (idol->main_action_group,
			"sidebar");

	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);

	/* Signals */
	g_signal_connect (G_OBJECT (idol->sidebar), "closed",
			G_CALLBACK (toggle_sidebar_from_sidebar), idol);

	gtk_widget_show_all (idol->sidebar);
	gtk_widget_realize (idol->sidebar);

	if (!visible) {
		gtk_widget_hide (idol->sidebar);
	} else {
		GtkWidget *box, *arrow;

		box = GTK_WIDGET (gtk_builder_get_object (idol->xml, "tmw_sidebar_button_hbox"));
		arrow = g_object_get_data (G_OBJECT (box), "arrow");
		gtk_arrow_set (GTK_ARROW (arrow), GTK_ARROW_LEFT, GTK_SHADOW_NONE);
	}
}

char *
idol_sidebar_get_current_page (Idol *idol)
{
	if (idol->sidebar == NULL)
		return NULL;
	return ev_sidebar_get_current_page (EV_SIDEBAR (idol->sidebar));
}

void
idol_sidebar_set_current_page (Idol *idol,
				const char *name,
				gboolean force_visible)
{
	if (name == NULL)
		return;

	ev_sidebar_set_current_page (EV_SIDEBAR (idol->sidebar), name);
	if (force_visible != FALSE)
		idol_sidebar_toggle (idol, TRUE);
}
