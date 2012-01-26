/* idol-options.h

   Copyright (C) 2004,2007 Bastien Nocera <hadess@hadess.net>

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

#ifndef IDOL_OPTIONS_H
#define IDOL_OPTIONS_H

#include <mateconf/mateconf-client.h>
#include <unique/uniqueapp.h>

#include "idol.h"

G_BEGIN_DECLS

/* Stores the state of the command line options */
typedef struct {
	gboolean debug;
	gboolean playpause;
	gboolean play;
	gboolean pause;
	gboolean next;
	gboolean previous;
	gboolean seekfwd;
	gboolean seekbwd;
	gboolean volumeup;
	gboolean volumedown;
	gboolean mute;
	gboolean fullscreen;
	gboolean togglecontrols;
	gboolean quit;
	gboolean enqueue;
	gboolean replace;
	gboolean notconnectexistingsession;
	gdouble playlistidx;
	gint64 seek;
	gchar **filenames;
} IdolCmdLineOptions;

extern const GOptionEntry all_options[];
extern IdolCmdLineOptions optionstate;

void idol_options_register_remote_commands (Idol *idol);
void idol_options_process_early (Idol *idol,
				  const IdolCmdLineOptions* options);
void idol_options_process_late (Idol *idol, 
				 const IdolCmdLineOptions* options);
void idol_options_process_for_server (UniqueApp *app,
				       const IdolCmdLineOptions* options);

G_END_DECLS

#endif /* IDOL_OPTIONS_H */
