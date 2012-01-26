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
#include <stdio.h>
#include <stdarg.h>

#include <glib.h>

#include "idolNPClass.h"
#include "idolNPObject.h"

#ifdef DEBUG_PLUGIN
#define NOTE(x) x
#else
#define NOTE(x)
#endif

static const char *variantTypes[] = {
  "void",
  "null",
  "bool",
  "int32",
  "double",
  "string",
  "object",
  "unknown"
};

#define VARIANT_TYPE(type) (variantTypes[MIN (type, NPVariantType_Object + 1)])

void*
idolNPObject::operator new (size_t aSize) throw ()
{
  void *instance = ::operator new (aSize);
  if (instance) {
    memset (instance, 0, aSize);
  }

  return instance;
}

idolNPObject::idolNPObject (NPP aNPP)
  : mNPP (aNPP),
    mPlugin (reinterpret_cast<idolPlugin*>(aNPP->pdata))
{
  NOTE (g_print ("idolNPObject ctor [%p]\n", (void*) this));
}

idolNPObject::~idolNPObject ()
{
  NOTE (g_print ("idolNPObject dtor [%p]\n", (void*) this));
}

bool
idolNPObject::Throw (const char *aMessage)
{
  NOTE (g_print ("idolNPObject::Throw [%p] : %s\n", (void*) this, aMessage));

  NPN_SetException (this, aMessage);
  return false;
}

bool
idolNPObject::ThrowPropertyNotWritable ()
{
  return Throw ("Property not writable");
}

bool
idolNPObject::ThrowSecurityError ()
{
  return Throw ("Access denied");
}

bool
idolNPObject::CheckArgc (uint32_t argc,
                          uint32_t minArgc,
                          uint32_t maxArgc,
                          bool doThrow)
{
  if (argc >= minArgc && argc <= maxArgc)
    return true;

  if (argc < minArgc) {
    if (doThrow)
      return Throw ("Not enough arguments");

    return false;
  }

  if (doThrow)
    return Throw ("Too many arguments");

  return false;
}

bool
idolNPObject::CheckArgType (NPVariantType argType,
                             NPVariantType expectedType,
                             uint32_t argNum)
{
  bool conforms;

  switch (argType) {
    case NPVariantType_Void:
    case NPVariantType_Null:
      conforms = (argType == expectedType);
      break;

    case NPVariantType_Bool:
      conforms = (argType == NPVariantType_Bool ||
                  argType == NPVariantType_Int32 ||
                  argType == NPVariantType_Double);
      break;

    case NPVariantType_Int32:
    case NPVariantType_Double:
      /* FIXMEchpe: also accept NULL or VOID ? */
      conforms = (argType == NPVariantType_Int32 ||
                  argType == NPVariantType_Double);
      break;

    case NPVariantType_String:
    case NPVariantType_Object:
      conforms = (argType == expectedType ||
                  argType == NPVariantType_Null ||
                  argType == NPVariantType_Void);
      break;
    default:
      conforms = false;
  }

  if (!conforms) {
      char msg[128];
      g_snprintf (msg, sizeof (msg),
                  "Wrong type of argument %d: expected %s but got %s\n",
                  argNum, VARIANT_TYPE (expectedType), VARIANT_TYPE (argType));

      return Throw (msg);
  }

  return true;
}

bool
idolNPObject::CheckArg (const NPVariant *argv,
                         uint32_t argc,
                         uint32_t argNum,
                         NPVariantType type)
{
  if (!CheckArgc (argc, argNum + 1))
    return false;

  return CheckArgType (argv[argNum].type, type, argNum);
}

bool
idolNPObject::CheckArgv (const NPVariant* argv,
                          uint32_t argc,
                          uint32_t expectedArgc,
                          ...)
{
  if (!CheckArgc (argc, expectedArgc, expectedArgc))
    return false;

  va_list type_args;
  va_start (type_args, expectedArgc);

  for (uint32_t i = 0; i < argc; ++i) {
    NPVariantType type = NPVariantType (va_arg (type_args, int /* promotion */));

    if (!CheckArgType (argv[i].type, type)) {
      va_end (type_args);
      return false;
    }
  }

  va_end (type_args);

  return true;
}

bool
idolNPObject::GetBoolFromArguments (const NPVariant* argv,
                                     uint32_t argc,
                                     uint32_t argNum,
                                     bool& _result)
{
  if (!CheckArg (argv, argc, argNum, NPVariantType_Bool))
    return false;

  NPVariant arg = argv[argNum];
  if (NPVARIANT_IS_BOOLEAN (arg)) {
    _result = NPVARIANT_TO_BOOLEAN (arg);
  } else if (NPVARIANT_IS_INT32 (arg)) {
    _result = NPVARIANT_TO_INT32 (arg) != 0;
  } else if (NPVARIANT_IS_DOUBLE (arg)) {
    _result = NPVARIANT_TO_DOUBLE (arg) != 0.0;
  } else {
    /* void/null */
    _result = false;
  }

  return true;
}

