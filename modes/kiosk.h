// kiosk.h
// Copyright (C)  2002  Dominique Devriese <fritmebufstek@pandora.be>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef KIOSK_H
#define KIOSK_H

#include "mode.h"

// Kiosk mode == full screen...
class KioskMode
  : public KigMode
{
public:
  KioskMode( KigDocument* );
  ~KioskMode();
  void leftClicked( QMouseEvent*, KigView* );
  void midClicked( QMouseEvent*, KigView* );
  void rightClicked( QMouseEvent*, KigView* );
  void leftReleased( QMouseEvent*, KigView* );
  void midReleased( QMouseEvent*, KigView* );
  void rightReleased( QMouseEvent*, KigView* );
  void mouseMoved( QMouseEvent*, KigView* );
};

#endif
