/* Idol Cone plugin
 *
 * Copyright © 2004 Bastien Nocera <hadess@hadess.net>
 * Copyright © 2002 David A. Schleef <ds@schleef.org>
 * Copyright © 2006, 2007, 2008 Christian Persch
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

#ifndef __IDOL_CONE_PLAYLIST_H__
#define __IDOL_CONE_PLAYLIST_H__

#include "idolNPClass.h"
#include "idolNPObject.h"

class idolConePlaylist : public idolNPObject
{
  public:
    idolConePlaylist (NPP);
    virtual ~idolConePlaylist ();

  private:

    enum Methods {
      eAdd,
      eNext,
      ePlay,
      ePlayItem,
      ePrev,
      eRemoveItem,
      eStop,
      eTogglePause
    };

    enum Properties {
      eIsPlaying,
      eItems
    };

    virtual bool InvokeByIndex (int aIndex, const NPVariant *argv, uint32_t argc, NPVariant *_result);
    virtual bool GetPropertyByIndex (int aIndex, NPVariant *_result);
    virtual bool SetPropertyByIndex (int aIndex, const NPVariant *aValue);
};

IDOL_DEFINE_NPCLASS (idolConePlaylist);

#endif /* __IDOL_CONE_PLAYLIST_H__ */
