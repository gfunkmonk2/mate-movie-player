/* 
   Copyright (C) 2004, Bastien Nocera <hadess@hadess.net>

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

#ifndef IDOL_SCRSAVER_H
#define IDOL_SCRSAVER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define IDOL_TYPE_SCRSAVER		(idol_scrsaver_get_type ())
#define IDOL_SCRSAVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), IDOL_TYPE_SCRSAVER, IdolScrsaver))
#define IDOL_SCRSAVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), IDOL_TYPE_SCRSAVER, IdolScrsaverClass))
#define IDOL_IS_SCRSAVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IDOL_TYPE_SCRSAVER))
#define IDOL_IS_SCRSAVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), IDOL_TYPE_SCRSAVER))

typedef struct IdolScrsaver IdolScrsaver;
typedef struct IdolScrsaverClass IdolScrsaverClass;
typedef struct IdolScrsaverPrivate IdolScrsaverPrivate;

struct IdolScrsaver {
	GObject parent;
	IdolScrsaverPrivate *priv;
};

struct IdolScrsaverClass {
	GObjectClass parent_class;
};

GType idol_scrsaver_get_type		(void) G_GNUC_CONST;
IdolScrsaver *idol_scrsaver_new       (void);
void idol_scrsaver_enable		(IdolScrsaver *scr);
void idol_scrsaver_disable		(IdolScrsaver *scr);
void idol_scrsaver_set_state		(IdolScrsaver *scr,
					 gboolean enable);

G_END_DECLS

#endif /* !IDOL_SCRSAVER_H */
