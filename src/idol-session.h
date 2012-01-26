/* idol-session.h

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

#ifndef IDOL_SESSION_H
#define IDOL_SESSION_H

#include "idol.h"

G_BEGIN_DECLS

void idol_session_add_options (GOptionContext *context);
void idol_session_setup (Idol *idol, char **argv);
void idol_session_restore (Idol *idol, char **filenames);

G_END_DECLS

#endif /* IDOL_SESSION_H */
