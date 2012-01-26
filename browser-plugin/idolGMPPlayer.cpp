/* Idol GMP plugin
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

#include "npupp.h"

#include "idolPlugin.h"
#include "idolGMPPlayer.h"

static const char *propertyNames[] = {
  "cdromCollection",
  "closedCaption",
  "controls",
  "currentMedia",
  "currentPlaylist",
  "dvd",
  "enableContextMenu",
  "enabled",
  "error",
  "fullScreen",
  "isOnline",
  "isRemote",
  "mediaCollection",
  "network",
  "openState",
  "playerApplication",
  "playlistCollection",
  "playState",
  "settings",
  "status",
  "stretchToFit",
  "uiMode",
  "URL",
  "versionInfo",
  "windowlessVideo",
};

static const char *methodNames[] = {
  "close",
  "launchURL",
  "newMedia",
  "newPlaylist",
  "openPlayer"
};

IDOL_IMPLEMENT_NPCLASS (idolGMPPlayer,
                         propertyNames, G_N_ELEMENTS (propertyNames),
                         methodNames, G_N_ELEMENTS (methodNames),
                         NULL);

idolGMPPlayer::idolGMPPlayer (NPP aNPP)
  : idolNPObject (aNPP)
{
  IDOL_LOG_CTOR ();
}

idolGMPPlayer::~idolGMPPlayer ()
{
  IDOL_LOG_DTOR ();
}

bool
idolGMPPlayer::InvokeByIndex (int aIndex,
                               const NPVariant *argv,
                               uint32_t argc,
                               NPVariant *_result)
{
  IDOL_LOG_INVOKE (aIndex, idolGMPPlayer);

  switch (Methods (aIndex)) {
    case eNewPlaylist:
      /* idolIGMPPlaylist newPlaylist (in AUTF8String name, in AUTF8String URL); */
      IDOL_WARN_INVOKE_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return NullVariant (_result);

    case eClose:
      /* void close (); */
    case eNewMedia:
      /* idolIGMPMedia newMedia (in AUTF8String URL); */
    case eOpenPlayer:
      /* void openPlayer (in AUTF8String URL); */
    case eLaunchURL:
      /* void launchURL (in AUTF8String URL); */
      return ThrowSecurityError ();
  }

  return false;
}

bool
idolGMPPlayer::GetPropertyByIndex (int aIndex,
                                    NPVariant *_result)
{
  IDOL_LOG_GETTER (aIndex, idolGMPPlayer);

  switch (Properties (aIndex)) {
    case eControls:
      /* readonly attribute idolIGMPControls controls; */
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eGMPControls));

    case eNetwork:
      /* readonly attribute idolIGMPNetwork network; */
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eGMPNetwork));

    case eSettings:
      /* readonly attribute idolIGMPSettings settings; */
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eGMPSettings));

    case eVersionInfo:
      /* readonly attribute ACString versionInfo; */
      return StringVariant (_result, IDOL_GMP_VERSION_BUILD);

    case eFullScreen:
      /* attribute boolean fullScreen; */
      return BoolVariant (_result, Plugin()->IsFullscreen());

    case eWindowlessVideo:
      /* attribute boolean windowlessVideo; */
      return BoolVariant (_result, Plugin()->IsWindowless());

    case eIsOnline:
      /* readonly attribute boolean isOnline; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return BoolVariant (_result, true);

    case eEnableContextMenu:
      /* attribute boolean enableContextMenu; */
      return BoolVariant (_result, Plugin()->AllowContextMenu());

    case eClosedCaption:
      /* readonly attribute idolIGMPClosedCaption closedCaption; */
    case eCurrentMedia:
      /* attribute idolIGMPMedia currentMedia; */
    case eCurrentPlaylist:
      /* attribute idolIGMPPlaylist currentPlaylist; */
    case eError:
      /* readonly attribute idolIGMPError error; */
      IDOL_WARN_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return NullVariant (_result);

    case eStatus:
      /* readonly attribute AUTF8String status; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return StringVariant (_result, "OK");

    case eURL:
      /* attribute AUTF8String URL; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return StringVariant (_result, Plugin()->Src()); /* FIXMEchpe use URL()? */

    case eEnabled:
      /* attribute boolean enabled; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return BoolVariant (_result, true);

    case eOpenState:
      /* readonly attribute long openState; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return Int32Variant (_result, 0);

    case ePlayState:
      /* readonly attribute long playState; */
      return Int32Variant (_result, mPluginState);

    case eStretchToFit:
      /* attribute boolean stretchToFit; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return BoolVariant (_result, false);

    case eUiMode:
      /* attribute ACString uiMode; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return VoidVariant (_result);

    case eCdromCollection:
      /* readonly attribute idolIGMPCdromCollection cdromCollection; */
    case eDvd:
      /* readonly attribute idolIGMPDVD dvd; */
    case eMediaCollection:
      /* readonly attribute idolIGMPMediaCollection mediaCollection; */
    case ePlayerApplication:
      /* readonly attribute idolIGMPPlayerApplication playerApplication; */
    case ePlaylistCollection:
      /* readonly attribute idolIGMPPlaylistCollection playlistCollection; */
    case eIsRemote:
      /* readonly attribute boolean isRemote; */
      return ThrowSecurityError ();
  }

  return false;
}