bool
idolNPObject::GetInt32FromArguments (const NPVariant* argv,
                                      uint32_t argc,
                                      uint32_t argNum,
                                      int32_t& _result)
{
  if (!CheckArg (argv, argc, argNum, NPVariantType_Int32))
    return false;

  NPVariant arg = argv[argNum];
  if (NPVARIANT_IS_INT32 (arg)) {
    _result = NPVARIANT_TO_INT32 (arg);
  } else if (NPVARIANT_IS_DOUBLE (arg)) {
    _result = int32_t (NPVARIANT_TO_DOUBLE (arg));
    /* FIXMEchpe: overflow? */
  }

  return true;
}

bool
idolNPObject::GetDoubleFromArguments (const NPVariant* argv,
                                       uint32_t argc,
                                       uint32_t argNum,
                                       double& _result)
{
  if (!CheckArg (argv, argc, argNum, NPVariantType_Double))
    return false;

  NPVariant arg = argv[argNum];
  if (NPVARIANT_IS_DOUBLE (arg)) {
    _result = NPVARIANT_TO_DOUBLE (arg);
  } else if (NPVARIANT_IS_INT32 (arg)) {
    _result = double (NPVARIANT_TO_INT32 (arg));
  }

  return true;
}

bool
idolNPObject::GetNPStringFromArguments (const NPVariant* argv,
                                         uint32_t argc,
                                         uint32_t argNum,
                                         NPString& _result)
{
  if (!CheckArg (argv, argc, argNum, NPVariantType_String))
    return false;

  NPVariant arg = argv[argNum];
  if (NPVARIANT_IS_STRING (arg)) {
    _result = NPVARIANT_TO_STRING (arg);
  } else if (NPVARIANT_IS_NULL (arg) ||
             NPVARIANT_IS_VOID (arg)) {
    _result.UTF8Characters = NULL;
    _result.UTF8Length = 0;
  }

  return true;
}

bool
idolNPObject::DupStringFromArguments (const NPVariant* argv,
                                       uint32_t argc,
                                       uint32_t argNum,
                                       char*& _result)
{
  NPN_MemFree (_result);
  _result = NULL;

  NPString newValue;
  if (!GetNPStringFromArguments (argv, argc, argNum, newValue))
    return false;

  _result = NPN_StrnDup (newValue.UTF8Characters, newValue.UTF8Length);
  return true;
}

bool
idolNPObject::GetObjectFromArguments (const NPVariant* argv,
                                        uint32_t argc,
                                        uint32_t argNum,
                                        NPObject*& _result)
{
  if (!CheckArg (argv, argc, argNum, NPVariantType_Object))
    return false;

  NPVariant arg = argv[argNum];
  if (NPVARIANT_IS_STRING (arg)) {
    _result = NPVARIANT_TO_OBJECT (arg);
  } else if (NPVARIANT_IS_NULL (arg) ||
             NPVARIANT_IS_VOID (arg)) {
    _result = NULL;
  }

  return true;
}

bool
idolNPObject::VoidVariant (NPVariant* _result)
{
  VOID_TO_NPVARIANT (*_result);
  return true;
}

bool
idolNPObject::NullVariant (NPVariant* _result)
{
  NULL_TO_NPVARIANT (*_result);
  return true;
}

bool
idolNPObject::BoolVariant (NPVariant* _result,
                            bool value)
{
  BOOLEAN_TO_NPVARIANT (value, *_result);
  return true;
}

bool
idolNPObject::Int32Variant (NPVariant* _result,
                             int32_t value)
{
  INT32_TO_NPVARIANT (value, *_result);
  return true;
}

bool
idolNPObject::DoubleVariant (NPVariant* _result,
                              double value)
{
  DOUBLE_TO_NPVARIANT (value, *_result);
  return true;
}

bool
idolNPObject::StringVariant (NPVariant* _result,
                              const char* value,
                              int32_t len)
{
  if (!value) {
    NULL_TO_NPVARIANT (*_result);
  } else {
    char *dup;

    if (len < 0) {
      len = strlen (value);
      dup = (char*) NPN_MemDup (value, len + 1);
    } else {
      dup = (char*) NPN_MemDup (value, len);
    }

    if (dup) {
      STRINGN_TO_NPVARIANT (dup, len, *_result);
    } else {
      NULL_TO_NPVARIANT (*_result);
    }
  }

  return true;
}

bool
idolNPObject::ObjectVariant (NPVariant* _result,
                              NPObject* object)
{
  if (object) {
    NPN_RetainObject (object);
    OBJECT_TO_NPVARIANT (object, *_result);
  } else {
    NULL_TO_NPVARIANT (*_result);
  }

  return true;
}

/* NPObject method default implementations */

void
idolNPObject::Invalidate ()
{
  NOTE (g_print ("idolNPObject %p invalidated\n", (void*) this));

  mNPP = NULL;
  mPlugin = NULL;
}

