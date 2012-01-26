/* idol-uri.h

   Copyright (C) 2004 Bastien Nocera <hadess@hadess.net>

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

#ifndef IDOL_URI_H
#define IDOL_URI_H

#include "idol.h"
#include <gtk/gtk.h>
#include <gio/gio.h>

G_BEGIN_DECLS

const char *	idol_dot_dir			(void);
const char *	idol_data_dot_dir		(void);
char *		idol_pictures_dir		(void);
char *		idol_create_full_path		(const char *path);
GMount *	idol_get_mount_for_media	(const char *uri);
gboolean	idol_playing_dvd		(const char *uri);
gboolean	idol_uri_is_subtitle		(const char *uri);
gboolean	idol_is_special_mrl		(const char *uri);
gboolean	idol_is_block_device		(const char *uri);
void		idol_setup_file_monitoring	(Idol *idol);
void		idol_setup_file_filters	(void);
void		idol_destroy_file_filters	(void);
char *		idol_uri_get_subtitle_uri	(const char *uri);
char *		idol_uri_escape_for_display	(const char *uri);
GSList *	idol_add_files			(GtkWindow *parent,
						 const char *path);
char *		idol_add_subtitle		(GtkWindow *parent, 
						 const char *uri);

void idol_save_position (Idol *idol);
void idol_try_restore_position (Idol *idol, const char *mrl);

G_END_DECLS

#endif /* IDOL_URI_H */
