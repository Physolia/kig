// normal.h
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

#ifndef NORMAL_H
#define NORMAL_H

#include "mode.h"

#include "../misc/objects.h"

#include <qpoint.h>

class Object;
class KigObjectsPopup;
class KigDocumentPopup;

class NormalMode
  : public KigMode
{
public:
  NormalMode( KigDocument* );
  ~NormalMode();
  void leftClicked( QMouseEvent*, KigView* );
  void leftMouseMoved( QMouseEvent*, KigView* );
  void leftReleased( QMouseEvent*, KigView* );
  void midClicked( QMouseEvent*, KigView* v );
  void midMouseMoved( QMouseEvent*, KigView* );
  void midReleased( QMouseEvent*, KigView* );
  void rightClicked( QMouseEvent*, KigView* );
  void rightMouseMoved( QMouseEvent*, KigView* );
  void rightReleased( QMouseEvent*, KigView* );
  void mouseMoved( QMouseEvent*, KigView* );

  // objects were added or removed by a command in mDoc->history ...
  void objectsAdded();
  void objectsRemoved();

  void enableActions();

  void deleteObjects();
  void showHidden();
  void newMacro();

  void selectObject( Object* o );
  void selectObjects( Objects& os );
  void unselectObject( Object* o );
  void clearSelection();

  KigObjectsPopup* popup( const Objects& os );
  KigDocumentPopup* popup( KigDocument* );
protected:
  // selected objects...
  Objects sos;

  // objects clicked on...
  Objects oco;

  // point last clicked..
  QPoint plc;
};

#endif