bool
idolGMPPlayer::SetPropertyByIndex (int aIndex,
                                    const NPVariant *aValue)
{
  IDOL_LOG_SETTER (aIndex, idolGMPPlayer);

  switch (Properties (aIndex)) {
    case eFullScreen: {
      /* attribute boolean fullScreen; */
      bool enabled;
      if (!GetBoolFromArguments (aValue, 1, 0, enabled))
        return false;

      Plugin()->SetFullscreen (enabled);
      return true;
    }

    case eWindowlessVideo: {
      /* attribute boolean windowlessVideo; */
      bool enabled;
      if (!GetBoolFromArguments (aValue, 1, 0, enabled))
        return false;

      Plugin()->SetIsWindowless(enabled);
      return true;
    }

    case eURL: {
      /* attribute AUTF8String URL; */
      NPString url;
      if (!GetNPStringFromArguments (aValue, 1, 0, url))
        return false;

      Plugin()->SetSrc (url); /* FIXMEchpe: use SetURL instead?? */
      return true;
    }

    case eEnableContextMenu: {
      /* attribute boolean enableContextMenu; */
      bool enabled;
      if (!GetBoolFromArguments (aValue, 1, 0, enabled))
        return false;

      Plugin()->SetAllowContextMenu (enabled);
      return true;
    }

    case eCurrentMedia:
      /* attribute idolIGMPMedia currentMedia; */
    case eCurrentPlaylist:
      /* attribute idolIGMPPlaylist currentPlaylist; */
    case eEnabled:
      /* attribute boolean enabled; */
    case eStretchToFit:
      /* attribute boolean stretchToFit; */
    case eUiMode:
      /* attribute ACString uiMode; */
      IDOL_WARN_SETTER_UNIMPLEMENTED (aIndex, idolGMPPlayer);
      return true;

    case eCdromCollection:
      /* readonly attribute idolIGMPCdromCollection cdromCollection; */
    case eClosedCaption:
      /* readonly attribute idolIGMPClosedCaption closedCaption; */
    case eControls:
      /* readonly attribute idolIGMPControls controls; */
    case eDvd:
      /* readonly attribute idolIGMPDVD dvd; */
    case eError:
      /* readonly attribute idolIGMPError error; */
    case eIsOnline:
      /* readonly attribute boolean isOnline; */
    case eIsRemote:
      /* readonly attribute boolean isRemote; */
    case eMediaCollection:
      /* readonly attribute idolIGMPMediaCollection mediaCollection; */
    case eNetwork:
      /* readonly attribute idolIGMPNetwork network; */
    case eOpenState:
      /* readonly attribute long openState; */
    case ePlayerApplication:
      /* readonly attribute idolIGMPPlayerApplication playerApplication; */
    case ePlaylistCollection:
      /* readonly attribute idolIGMPPlaylistCollection playlistCollection; */
    case ePlayState:
      /* readonly attribute long playState; */
    case eSettings:
      /* readonly attribute idolIGMPSettings settings; */
    case eStatus:
      /* readonly attribute AUTF8String status; */
    case eVersionInfo:
      /* readonly attribute ACString versionInfo; */
      return ThrowPropertyNotWritable ();
  }

  return false;
}