bool
idolNPObject::HasMethod (NPIdentifier aName)
{
  if (!IsValid ())
    return false;

  int methodIndex = GetClass()->GetMethodIndex (aName);
  NOTE (g_print ("idolNPObject::HasMethod [%p] %s => %s\n", (void*) this, NPN_UTF8FromIdentifier (aName), methodIndex >= 0 ? "yes" : "no"));
  if (methodIndex >= 0)
    return true;

  if (aName == NPN_GetStringIdentifier ("__noSuchMethod__"))
    return true;

  return false;
}

bool
idolNPObject::Invoke (NPIdentifier aName,
                       const NPVariant *argv,
                       uint32_t argc,
                       NPVariant *_result)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::Invoke [%p] %s\n", (void*) this, NPN_UTF8FromIdentifier (aName)));
  int methodIndex = GetClass()->GetMethodIndex (aName);
  if (methodIndex >= 0)
    return InvokeByIndex (methodIndex, argv, argc, _result);

  if (aName == NPN_GetStringIdentifier ("__noSuchMethod__")) {
    /* http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Global_Objects:Object:_noSuchMethod */
    if (!CheckArgv (argv, argc, 2, NPVariantType_String, NPVariantType_Object))
      return false;

    const char *id = NPVARIANT_TO_STRING (argv[0]).UTF8Characters;
    g_message ("NOTE: site calls unknown function \"%s\" on idolNPObject %p\n", id ? id : "(null)", (void*) this);

    /* Silently ignore the invocation */
    VOID_TO_NPVARIANT (*_result);
    return true;
  }

  return Throw ("No method with this name exists.");
}

bool
idolNPObject::InvokeDefault (const NPVariant *argv,
                              uint32_t argc,
                              NPVariant *_result)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::InvokeDefault [%p]\n", (void*) this));
  int defaultMethodIndex = GetClass()->GetDefaultMethodIndex ();
  if (defaultMethodIndex >= 0)
    return InvokeByIndex (defaultMethodIndex, argv, argc, _result);

  return false;
}

bool
idolNPObject::HasProperty (NPIdentifier aName)
{
  if (!IsValid ())
    return false;

  int propertyIndex = GetClass()->GetPropertyIndex (aName);
  NOTE (g_print ("idolNPObject::HasProperty [%p] %s => %s\n", (void*) this, NPN_UTF8FromIdentifier (aName), propertyIndex >= 0 ? "yes" : "no"));
  if (propertyIndex >= 0)
    return true;

  return false;
}

bool
idolNPObject::GetProperty (NPIdentifier aName,
                            NPVariant *_result)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::GetProperty [%p] %s\n", (void*) this, NPN_UTF8FromIdentifier (aName)));
  int propertyIndex = GetClass()->GetPropertyIndex (aName);
  if (propertyIndex >= 0)
    return GetPropertyByIndex (propertyIndex, _result);

  return Throw ("No property with this name exists.");
}

bool
idolNPObject::SetProperty (NPIdentifier aName,
                            const NPVariant *aValue)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::SetProperty [%p] %s\n", (void*) this, NPN_UTF8FromIdentifier (aName)));
  int propertyIndex = GetClass()->GetPropertyIndex (aName);
  if (propertyIndex >= 0)
    return SetPropertyByIndex (propertyIndex, aValue);

  return Throw ("No property with this name exists.");
}

bool
idolNPObject::RemoveProperty (NPIdentifier aName)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::RemoveProperty [%p] %s\n", (void*) this, NPN_UTF8FromIdentifier (aName)));
  int propertyIndex = GetClass()->GetPropertyIndex (aName);
  if (propertyIndex >= 0)
    return RemovePropertyByIndex (propertyIndex);

  return Throw ("No property with this name exists.");
}

bool
idolNPObject::Enumerate (NPIdentifier **_result,
                          uint32_t *_count)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::Enumerate [%p]\n", (void*) this));
  return GetClass()->EnumerateProperties (_result, _count);
}

bool
idolNPObject::Construct (const NPVariant *argv,
                          uint32_t argc,
                          NPVariant *_result)
{
  if (!IsValid ())
    return false;

  NOTE (g_print ("idolNPObject::Construct [%p]\n", (void*) this));
  return false; /* FIXMEchpe! */
}

/* by-index methods */

bool
idolNPObject::InvokeByIndex (int aIndex,
                              const NPVariant *argv,
                              uint32_t argc,
                              NPVariant *_result)
{
  return false;
}

bool
idolNPObject::GetPropertyByIndex (int aIndex,
                                   NPVariant *_result)
{
  return false;
}

bool
idolNPObject::SetPropertyByIndex (int aIndex,
                                   const NPVariant *aValue)
{
  return false;
}

bool
idolNPObject::RemovePropertyByIndex (int aIndex)
{
  return Throw ("Removing properties is not supported.");
}
