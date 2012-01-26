/* Idol GMP plugin
 *
 * Copyright © 2006, 2007 Christian Persch
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
#include "idolGMPSettings.h"

static const char *propertyNames[] = {
  "autostart",
  "balance",
  "baseURL",
  "defaultAudioLanguage",
  "defaultFrame",
  "enableErrorDialogs",
  "invokeURLs",
  "mediaAccessRights",
  "mute",
  "playCount",
  "rate",
  "volume"
};

static const char *methodNames[] = {
  "getMode",
  "isAvailable",
  "requestMediaAccessRights",
  "setMode"
};

IDOL_IMPLEMENT_NPCLASS (idolGMPSettings,
                         propertyNames, G_N_ELEMENTS (propertyNames),
                         methodNames, G_N_ELEMENTS (methodNames),
                         NULL);

idolGMPSettings::idolGMPSettings (NPP aNPP)
  : idolNPObject (aNPP),
    mMute (false)
{
  IDOL_LOG_CTOR ();
}

idolGMPSettings::~idolGMPSettings ()
{
  IDOL_LOG_DTOR ();
}

bool
idolGMPSettings::InvokeByIndex (int aIndex,
                                 const NPVariant *argv,
                                 uint32_t argc,
                                 NPVariant *_result)
{
  IDOL_LOG_INVOKE (aIndex, idolGMPSettings);

  switch (Methods (aIndex)) {
    case eIsAvailable:
      /* boolean isAvailable (in ACString name); */
    case eGetMode:
      /* boolean getMode (in ACString modeName); */
    case eSetMode:
      /* void setMode (in ACString modeName, in boolean state); */
    case eRequestMediaAccessRights:
      /* boolean requestMediaAccessRights (in ACString access); */
      IDOL_WARN_INVOKE_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return BoolVariant (_result, false);
  }

  return false;
}

bool
idolGMPSettings::GetPropertyByIndex (int aIndex,
                                      NPVariant *_result)
{
  IDOL_LOG_GETTER (aIndex, idolGMPSettings);

  switch (Properties (aIndex)) {
    case eMute:
      /* attribute boolean mute; */
      return BoolVariant (_result, Plugin()->IsMute());

    case eVolume:
      /* attribute long volume; */
      return Int32Variant (_result, Plugin()->Volume () * 100.0);

    case eAutostart:
      /* attribute boolean autoStart; */
      return BoolVariant (_result, Plugin()->AutoPlay());

    case eBalance:
      /* attribute long balance; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return Int32Variant (_result, 0);

    case eBaseURL:
      /* attribute AUTF8String baseURL; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return StringVariant (_result, "");

    case eDefaultAudioLanguage:
      /* readonly attribute long defaultAudioLanguage; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return Int32Variant (_result, 0); /* FIXME */

    case eDefaultFrame:
      /* attribute AUTF8String defaultFrame; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return StringVariant (_result, "");

    case eEnableErrorDialogs:
      /* attribute boolean enableErrorDialogs; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return BoolVariant (_result, true);

    case eInvokeURLs:
      /* attribute boolean invokeURLs; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return BoolVariant (_result, true);

    case eMediaAccessRights:
      /* readonly attribute ACString mediaAccessRights; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return StringVariant (_result, "");

    case ePlayCount:
      /* attribute long playCount; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return Int32Variant (_result, 1);

    case eRate:
      /* attribute double rate; */
      IDOL_WARN_1_GETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return DoubleVariant (_result, 1.0);
  }

  return false;
}

bool
idolGMPSettings::SetPropertyByIndex (int aIndex,
                                      const NPVariant *aValue)
{
  IDOL_LOG_SETTER (aIndex, idolGMPSettings);

  switch (Properties (aIndex)) {
    case eMute: {
      /* attribute boolean mute; */
      bool enabled;
      if (!GetBoolFromArguments (aValue, 1, 0, enabled))
        return false;

      Plugin()->SetMute (enabled);
      return true;
    }

    case eVolume: {
      /* attribute long volume; */
      int32_t volume;
      if (!GetInt32FromArguments (aValue, 1, 0, volume))
        return false;

      Plugin()->SetVolume ((double) CLAMP (volume, 0, 100) / 100.0);
      return true;
    }

    case eAutostart: {
      /* attribute boolean autoStart; */
      bool enabled;
      if (!GetBoolFromArguments (aValue, 1, 0, enabled))
        return false;

      Plugin()->SetAutoPlay (enabled);
      return true;
    }

    case eBalance:
      /* attribute long balance; */
    case eBaseURL:
      /* attribute AUTF8String baseURL; */
    case eDefaultFrame:
      /* attribute AUTF8String defaultFrame; */
    case eEnableErrorDialogs:
      /* attribute boolean enableErrorDialogs; */
    case eInvokeURLs:
      /* attribute boolean invokeURLs; */
    case ePlayCount:
      /* attribute long playCount; */
    case eRate:
      /* attribute double rate; */
      IDOL_WARN_SETTER_UNIMPLEMENTED (aIndex, idolGMPSettings);
      return true;

    case eDefaultAudioLanguage:
      /* readonly attribute long defaultAudioLanguage; */
    case eMediaAccessRights:
      /* readonly attribute ACString mediaAccessRights; */
      return ThrowPropertyNotWritable ();
  }

  return false;
}
