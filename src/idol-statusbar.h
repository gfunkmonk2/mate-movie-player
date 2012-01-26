/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 * IdolStatusbar Copyright (C) 1998 Shawn T. Amundson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301  USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __IDOL_STATUSBAR_H__
#define __IDOL_STATUSBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IDOL_TYPE_STATUSBAR            (idol_statusbar_get_type ())
#define IDOL_STATUSBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_STATUSBAR, IdolStatusbar))
#define IDOL_STATUSBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_STATUSBAR, IdolStatusbarClass))
#define IDOL_IS_STATUSBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_STATUSBAR))
#define IDOL_IS_STATUSBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_STATUSBAR))
#define IDOL_STATUSBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), IDOL_TYPE_STATUSBAR, IdolStatusbarClass))


typedef struct _IdolStatusbar      IdolStatusbar;

struct _IdolStatusbar
{
  GtkStatusbar parent_instance;

  GtkWidget *progress;
  GtkWidget *time_label;

  gint time;
  gint length;
  guint timeout;
  guint percentage;

  guint pushed : 1;
  guint seeking : 1;
  guint timeout_ticks : 2;
};

typedef GtkStatusbarClass IdolStatusbarClass;

G_MODULE_EXPORT GType idol_statusbar_get_type  (void) G_GNUC_CONST;
GtkWidget* idol_statusbar_new          	(void);

void       idol_statusbar_set_time		(IdolStatusbar *statusbar,
						 gint time);
void       idol_statusbar_set_time_and_length	(IdolStatusbar *statusbar,
						 gint time, gint length);
void       idol_statusbar_set_seeking          (IdolStatusbar *statusbar,
						 gboolean seeking);

void       idol_statusbar_set_text             (IdolStatusbar *statusbar,
						 const char *label);
void       idol_statusbar_push_help            (IdolStatusbar *statusbar,
						 const char *message);
void       idol_statusbar_pop_help             (IdolStatusbar *statusbar);
void	   idol_statusbar_push			(IdolStatusbar *statusbar,
						 guint percentage);
void       idol_statusbar_pop			(IdolStatusbar *statusbar);

G_END_DECLS

#endif /* __IDOL_STATUSBAR_H__ */
