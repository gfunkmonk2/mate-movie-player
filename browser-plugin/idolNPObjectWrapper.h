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

#ifndef __IDOL_NPOBJECT_WRAPPER_H__
#define __IDOL_NPOBJECT_WRAPPER_H__

#include <npapi.h>
#include <npruntime.h>

#include <assert.h>

class idolNPObjectWrapper {

  public:

    idolNPObjectWrapper () : mObject (0) { }
    idolNPObjectWrapper (NPObject *aObject) : mObject (aObject) { } /* adopts */
    idolNPObjectWrapper (const idolNPObjectWrapper& aOther) { Assign (aOther.mObject); }

    ~idolNPObjectWrapper () { Assign (0); }

    bool IsNull () const { return mObject == 0; }

    idolNPObjectWrapper& operator= (NPObject *aObject) { Assign (aObject); return *this; }

    operator void*() const { return reinterpret_cast<void*>(mObject); }
    operator NPObject*() const { return mObject; }
    NPObject* operator->() const { assert (!IsNull ()); return mObject; }

    class GetterRetains {
      public:
       explicit GetterRetains (idolNPObjectWrapper& aTarget) : mTarget (aTarget) { VOID_TO_NPVARIANT (mVariant); }
        ~GetterRetains () {
          if (!NPVARIANT_IS_VOID (mVariant)) {
            if (NPVARIANT_IS_OBJECT (mVariant)) {
              mTarget = NPVARIANT_TO_OBJECT (mVariant);
            }
            NPN_ReleaseVariantValue (&mVariant);
          }
       }

       operator void**() { return reinterpret_cast<void**> (mTarget.StartAssignment ()); } // FIXMEchpe this looks wrong...
       operator NPObject**() { return mTarget.StartAssignment (); }
       operator NPVariant*() { return &mVariant; }

       /* NPN_GetValue uses void* which is broken */
       operator void*() { return reinterpret_cast<void*> (mTarget.StartAssignment ()); }

      private:
        idolNPObjectWrapper& mTarget;
        NPVariant mVariant;
    };

    class AlreadyRetained {
      public:
        explicit AlreadyRetained (NPObject *aObject) : mObject (aObject) { }
        ~AlreadyRetained () { }

        NPObject *Get () const { return mObject; }
      private:
        NPObject *mObject;
    };

    idolNPObjectWrapper& operator= (const AlreadyRetained& aRetainer) { Adopt (aRetainer.Get()); return *this; }

  protected:

    idolNPObjectWrapper& operator= (const idolNPObjectWrapper&); // not implemented

    void Assign (NPObject *aObject) {
      if (mObject) {
        NPN_ReleaseObject (mObject);
      }

      mObject = aObject;
      if (mObject) {
        NPN_RetainObject (mObject);
      }
    }

    void Adopt (NPObject *aObject) {
      if (mObject) {
        NPN_ReleaseObject (mObject);
      }

      mObject = aObject;
    }

    NPObject** StartAssignment () { Assign (0); return &mObject; }

    NPObject *mObject;
};

inline idolNPObjectWrapper::GetterRetains
getter_Retains (idolNPObjectWrapper &aTarget)
{
  return idolNPObjectWrapper::GetterRetains (aTarget);
}

inline idolNPObjectWrapper::AlreadyRetained
do_CreateInstance (idolNPClass_base* aClass, NPP aNPP)
{
  assert (aClass);
  assert (aNPP);
  return idolNPObjectWrapper::AlreadyRetained (aClass->CreateInstance (aNPP));
}

#endif /* __IDOL_NPOBJECT_WRAPPER_H__ */
