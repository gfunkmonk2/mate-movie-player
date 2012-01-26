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
#include "idolCone.h"

static const char *propertyNames[] = {
  "audio",
  "input",
  "iterator",
  "log",
  "messages",
  "playlist",
  "VersionInfo",
  "video",
};

static const char *methodNames[] = {
  "versionInfo"
};

IDOL_IMPLEMENT_NPCLASS (idolCone,
                         propertyNames, G_N_ELEMENTS (propertyNames),
                         methodNames, G_N_ELEMENTS (methodNames),
                         NULL);

idolCone::idolCone (NPP aNPP)
  : idolNPObject (aNPP)
{
  IDOL_LOG_CTOR ();
}

idolCone::~idolCone ()
{
  IDOL_LOG_DTOR ();
}

bool
idolCone::InvokeByIndex (int aIndex,
                          const NPVariant *argv,
                          uint32_t argc,
                          NPVariant *_result)
{
  IDOL_LOG_INVOKE (aIndex, idolCone);

  switch (Methods (aIndex)) {
    case eversionInfo:
      return GetPropertyByIndex (eVersionInfo, _result);
  }

  return false;
}

bool
idolCone::GetPropertyByIndex (int aIndex,
                               NPVariant *_result)
{
  IDOL_LOG_GETTER (aIndex, idolCone);

  switch (Properties (aIndex)) {
    case eAudio:
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eConeAudio));

    case eInput:
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eConeInput));

    case ePlaylist:
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eConePlaylist));

    case eVideo:
      return ObjectVariant (_result, Plugin()->GetNPObject (idolPlugin::eConeVideo));

    case eVersionInfo:
      return StringVariant (_result, IDOL_CONE_VERSION);

    case eIterator:
    case eLog:
    case eMessages:
      IDOL_WARN_GETTER_UNIMPLEMENTED (aIndex, _result);
      return NullVariant (_result);
  }

  return false;
}

bool
idolCone::SetPropertyByIndex (int aIndex,
                               const NPVariant *aValue)
{
  IDOL_LOG_SETTER (aIndex, idolCone);

  return ThrowPropertyNotWritable ();
}
