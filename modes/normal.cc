// normal.cc
// Copyright (C)  2002  Dominique Devriese <devriese@kde.org>

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
#include "normal.h"

#include "../kig/kig_view.h"
#include "../kig/kig_part.h"
#include "../objects/normalpoint.h"
#include "../misc/kigpainter.h"
#include "popup.h"
#include "moving.h"
#include "macro.h"
#include "dragrectmode.h"
#include "typesdialog.h"

#include <kcursor.h>
#include <kaction.h>
#include <kcommand.h>

QString i18n( const char* );

NormalMode::NormalMode( KigDocument& d )
  : KigMode( d )
{
}

NormalMode::~NormalMode()
{
}

void NormalMode::leftClicked( QMouseEvent* e, KigWidget* v )
{
  plc = e->pos();
  oco = mdoc.whatAmIOn( v->fromScreen( e->pos() ), v->screenInfo() );
  if( oco.empty() )
  {
    DragRectMode d( e->pos(), mdoc, *v );
    mdoc.runMode( &d );
    Objects sel = d.ret();

    Objects cos = sel;

    if ( d.needClear() )
    {
      cos |= sos;
      clearSelection();
    };

    selectObjects( sel );

    KigPainter p( v->screenInfo(), &v->stillPix );
    p.drawObjects( cos );
    v->updateCurPix( p.overlay() );
    v->updateWidget();
  }
  else
  {
    // the user clicked on some object.. --> this could either mean
    // that he/she wants to select the object or that he wants to
    // start moving it.  We assume nothing here, we wait till he
    // either moves some 4 pixels, or till he releases his mouse
    // button in leftReleased() or mouseMoved()...
  };
};

void NormalMode::leftMouseMoved( QMouseEvent* e, KigWidget* v )
{
  // clicked on an object, move it ?
  if( ( plc - e->pos() ).manhattanLength() > 3 )
  {
    // yes, we move it...
    // now to determine what to move...
    if( ( oco & sos ).empty() )
    {
      // the user clicked on something that is currently not
      // selected... --> we select it, taking the Ctrl- and
      // Shift-buttons into account...
      if (!(e->state() & (ControlButton | ShiftButton)))
        clearSelection();
      selectObject(oco.front());
    }

    MovingMode m( sos, v->fromScreen( plc ), *v, mdoc );
    mdoc.runMode( &m );
  };
};

void NormalMode::leftReleased( QMouseEvent* e, KigWidget* v )
{
  Objects cos; // objects whose selection changed..
  if( oco.empty() )
  {
    // the rect for selecting stuff...
    if (!(e->state() & (ControlButton | ShiftButton)))
    {
      cos = sos;
      clearSelection();
    };
    const Rect r =  v->fromScreen( QRect( plc, e->pos() ) );
    Objects os = mdoc.whatIsInHere( r );
    for ( Objects::iterator i = os.begin(); i != os.end(); ++i )
      if ( !cos.contains( *i ) ) cos.push_back( *i );
    selectObjects( os );
  }
  else
  {
    if( (plc - e->pos()).manhattanLength() > 4 ) return;
    if( !sos.contains( oco.front() ) )
    {
      // clicked on objects that weren't selected....
      // we only use oco.front(), since that's what the user
      // expects.  E.g. if he clicks on a point which is on a line,
      // then oco will contain first the point, then the line.
      // Obviously, we only want the point...
      if (!(e->state() & (ControlButton | ShiftButton)))
      {
        cos = sos;
        clearSelection();
      };
      selectObject( oco.front() );
      cos.push_back( oco.front() );
    }
    else
    {
      // clicked on selected objects...
      // we only use oco.front(), since that's what the user
      // expects.  E.g. if he clicks on a point which is on a line,
      // then oco will contain first the point, then the line.
      // Obviously, we only want the point...
      unselectObject( oco.front() );
      cos.push_back( oco.front() );
    };
  };
  KigPainter p( v->screenInfo(), &v->stillPix );
  p.drawObjects( cos );
  v->updateCurPix( p.overlay() );
  v->updateWidget();
}

void NormalMode::midClicked( QMouseEvent* e, KigWidget* v )
{
  plc = e->pos();
  oco = mdoc.whatAmIOn( v->fromScreen( e->pos() ), v->screenInfo() );
  // get rid of text still showing...
  v->updateCurPix();
  v->updateWidget();
}

void NormalMode::midMouseMoved( QMouseEvent*, KigWidget* )
{
};

