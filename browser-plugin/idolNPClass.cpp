/*
 * Copyright © 2008 Christian Persch
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <string.h>

#include "idolNPClass.h"
#include "idolNPObject.h"

idolNPClass_base::idolNPClass_base (const char *aPropertNames[],
                                      uint32_t aPropertyCount,
                                      const char *aMethodNames[],
                                      uint32_t aMethodCount,
                                      const char *aDefaultMethodName) :
  mPropertyNameIdentifiers (GetIdentifiersForNames (aPropertNames, aPropertyCount)),
  mPropertyNamesCount (aPropertyCount),
  mMethodNameIdentifiers (GetIdentifiersForNames (aMethodNames, aMethodCount)),
  mMethodNamesCount (aMethodCount),
  mDefaultMethodIndex (aDefaultMethodName ? GetMethodIndex (NPN_GetStringIdentifier (aDefaultMethodName)) : -1)
{
  structVersion  = NP_CLASS_STRUCT_VERSION_ENUM;
  allocate       = Allocate;
  deallocate     = Deallocate;
  invalidate     = Invalidate;
  hasMethod      = HasMethod;
  invoke         = Invoke;
  invokeDefault  = InvokeDefault;
  hasProperty    = HasProperty;
  getProperty    = GetProperty;
  setProperty    = SetProperty;
  removeProperty = RemoveProperty;
#if defined(NP_CLASS_STRUCT_VERSION_ENUM) && (NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_ENUM)
  enumerate      = Enumerate;
#endif
#if defined(NP_CLASS_STRUCT_VERSION_CTOR) && (NP_CLASS_STRUCT_VERSION >= NP_CLASS_STRUCT_VERSION_CTOR)
  /* FIXMEchpe find out what's this supposed to do */
  /* construct      = Construct; */
  construct = NULL;
#endif
}

idolNPClass_base::~idolNPClass_base ()
{
  NPN_MemFree (mPropertyNameIdentifiers);
  NPN_MemFree (mMethodNameIdentifiers);
}

NPIdentifier*
idolNPClass_base::GetIdentifiersForNames (const char *aNames[],
                                           uint32_t aCount)
{
  if (aCount == 0)
    return NULL;

  NPIdentifier *identifiers = reinterpret_cast<NPIdentifier*>(NPN_MemAlloc (aCount * sizeof (NPIdentifier)));
  if (!identifiers)
    return NULL;

  NPN_GetStringIdentifiers (aNames, aCount, identifiers);

  return identifiers;
}

int
idolNPClass_base::GetPropertyIndex (NPIdentifier aName)
{
  if (!mPropertyNameIdentifiers)
    return -1;

  for (int i = 0; i < mPropertyNamesCount; ++i) {
    if (aName == mPropertyNameIdentifiers[i])
      return i;
  }

  return -1;
}

int
idolNPClass_base::GetMethodIndex (NPIdentifier aName)
{
  if (!mMethodNameIdentifiers)
    return -1;

  for (int i = 0; i < mMethodNamesCount; ++i) {
    if (aName == mMethodNameIdentifiers[i])
      return i;
  }

  return -1;
}

bool
idolNPClass_base::EnumerateProperties (NPIdentifier **_result, uint32_t *_count)
{
  if (!mPropertyNameIdentifiers)
    return false;

  uint32_t bytes = mPropertyNamesCount * sizeof (NPIdentifier);
  NPIdentifier *identifiers = reinterpret_cast<NPIdentifier*>(NPN_MemAlloc (bytes));
  if (!identifiers)
    return false;

  memcpy (identifiers, mPropertyNameIdentifiers, bytes);

  *_result = identifiers;
  *_count = mPropertyNamesCount;

  return true;
}

NPObject*
idolNPClass_base::Allocate (NPP aNPP, NPClass *aClass)
{
  idolNPClass_base* _class = static_cast<idolNPClass_base*>(aClass);
  return _class->InternalCreate (aNPP);
}

void
idolNPClass_base::Deallocate (NPObject *aObject)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  delete object;
}

void
idolNPClass_base::Invalidate (NPObject *aObject)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  object->Invalidate ();
}

bool
idolNPClass_base::HasMethod (NPObject *aObject, NPIdentifier aName)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->HasMethod (aName);
}

bool
idolNPClass_base::Invoke (NPObject *aObject, NPIdentifier aName, const NPVariant *argv, uint32_t argc, NPVariant *_result)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->Invoke (aName, argv, argc, _result);
}

bool
idolNPClass_base::InvokeDefault (NPObject *aObject, const NPVariant *argv, uint32_t argc, NPVariant *_result)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->InvokeDefault (argv, argc, _result);
}

bool
idolNPClass_base::HasProperty (NPObject *aObject, NPIdentifier aName)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->HasProperty (aName);
}

bool
idolNPClass_base::GetProperty (NPObject *aObject, NPIdentifier aName, NPVariant *_result)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->GetProperty (aName, _result);
}

bool
idolNPClass_base::SetProperty (NPObject *aObject, NPIdentifier aName, const NPVariant *aValue)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->SetProperty (aName, aValue);
}

bool
idolNPClass_base::RemoveProperty (NPObject *aObject, NPIdentifier aName)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->RemoveProperty (aName);
}

bool
idolNPClass_base::Enumerate (NPObject *aObject, NPIdentifier **_result, uint32_t *_count)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->Enumerate (_result, _count);
}

bool
idolNPClass_base::Construct (NPObject *aObject, const NPVariant *argv, uint32_t argc, NPVariant *_result)
{
  idolNPObject* object = static_cast<idolNPObject*> (aObject);
  return object->Construct (argv, argc, _result);
}
