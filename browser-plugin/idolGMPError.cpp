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

#include "idolGMPError.h"

static const char *propertyNames[] = {
  "errorCount",
};

static const char *methodNames[] = {
  "clearErrorQueue",
  "item",
  "webHelp"
};

IDOL_IMPLEMENT_NPCLASS (idolGMPError,
                         propertyNames, G_N_ELEMENTS (propertyNames),
                         methodNames, G_N_ELEMENTS (methodNames),
                         NULL);

idolGMPError::idolGMPError (NPP aNPP)
  : idolNPObject (aNPP)
{
  IDOL_LOG_CTOR ();
}

idolGMPError::~idolGMPError ()
{
  IDOL_LOG_DTOR ();
}

bool
idolGMPError::InvokeByIndex (int aIndex,
                              const NPVariant *argv,
                              uint32_t argc,
                              NPVariant *_result)
{
  IDOL_LOG_INVOKE (aIndex, idolGMPError);

  switch (Methods (aIndex)) {
    case eClearErrorQueue:
      /* void clearErrorQueue (); */
    case eWebHelp:
      /* void webHelp (); */
      IDOL_WARN_INVOKE_UNIMPLEMENTED (aIndex, idolGMPError);
      return VoidVariant (_result);
  
    case eItem:
      /* idolIGMPErrorItem item (in long index); */
      IDOL_WARN_1_INVOKE_UNIMPLEMENTED (aIndex, idolGMPError);
      return NullVariant (_result);
  }

  return false;
}

bool
idolGMPError::GetPropertyByIndex (int aIndex,
                                   NPVariant *_result)
{
  IDOL_LOG_GETTER (aIndex, idolGMPError);

  switch (Properties (aIndex)) {
    case eErrorCount:
      /* readonly attribute long errorCount; */
      return Int32Variant (_result, 0);
  }

  return false;
}

bool
idolGMPError::SetPropertyByIndex (int aIndex,
                                   const NPVariant *aValue)
{
  IDOL_LOG_SETTER (aIndex, idolGMPError);

  return ThrowPropertyNotWritable ();
}