void NormalMode::midReleased( QMouseEvent* e, KigWidget* v )
{
  // moved too far
  if( (e->pos() - plc).manhattanLength() > 4 ) return;

  Point* pt = NormalPoint::sensiblePoint( v->fromScreen( plc ), mdoc, *v );
  pt->calcForWidget( *v );
  mdoc.addObject( pt );

  // refresh the screen...
  v->redrawScreen();
  v->updateScrollBars();
}

void NormalMode::rightClicked( QMouseEvent* e, KigWidget* v )
{
  plc = e->pos();
  oco = mdoc.whatAmIOn( v->fromScreen( e->pos() ), v->screenInfo() );

  // get rid of text still showing...
  v->updateCurPix();
  // commit this to the widget...
  v->updateWidget();
  // set a normal cursor...
  v->setCursor( KCursor::arrowCursor() );

  if( !oco.empty() )
  {
    if( !sos.contains( oco.front() ) )
    {
      clearSelection();
      selectObject( oco.front() );
    };
    // show a popup menu...
    NormalModePopupObjects* p = new NormalModePopupObjects( mdoc, v, this, sos );
    p->exec( QCursor::pos() );
    delete p;
  }
  else
  {
//     KigDocumentPopup* m = popup( mDoc );
//     if( m ) m->exec( v->mapToGlobal( plc ) );
  };
}

void NormalMode::rightMouseMoved( QMouseEvent*, KigWidget* )
{
  // this is handled by the popup menus ( see rightClicked() )
};

void NormalMode::rightReleased( QMouseEvent*, KigWidget* )
{
  // this is handled by the popup menus ( see rightClicked() )
}

void NormalMode::mouseMoved( QMouseEvent* e, KigWidget* v )
{
  const Objects tmp = mdoc.whatAmIOn( v->fromScreen( e->pos() ), v->screenInfo() );
  v->updateCurPix();
  if( tmp.empty() )
  {
    v->setCursor( KCursor::arrowCursor() );
    mdoc.emitStatusBarText( 0 );
    v->updateWidget();
  }
  else
  {
    // the cursor is over an object, show object type next to cursor
    // and set statusbar text

    v->setCursor( KCursor::handCursor() );
    QString typeName = tmp.front()->vTBaseTypeName();

    // statusbar text
    mdoc.emitStatusBarText( i18n( "Select this %1" ).arg( typeName ) );     KigPainter p( v->screenInfo(), &v->curPix );

    // set the text next to the arrow cursor
    QPoint point = e->pos();
    point.setX(point.x()+15);

    p.drawTextStd( point, typeName );
    v->updateWidget( p.overlay() );
  };
}

void NormalMode::enableActions()
{
  KigMode::enableActions();
  mdoc.enableConstructActions( true );
  mdoc.aDeleteObjects->setEnabled( true );
  mdoc.aShowHidden->setEnabled( true );
  mdoc.aNewMacro->setEnabled( true );
  mdoc.aConfigureTypes->setEnabled( true );
  mdoc.history()->updateActions();
}

void NormalMode::deleteObjects()
{
  mdoc.delObjects( sos );
  sos.clear();
}

void NormalMode::selectObject( Object* o )
{
  sos.push_back( o );
  o->setSelected( true );
}

void NormalMode::selectObjects( Objects& os )
{
  // hehe, don't you love this c++ stuff ;)
  std::for_each( os.begin(), os.end(),
                 std::bind1st(
                   std::mem_fun( &NormalMode::selectObject ),
                   this ) );
}

void NormalMode::unselectObject( Object* o )
{
  o->setSelected( false );
  sos.remove( o );
}

void NormalMode::clearSelection()
{
  for ( Objects::iterator i = sos.begin(); i != sos.end(); ++i )
    (*i)->setSelected( false );
  sos.clear();
}

// KigObjectsPopup* NormalMode::popup( const Objects& )
// {
//   return 0;
// }

// KigDocumentPopup* NormalMode::popup( KigDocument* )
// {
//   return 0;
// }

void NormalMode::showHidden()
{
  const Objects& os = mdoc.objects();
  for (Objects::const_iterator i = os.begin(); i != os.end(); ++i )
    (*i)->setShown(true);
  objectsAdded();
}

void NormalMode::newMacro()
{
  DefineMacroMode m( mdoc );
  mdoc.runMode( &m );
}

void NormalMode::objectsAdded()
{
  KigWidget* w = mdoc.mainWidget()->realWidget();
  w->redrawScreen();
  w->updateScrollBars();
}

void NormalMode::objectsRemoved()
{
  objectsAdded();
}

void NormalMode::editTypes()
{
  TypesDialog* d = new TypesDialog( mdoc.widget() );
  d->exec();
  delete d;
}
