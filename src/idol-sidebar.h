/* idol-sidebar.h

   Copyright (C) 2004-2005 Bastien Nocera <hadess@hadess.net>

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

#ifndef IDOL_SIDEBAR_H
#define IDOL_SIDEBAR_H

G_BEGIN_DECLS

void idol_sidebar_setup (Idol *idol, gboolean visible,
			  const char *page_id);
void idol_sidebar_toggle (Idol *idol, gboolean state);
void idol_sidebar_set_visibility (Idol *idol, gboolean visible);
gboolean idol_sidebar_is_visible (Idol *idol);
gboolean idol_sidebar_is_focused (Idol *idol, gboolean *handles_kbd);
char *idol_sidebar_get_current_page (Idol *idol);
void idol_sidebar_set_current_page (Idol *idol,
				     const char *name,
				     gboolean force_visible);

G_END_DECLS

#endif /* IDOL_SIDEBAR_H */
