/* Idol Plugin Viewer constants
 *
 * Copyright © 2006 Christian Persch
 * Copyright © 2007 Bastien Nocera <hadess@hadess.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301  USA.
 */

#ifndef __IDOL_PLUGIN_VIEWER_CONSTANTS__
#define __IDOL_PLUGIN_VIEWER_CONSTANTS__

#define IDOL_COMMAND_PLAY		"Play"
#define IDOL_COMMAND_PAUSE		"Pause"
#define IDOL_COMMAND_STOP		"Stop"

typedef enum {
	IDOL_STATE_PLAYING,
	IDOL_STATE_PAUSED,
	IDOL_STATE_STOPPED,
	IDOL_STATE_INVALID
} IdolStates;

static const char *idol_states[] = {
	"PLAYING",
	"PAUSED",
	"STOPPED",
	"INVALID"
};

#define IDOL_PROPERTY_VOLUME		"volume"
#define IDOL_PROPERTY_ISFULLSCREEN	"is-fullscreen"

#endif /* !__IDOL_PLUGIN_VIEWER_CONSTANTS__ */
