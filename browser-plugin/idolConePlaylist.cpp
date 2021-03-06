/* Idol Cone plugin
 *
 * Copyright © 2004 Bastien Nocera <hadess@hadess.net>
 * Copyright © 2002 David A. Schleef <ds@schleef.org>
 * Copyright © 2006, 2008 Christian Persch
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

#include <config.h>

#include <string.h>

#include <glib.h>

#include "idolPlugin.h"
#include "idolConePlaylist.h"

static const char *propertyNames[] = {
  "isPlaying",
  "items"
};

static const char *methodNames[] = {
  "add",
  "next",
  "play",
  "playItem",
  "prev",
  "removeItem",
  "stop",
  "togglePause"
};

IDOL_IMPLEMENT_NPCLASS (idolConePlaylist,
                         propertyNames, G_N_ELEMENTS (propertyNames),
                         methodNames, G_N_ELEMENTS (methodNames),
                         NULL);

idolConePlaylist::idolConePlaylist (NPP aNPP)
  : idolNPObject (aNPP)
{
  IDOL_LOG_CTOR ();
}

idolConePlaylist::~idolConePlaylist ()
{
  IDOL_LOG_DTOR ();
}

bool
idolConePlaylist::InvokeByIndex (int aIndex,
                                  const NPVariant *argv,
                                  uint32_t argc,
                                  NPVariant *_result)
{
  IDOL_LOG_INVOKE (aIndex, idolConePlaylist);

  switch (Methods (aIndex)) {
    case eAdd: {
      /* long add (in AUTF8String MRL, [in AUTF8String name, in AUTF8String options]); */
      if (!CheckArgc (argc, 1, 3))
        return false;

      NPString mrl;
      if (!GetNPStringFromArguments (argv, argc, 0, mrl))
        return false;

      NPString title;
      if (argc != 3 || !GetNPStringFromArguments (argv, argc, 1, title))
        title.UTF8Characters = NULL;

      NPString options;
      if (argc != 3 || !GetNPStringFromArguments (argv, argc, 2, options))
        options.UTF8Characters = NULL;
      //FIXME handle options as array
      //http://wiki.videolan.org/Documentation:WebPlugin#Playlist_object
      char *subtitle = NULL;
      if (options.UTF8Characters && options.UTF8Length) {
        char *str, **items;
        guint i;

        str = g_strndup (options.UTF8Characters, options.UTF8Length);
        items = g_strsplit (str, " ", -1);
        g_free (str);

        for (i = 0; items[i] != NULL; i++) {
          if (g_str_has_prefix (items[i], ":sub-file=")) {
            subtitle = g_strdup (items[i] + strlen (":sub-file="));
	    break;
	  }
	}
	g_strfreev (items);
      }

      Plugin()->AddItem (mrl, title, subtitle);
      g_free (subtitle);
      //FIXME we're supposed to return a unique number here
      return Int32Variant (_result, 1);
    }

    case ePlay:
      Plugin()->Command (IDOL_COMMAND_PLAY);
      return VoidVariant (_result);

    case eStop:
      Plugin()->Command (IDOL_COMMAND_STOP);
      return VoidVariant (_result);

    case eTogglePause:
      if (Plugin()->State() == IDOL_STATE_PLAYING) {
	Plugin()->Command (IDOL_COMMAND_PAUSE);
      } else if (Plugin()->State() == IDOL_STATE_PAUSED) {
	Plugin()->Command (IDOL_COMMAND_PLAY);
      }
      return VoidVariant (_result);

    case eNext:
    case ePlayItem:
    case ePrev:
    case eRemoveItem:
      IDOL_WARN_INVOKE_UNIMPLEMENTED (aIndex, idolConePlaylist);
      return VoidVariant (_result);
  }

  return false;
}

bool
idolConePlaylist::GetPropertyByIndex (int aIndex,
                                       NPVariant *_result)
{
  IDOL_LOG_GETTER (aIndex, idolConePlaylist);

  switch (Properties (aIndex)) {
    case eItems:
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eConePlaylistItems));

    case eIsPlaying:
      return BoolVariant (_result, Plugin()->State() == IDOL_STATE_PLAYING);
  }

  return false;
}

bool
idolConePlaylist::SetPropertyByIndex (int aIndex,
                                       const NPVariant *aValue)
{
  IDOL_LOG_SETTER (aIndex, idolConePlaylist);

  return ThrowPropertyNotWritable ();
}
