/**
 This file is part of Kig, a KDE program for Interactive Geometry...
 Copyright (C) 2002  Dominique Devriese <dominique.devriese@student.kuleuven.ac.be>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 USA
**/


#include "locus.h"

#include "normalpoint.h"

#include "../misc/calcpaths.h"
#include "../misc/kigpainter.h"

#include <kdebug.h>

#include "../misc/i18n.h"

/**
 * Locus: the calc routines of this class are a bit unusual:
 * they're the reason we gave calc() the argument showingRect.
 * calcPointLocus only keeps points that are within the current
 * showing rect...
 */

void Locus::draw(KigPainter& p, bool ss) const
{
  // we don't let the points draw themselves, since that would get us
  // the big points (5x5) like we normally see them...
  // all objs are of the same type, so we only check the first
  p.setPen( QPen( selected && ss ? Qt::red : mColor ) );
  for (CPts::const_iterator i = pts.begin(); i != pts.end(); ++i)
    p.drawPoint( i->pt );
}

bool Locus::contains(const Coordinate& o, const double fault ) const
{
  for (CPts::const_iterator i = pts.begin(); i != pts.end(); ++i)
    if( ( i->pt - o ).length() < fault ) return true;
  return false;
}

bool Locus::inRect(const Rect& r) const
{
  for (CPts::const_iterator i = pts.begin(); i != pts.end(); ++i)
    if( r.contains( i->pt ) ) return true;
  return false;
}

void Locus::calc( const ScreenInfo& r )
{
  mvalid = cp->valid() && mp->valid();
  if ( mvalid )
  {
    // sometimes calc() is called with an empty rect, cause the
    // shownRect is not known yet.. we ignore those calcs...
    if ( r.shownRect() == Rect() ) return;
    pts.clear();
    pts.reserve( numberOfSamples );
    double oldP = cp->constrainedImp()->getP();
    CPts::iterator b = addPoint( 0, r );
    CPts::iterator e = addPoint( 1, r );
    int i = 2;
    recurse(b,e,i,r);
    // reset cp and its children to their former state...
    cp->constrainedImp()->setP(oldP);
    cp->calc( r );
    calcpath.calc( r );
  };
}

Coordinate Locus::getPoint( double param ) const
{
  if ( ! mvalid ) return Coordinate();

  // save the old param..
  double tmp = cp->constrainedImp()->getP();
  cp->constrainedImp()->setP(param);

  // calc the new coord..
  // hack... hope no-one tries recursive Locuses...
  calcpath.calc( ScreenInfo( Rect(), QRect() ) );
  Coordinate t = mp->getCoord();

  // restore the old param...
  cp->constrainedImp()->setP(tmp);
  // hack... hope no-one tries recursive Locuses...
  calcpath.calc( ScreenInfo( Rect(), QRect() ) );
  return t;
}

double Locus::getParam(const Coordinate&) const
{
  return 0.5;
}

inline Locus::CPts::iterator Locus::addPoint( double param, const ScreenInfo& r )
{
  cp->constrainedImp()->setP(param);
  cp->calc( r );
  calcpath.calc( r );
  pts.push_back(CPt(mp->getCoord(), param));
  return pts.end() - 1;
}

void Locus::recurse(CPts::iterator first, CPts::iterator last, int& i, const ScreenInfo& si )
{
  const Rect& r = si.shownRect();
  if ( i++ > numberOfSamples ) return;
  if( !( r.contains( first->pt ) || r.contains( last->pt ) ) && i > 20 ) return;
  double p = (first->pm+last->pm)/2;
  CPts::iterator n = addPoint( p, si );
  if( i <= 20 || ( n->pt - first->pt ).length() > (1.5*si.pixelWidth()) )
    recurse( n, first, i, si );
  if (i > numberOfSamples) return;
  if( i <= 20 || ( n->pt - last->pt ).length() > (1.5*si.pixelWidth()) )
    recurse( n, last, i, si );
  if (i > numberOfSamples) return;
}

Locus::Locus(const Locus& loc)
  : Curve(), cp(loc.cp), mp(loc.mp)
{
  cp->addChild(this);
  mp->addChild(this);
  calcpath = calcPath( Objects( cp ), mp );
  // we always want to calc obj after its arguments...
  calcpath.push_back( mp );
}

Objects Locus::getParents() const
{
  Objects tmp;
  tmp.push_back(cp);
  tmp.push_back(mp);
  return tmp;
}

Locus::~Locus()
{
  delete_all( objs.begin(), objs.end() );
}

Locus* Locus::copy()
{
  return new Locus(*this);
}

const QCString Locus::vBaseTypeName() const
{
  return sBaseTypeName();
}

const QCString Locus::sBaseTypeName()
{
  return I18N_NOOP("curve");
}

const QCString Locus::vFullTypeName() const
{
  return sFullTypeName();
}

const QCString Locus::sFullTypeName()
{
  return "Locus";
}

const QString Locus::vDescriptiveName() const
{
  return sDescriptiveName();
}

const QString Locus::sDescriptiveName()
{
  return i18n("Locus");
}

const QString Locus::vDescription() const
{
  return sDescription();
}

const QString Locus::sDescription()
{
  return i18n( "Construct a locus: let one point move around, and record "
               "the places another object passes through. These combined "
               "form a new object: the locus..." );
}

const QCString Locus::vIconFileName() const
{
  return sIconFileName();
}

const QCString Locus::sIconFileName()
{
  return "locus";
}

const int Locus::vShortCut() const
{
  return sShortCut();
}

const int Locus::sShortCut()
{
  return 0;
}

void Locus::startMove(const Coordinate&)
{
}

void Locus::moveTo(const Coordinate&)
{
}

void Locus::stopMove()
{
}

const char* Locus::sActionName()
{
  return "objects_new_locus";
}

Locus::Locus( const Objects& os )
{
  assert( os.size() == 2 );
  for ( Objects::const_iterator i = os.begin(); i != os.end(); ++i )
  {
    if ( (*i)->toNormalPoint() && (*i)->toNormalPoint()->constrainedImp() )
      cp = (*i)->toNormalPoint();
    else
    {
      mp = (*i)->toPoint();
      assert( mp );
    }
  };
  assert ( cp && mp );
  cp->addChild( this );
  mp->addChild( this );

  calcpath = calcPath( Objects( cp ), mp );
  calcpath.push_back( mp );
};

Object::WantArgsResult Locus::sWantArgs( const Objects& os )
{
  uint size = os.size();
  if ( size != 1 && size != 2 ) return NotGood;
  bool gotcp = false;
  bool gotmp = false;
  for ( Objects::const_iterator i = os.begin(); i != os.end(); ++i )
  {
    if ( !gotcp && (*i)->toNormalPoint()
         && (*i)->toNormalPoint()->constrainedImp() )
      gotcp = true;
    else if ( (*i)->toPoint() ) gotmp = true;
  };
  if ( size == 1 ) return ( gotmp || gotcp ) ? NotComplete : NotGood;
  assert( size == 2 );
  return ( gotmp && gotcp ) ? Complete : NotGood;
}

QString Locus::sUseText( const Objects&, const Object* o )
{
  if (o->toNormalPoint() && o->toNormalPoint()->constrainedImp() )
    return i18n("Moving point");
  return i18n("Dependent object");
}

void Locus::sDrawPrelim( KigPainter&, const Objects& )
{
}
